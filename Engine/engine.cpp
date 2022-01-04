#include "engine.h"

void Engine::_parseQualifier(Qualifiers & qualifier_output, const json & qualifier_json) {
    for (const auto& qualifier : qualifier_json.items()) {
        std::string qualifier_key_string(qualifier.key());

        for (const auto & single_qualifier : qualifier.value()) {
            std::shared_ptr<BaseValue> val_ptr;
            BaseValue::parseValue(val_ptr, single_qualifier);
            qualifier_output[qualifier_key_string].push_back(val_ptr);
            _key_type[qualifier_key_string] = val_ptr -> type;
        }
    }
}

Engine::Engine(std::string & kb_file_name, int worker_num) {
    _worker_num = worker_num;

    std::cout << "Initiate a new KB reading from \"" + kb_file_name + "\"" << std::endl;
    json kb;
    std::ifstream kb_file;
    kb_file.open(kb_file_name, std::ios::in);

    if (kb_file.is_open()) {
        kb_file >> kb;
        kb_file.close();
    }
    else {
        std::cout << "Cannot find the file\n";
        kb_file.close();
        exit(235);
    }
    std::cout << "End of parsing json file \n";

    json concept_json(kb.at("concepts"));
    json entity_json(kb.at("entities"));

    auto total_concept_num = concept_json.size();
    auto total_entity_num  = entity_json.size();
    std::cout << "number of concepts " << total_concept_num << std::endl;
    std::cout << "number of entities " << total_entity_num << std::endl;

    // Reserve Space for vectors
    _concept_name.reserve(total_concept_num);
    _concept_sub_class_of.reserve(total_concept_num);

    _entity_name.reserve(total_entity_num);
    _entity_is_instance_of.reserve(total_entity_num);
    _entity_attribute.reserve(total_entity_num);
    _entity_relation.reserve(total_entity_num);

    // Reserve for Index
    _concept_has_instance_entities.resize(total_concept_num);


    // Construct "_concept_id", "_concept_id_to_number", "_concept_name", "_concept_name_to_number"
    for (const auto & concept : concept_json.items()) {
        std::string concept_id(concept.key());
        std::string concept_name(concept.value().at("name"));

        _concept_id.push_back(concept_id);
        _concept_id_to_number[concept_id] = (int)_concept_id.size() - 1;

        _concept_name.push_back(concept_name);
        _concept_name_to_number[concept_name].push_back((int)_concept_name.size() - 1);
    }

    // Construct "_concept_sub_class_of"
    for (const auto & c: concept_json.items()) {
        std::string concept_id(c.key());
        int cur_number = _concept_id_to_number[concept_id];

        _concept_sub_class_of.emplace_back();

        auto & concept_super_class = c.value().at("subclassOf");
        for (const auto& sup_class : concept_super_class) {
            int sup_class_number = _concept_id_to_number[sup_class];
            _concept_sub_class_of[cur_number].insert(sup_class_number);
        }
    }

    // Construct "entity_id", "entity_id_to_number", "entity_name", "entity_name_to_number"
    // Construct "_entity_is_instance_of"
    // Construct "_entity_attribute"
    for (const auto & entity : entity_json.items()) {
        std::string entity_id(entity.key());
        auto entity_name = entity.value().at("name").get<std::string>();

        // For "entity_id", "entity_id_to_number", "entity_name", "entity_name_to_number"
        _entity_id.push_back(entity_id);
        _entity_id_to_number[entity_id] = (int)_entity_id.size() - 1;
        _entity_name.push_back(entity_name);
        if (_entity_name_to_number.find(entity_name) == _entity_name_to_number.end()) {
            _entity_name_to_number[entity_name] = std::make_shared<Entities>();
        }
        _entity_name_to_number[entity_name] -> push_back((int)_entity_name.size() - 1);

        // For "_entity_is_instance_of"
        auto cur_entity_number = (int)_entity_name.size() - 1;
        _entity_is_instance_of.emplace_back();
        for (const auto& concept_id : entity.value().at("instanceOf")) {
            auto concept_id_in_string = concept_id.get<std::string>();
            int concept_number = _concept_id_to_number[concept_id_in_string];
            _entity_is_instance_of[cur_entity_number].insert(concept_number);
            _concept_has_instance_entities[concept_number].insert(cur_entity_number);
        }

        // For "_entity_attribute"
        std::map<std::string, std::vector<std::shared_ptr<Attribute>>> ent_attrs;
        for (const auto& attribute_json : entity.value().at("attributes")) {
            auto attribute = std::make_shared<Attribute>();
            // obtain key
            auto attribute_key = attribute_json.at("key").get<std::string>();
            // construct inverted index for attribute keys
            _attribute_key_to_entities[attribute_key].insert(cur_entity_number);
            // obtain value
            BaseValue::parseValue(attribute -> attribute_value, attribute_json.at("value"));
            _key_type[attribute_key] = attribute -> attribute_value -> type;
            // obtain qualifiers
            _parseQualifier(attribute -> fact_qualifiers, attribute_json.at("qualifiers"));

            ent_attrs[attribute_key].push_back(attribute);
        }
        _entity_attribute.push_back( ent_attrs );
    }

    // Construct "_entity_relation"
    for (const auto & entity : entity_json.items()) {
        std::vector<std::shared_ptr<Relation>> relation;

        for (const auto & relation_json : entity.value().at("relations")) {
            auto relation_name_string(relation_json.at("relation").get<std::string>());
            auto relation_direction_string(relation_json.at("direction").get<std::string>());
            RelationDirection relation_direction;
            if (relation_direction_string == "forward") {
                relation_direction = RelationDirection::forward;
            }
            else if (relation_direction_string == "backward") {
                relation_direction = RelationDirection::backward;
            }
            else {
                std::cout << "Error!\n";
                exit(113);
            }
            auto tail_entity_id_string(relation_json.at("object").get<std::string>());
            auto tail_entity_number = _entity_id_to_number[tail_entity_id_string];

            auto rel = std::make_shared<Relation>();
            rel -> relation_name = relation_name_string;
            rel -> relation_direction = relation_direction;
            rel -> relation_tail_entity = tail_entity_number;
            _parseQualifier(rel -> fact_qualifiers, relation_json.at("qualifiers"));

            relation.push_back(rel);

            RelationIndex relation_index(relation_name_string, relation_direction);
            EntityPairIndex entity_pair_index((int)_entity_relation.size(), tail_entity_number);

            _relation_to_entity_pair[relation_index].push_back(entity_pair_index);
            _entity_pair_to_relation[entity_pair_index].push_back(relation_index);

            auto rel_idx = (int)relation.size() - 1;
            _relation_in_entity_index[relation_index][entity_pair_index.head_entity].push_back(rel_idx);
            _entity_forward_relation_index[entity_pair_index].push_back(rel_idx);
        }
        _entity_relation.push_back(relation);
    }

    // construct all entities
    _all_entities = std::make_shared<EntitiesWithFacts>();
    _all_entities -> first = std::make_shared<Entities>();
    _all_entities -> first -> reserve(total_entity_num);
    for (std::size_t i = 0; i < _entity_name.size(); i++) {
        _all_entities -> first -> push_back((int)i);
    }

//    examineEntityAttribute();
//    examineRelation();
//    examineAttributeKeyIndex();
//    examineRelationIndex();
//    examineEntityPairIndex();

    std::cout << "End of initialize a new KB\n";
}

void Engine::examineEntityAttribute() const {
    for (std::size_t i = 0; i < _entity_name.size(); i++) {
        std::cout << "Name: " << _entity_name[i] << ", Taxonomy: ";
        for (const auto& tax : _entity_is_instance_of[i]) {
            std::cout << _concept_name[tax] << "|";
        }
        std::cout << std::endl;
        for (const auto & attr : _entity_attribute[i]) {
            auto attr_key = attr.first;
            auto attr_vals = attr.second;
            std::cout << "-- " << attr_key << ": " << std::endl;
            for (const auto & attr_val : attr_vals) {
                std::cout << "  -- " << attr_val -> attribute_value->toPrintStr() << std::endl;
                for (const auto & qualifier : attr_val -> fact_qualifiers) {
                    auto qual_key = qualifier.first;
                    auto qual_vals = qualifier.second;
                    std::cout << "    -- Qualifier Key: " << qual_key << ": " << std::endl;
                    for (const auto & qual_val : qual_vals) {
                        std::cout << "      -- " << qual_val->toPrintStr() << std::endl;
                    }
                }
            }
        }
    }
}

void Engine::examineRelation() const {
    for (std::size_t i = 0; i < _entity_name.size(); i++) {
        auto head_entity = _entity_name[i];
        std::cout << head_entity << ": " << _entity_relation[i].size() << std::endl;

        for (const auto & relation : _entity_relation[i]) {
            auto tail_entity_number = relation -> relation_tail_entity;
            auto tail_entity = _entity_name[tail_entity_number];
            auto relation_name = relation -> relation_name;
            auto relation_direction = relation -> relation_direction;
            if (relation_direction == RelationDirection::forward) {
                std::cout << "  -->";
            }
            else {
                std::cout << "  <--";
            }
            std::cout << "(" << head_entity << ", " << relation_name << ", " << tail_entity << ")" << std::endl;
            for (const auto & qualifiers : relation -> fact_qualifiers) {
                auto qual_key = qualifiers.first;
                auto qual_vals = qualifiers.second;
                std::cout << "    -- Qualifier Key: " << qual_key << ": " << std::endl;
                for (const auto & qual_val : qual_vals) {
                    std::cout << "      -- " << qual_val->toPrintStr() << std::endl;
                }
            }
        }
    }
}

void Engine::examineAttributeKeyIndex() const {

}

void Engine::examineRelationIndex() const {

}

void Engine::examineEntityPairIndex() const {

}


std::shared_ptr<Engine::EntitiesWithFacts>
Engine::_filter_qualifier(const std::shared_ptr<EntitiesWithFacts> & entity_with_fact,
                          const std::string & filter_key,
                          const std::shared_ptr<BaseValue> & value_to_compare,
                          const std::string & op) const {
    auto satisfy_entity_with_fact_ptr = std::make_shared<EntitiesWithFacts>();
    satisfy_entity_with_fact_ptr -> first = std::make_shared<Entities>();
    satisfy_entity_with_fact_ptr -> second = std::make_shared<Facts>();

    assert(entity_with_fact -> first -> size() == entity_with_fact -> second -> size());
    for (int i = 0; i < (int)(entity_with_fact -> first -> size()); ++i) {
        auto & entity_id = (*(entity_with_fact -> first))[i];
        auto & attr = (*(entity_with_fact -> second))[i];
        auto & qualifiers = attr -> fact_qualifiers;
        for (const auto & qualifier : qualifiers) {
            if (qualifier.first == filter_key) {
                for (const auto & qualifier_value : qualifier.second) {
                    if (qualifier_value -> valueCompare(value_to_compare.get(), op)) {
                        satisfy_entity_with_fact_ptr -> first -> push_back(entity_id);
                        satisfy_entity_with_fact_ptr -> second -> push_back(attr);
                    }
                }
            }
        }
    }
    return satisfy_entity_with_fact_ptr;
}

std::shared_ptr<Engine::EntitiesWithFacts>
Engine::_filter_attribute(
        const std::shared_ptr<EntitiesWithFacts> & entity_ids,
        const std::string & key,
        const std::shared_ptr<BaseValue> & value_to_compare,
        const std::string & op) const {

    auto entity_with_fact_ptr = std::make_shared<EntitiesWithFacts>();
    entity_with_fact_ptr -> first = std::make_shared<Entities>();
    entity_with_fact_ptr -> second = std::make_shared<Facts>();

    // enumerate over all entities
    for (const auto & ent : *(entity_ids -> first)) {
        const auto & entity_attributes = _entity_attribute[ent];

        // enumerate over all attributes with satisfying key
        if (entity_attributes.find(key) != entity_attributes.end()) {
            for (const auto & entity_att : entity_attributes.at(key)) {

                // save the fact that satisfying the filtering condition
                if (entity_att -> attribute_value -> valueCompare(value_to_compare.get(), op)) {
                    entity_with_fact_ptr -> first -> push_back(ent);
                    entity_with_fact_ptr -> second -> push_back(entity_att);
                }
            }
        }
    }
    return entity_with_fact_ptr;
}

VerifyResult
Engine::_verify(
        const std::shared_ptr<Values> & input_str_value,
        const std::shared_ptr<BaseValue> & verify_value,
        const std::string & verify_op) const {
    int match_num = 0;
    for (const auto & attr_value : *input_str_value) {
        if (attr_value -> valueCompare(verify_value.get(), verify_op)) {
            match_num++;
        }
    }
    if (match_num == int(input_str_value -> size())) {
        return VerifyResult::yes;
    } else if (match_num == 0) {
        return VerifyResult::no;
    } else {
        return VerifyResult::not_sure;
    }
}

std::shared_ptr<Engine::EntitiesWithFacts>
Engine::findAll() const {
    return _all_entities;
}

std::shared_ptr<Engine::EntitiesWithFacts>
Engine::find(const std::string & find_entity_name) const {
    if (_entity_name_to_number.find(find_entity_name) != _entity_name_to_number.end()) {
        auto res_ptr = std::make_shared<EntitiesWithFacts>();
        res_ptr -> first = _entity_name_to_number.at(find_entity_name);
        return res_ptr;
    }
    else {
        return std::make_shared<EntitiesWithFacts>();
    }
}

std::shared_ptr<Engine::EntitiesWithFacts>
Engine::filterConcept(
        const std::shared_ptr<EntitiesWithFacts> & entity_ids,
        const std::string & concept_name) const {

    auto concept_numbers = std::vector<int>();
    if (_concept_name_to_number.find(concept_name) != _concept_name_to_number.end()) {
        concept_numbers = _concept_name_to_number.at(concept_name);
    }

    std::set<int> entity_set;
    for (const auto& concept_num : concept_numbers) {
        auto concept_inst = _concept_has_instance_entities.at(concept_num);
        entity_set.insert(concept_inst.begin(), concept_inst.end());
    }

    auto output_entities = std::make_shared<EntitiesWithFacts>();
    output_entities -> first = std::make_shared<Entities>();

    for (const auto & ent : *(entity_ids -> first)) {
        if (entity_set.find(ent) != entity_set.end()) {
            output_entities -> first -> push_back(ent);
        }
    }

    return output_entities;
}

std::shared_ptr<Engine::EntitiesWithFacts>
Engine::filterStr(
        const std::shared_ptr<EntitiesWithFacts> & entity_ids,
        const std::string & string_key,
        const std::string & string_value) const {
    auto value_to_compare = std::make_shared<StringValue>(string_value);
    auto return_pairs = _filter_attribute(entity_ids, string_key, value_to_compare, "=");
    return return_pairs;
}

std::shared_ptr<Engine::EntitiesWithFacts>
Engine::filterNum(
        const std::shared_ptr<EntitiesWithFacts> & entity_ids,
        const std::string & number_key,
        const std::string & number_value,
        const std::string & op) const {
    auto value_to_compare = std::make_shared<QuantityValue>(number_value);
    auto return_pairs = _filter_attribute(entity_ids, number_key, value_to_compare, op);
    return return_pairs;
}

std::shared_ptr<Engine::EntitiesWithFacts>
Engine::filterYear(
        const std::shared_ptr<EntitiesWithFacts> & entity_ids,
        const std::string & year_key,
        const std::string & year_value,
        const std::string & op) const {
    auto value_to_compare = std::make_shared<YearValue>(year_value);
    auto return_pairs = _filter_attribute(entity_ids, year_key, value_to_compare, op);
    return return_pairs;
}

std::shared_ptr<Engine::EntitiesWithFacts>
Engine::filterDate(
        const std::shared_ptr<EntitiesWithFacts> & entity_ids,
        const std::string & date_key,
        const std::string & date_value,
        const std::string & op) const {
    auto value_to_compare = std::make_shared<DateValue>(date_value);
    auto return_pairs = _filter_attribute(entity_ids, date_key, value_to_compare, op);
    return return_pairs;
}

VerifyResult
Engine::verifyStr(
        const std::shared_ptr<Values> & input_str_value,
        const std::string & verify_str_value) const {
    auto value_to_compare = std::make_shared<StringValue>(verify_str_value);
    return _verify(input_str_value, value_to_compare, "=");
}

VerifyResult
Engine::verifyNum(
        const std::shared_ptr<Values> & input_num_value,
        const std::string & verify_num_value,
        const std::string & verify_num_op) const {
    auto value_to_compare = std::make_shared<QuantityValue>(verify_num_value);
    return _verify(input_num_value, value_to_compare, verify_num_op);
}

VerifyResult
Engine::verifyYear(
        const std::shared_ptr<Values> & input_year_value,
        const std::string & verify_year_value,
        const std::string & verify_year_op) const {
    auto value_to_compare = std::make_shared<YearValue>(verify_year_value);
    return _verify(input_year_value, value_to_compare, verify_year_op);
}

VerifyResult
Engine::verifyDate(
        const std::shared_ptr<Values> & input_date_value,
        const std::string & verify_date_value,
        const std::string & verify_date_op) const {
    auto value_to_compare = std::make_shared<DateValue>(verify_date_value);
    return _verify(input_date_value, value_to_compare, verify_date_op);
}

std::shared_ptr<std::vector<const std::string* >>
Engine::queryName(
        const std::shared_ptr<EntitiesWithFacts> & entity_ids) const {
    auto name_ptr_vector_ptr = std::make_shared<std::vector<const std::string* >>();
    for (const auto & entity_id : *(entity_ids -> first)) {
        name_ptr_vector_ptr -> push_back(&(_entity_name[entity_id]));
    }
    return name_ptr_vector_ptr;
}

std::shared_ptr<Engine::Values>
Engine::queryAttr(
        const std::shared_ptr<EntitiesWithFacts> & entity_ids,
        const std::string & query_attribute_key) const {
    auto base_value_ptr_vector_ptr = std::make_shared<Values>();
    for (const auto& entity_id : *(entity_ids -> first)) {
        const auto & entity_attributes = _entity_attribute[entity_id];
        if (entity_attributes.find(query_attribute_key) != entity_attributes.end()){
            for (const auto & entity_att : entity_attributes.at(query_attribute_key)) {
                base_value_ptr_vector_ptr -> push_back(entity_att -> attribute_value);
            }
        }
    }
    return base_value_ptr_vector_ptr;
}

std::shared_ptr<Engine::Values>
Engine::queryAttrUnderCondition(const std::shared_ptr<EntitiesWithFacts> & entity_ids,
                                const std::string & query_attribute_key,
                                const std::string & qualifier_key,
                                const std::string & qualifier_value) const {
    auto return_ptr = std::make_shared<Values>();
    auto qualifier_value_to_query_under = BaseValue::convertStringToValue(qualifier_value, _key_type.at(qualifier_key));
    for (const auto& entity_id : *(entity_ids -> first)) {
        const auto & entity_attributes = _entity_attribute[entity_id];
        if (entity_attributes.find(query_attribute_key) != entity_attributes.end()){
            for (const auto & entity_att : entity_attributes.at(query_attribute_key)) {
                const auto & qualifiers = entity_att -> fact_qualifiers;
                bool find_flag = false;
                for (const auto & qualifier_pair : qualifiers) {
                    if (qualifier_pair.first != qualifier_key)     continue;
                    for (const auto & q_value : qualifier_pair.second) {
                        if (q_value -> valueCompare(qualifier_value_to_query_under.get(), "=")) {
                            find_flag = true;
                            break;
                        }
                    }
                    if (find_flag)      break;
                }
                if (find_flag) {
                    return_ptr -> push_back(entity_att -> attribute_value);
                }
            }
        }
    }
    return return_ptr;
}

std::shared_ptr<std::vector<const std::string *>>
Engine::queryRelation(const std::shared_ptr<EntitiesWithFacts> & entity_ids_a,
                      const std::shared_ptr<EntitiesWithFacts> & entity_ids_b) const {
    auto return_ptr = std::make_shared<std::vector<const std::string *>>();
    for (const auto & entity_a : *(entity_ids_a -> first)) {
        for (const auto & entity_b : *(entity_ids_b -> first)) {
            EntityPairIndex entity_pair(entity_a, entity_b);
            if (_entity_pair_to_relation.find(entity_pair) != _entity_pair_to_relation.end()) {
                const auto & relations = _entity_pair_to_relation.at(entity_pair);
                for (const auto & relation : relations) {
                    return_ptr -> push_back(&(relation.relation_name));
                }
            }
        }
    }
    return return_ptr;
}

std::shared_ptr<Engine::Values>
Engine::queryAttrQualifier(
        const std::shared_ptr<EntitiesWithFacts> & entity_ids,
        const std::string & attribute_key,
        const std::string & attribute_value,
        const std::string & qualifier_key) const {
    auto return_ptr = std::make_shared<Values>();
    auto attribute_value_to_query = BaseValue::convertStringToValue(attribute_value, _key_type.at(attribute_key));
    for (const auto & entity_id : *(entity_ids -> first)) {
        const auto & entity_attributes = _entity_attribute[entity_id];
        if (entity_attributes.find(attribute_key) != entity_attributes.end()){
            for (const auto & entity_att : entity_attributes.at(attribute_key)) {
                if (entity_att -> attribute_value -> valueCompare(attribute_value_to_query.get(), "=")) {
                    const auto & qualifiers = entity_att -> fact_qualifiers;
                    for (const auto & qualifier_pair : qualifiers) {
                        if (qualifier_pair.first == qualifier_key) {
                            for (const auto & qualifier_value : qualifier_pair.second) {
                                return_ptr -> push_back(qualifier_value);
                            }
                        }
                    }
                }
            }
        }
    }
    return return_ptr;
}

std::shared_ptr<Engine::Values>
Engine::queryRelationQualifier(
        const std::shared_ptr<EntitiesWithFacts> & entity_ids_a,
        const std::shared_ptr<EntitiesWithFacts> & entity_ids_b,
        const std::string & qualifier_key,
        const std::string & relation_name) const {
    auto value_of_satisfied_qualifiers_ptr = std::make_shared<Values>();

    // enumerate over entity_list_a x entity_list_b
    for (const auto & entity_a : *(entity_ids_a -> first)) {
    for (const auto & entity_b : *(entity_ids_b -> first)) {
        EntityPairIndex entity_pair(entity_a, entity_b);

        // Find and enumerate over all relations between entity_a and entity_b
        if (_entity_forward_relation_index.find(entity_pair) != _entity_forward_relation_index.end()) {
        for (const auto & forward_relation_index: _entity_forward_relation_index.at(entity_pair)) {

            const auto & relation = _entity_relation[entity_a][forward_relation_index];
            if (relation -> relation_name == relation_name) {
                for (const auto & qualifier_key_value_pair : relation -> fact_qualifiers) {
                    const auto & qual_key = qualifier_key_value_pair.first;
                    if (qual_key == qualifier_key) {
                        value_of_satisfied_qualifiers_ptr -> insert(
                                value_of_satisfied_qualifiers_ptr->end(),
                                qualifier_key_value_pair.second.begin(),
                                qualifier_key_value_pair.second.end());
                    }
                }
            }
        }}
    }}

    return value_of_satisfied_qualifiers_ptr;
}

std::shared_ptr<std::vector<const std::string *>>
Engine::selectAmong(
        const std::shared_ptr<EntitiesWithFacts> & entity_ids,
        const std::string & attribute_key,
        const std::string & select_op) const {
    auto return_ptr = std::make_shared<std::vector<const std::string *>>();

    SelectOperator select_operator;
    if (select_op == "less") {
        select_operator = SelectOperator::smallest;
    }
    else if (select_op == "more") {
        select_operator = SelectOperator::largest;
    }
    else {
        std::cout << "Unsupported Operator\n";
        exit(234);
    }

    std::set<int> entities_set(entity_ids -> first -> begin(), entity_ids -> first -> end());
    std::vector<std::pair<int, std::shared_ptr<BaseValue>>> candidates;

    for (const auto & entity_id : entities_set) {
        const auto & entity_attributes = _entity_attribute[entity_id];
        if (entity_attributes.find(attribute_key) != entity_attributes.end()){
            for (const auto & entity_att : entity_attributes.at(attribute_key)) {
                if (entity_att -> attribute_value -> type == BaseValue::int_type || entity_att -> attribute_value -> type == BaseValue::float_type) {
                    candidates.emplace_back(entity_id, entity_att -> attribute_value);
                }
            }
        }
    }

    std::map<std::string, int> unit_cnt;
    for (const auto & candidate_pair : candidates) {
        auto & unit = ((QuantityValue*)(candidate_pair.second.get())) -> unit;
        if (unit_cnt.find(unit) == unit_cnt.end()) {
            unit_cnt[unit] = 0;
        }
        unit_cnt[unit]++;
    }
    int max_num = -1; std::string max_unit;
    for (const auto & unit_pair : unit_cnt) {
        if (unit_pair.second > max_num) {
            max_unit = unit_pair.first;
            max_num = unit_pair.second;
        }
    }

    double max_value = std::numeric_limits<double>::min(), min_value = std::numeric_limits<double>::max();
    for (const auto & candidate_pair : candidates) {
        auto & unit = ((QuantityValue*)(candidate_pair.second.get())) -> unit;
        if (unit != max_unit) continue;

        auto & value = ((QuantityValue*)(candidate_pair.second.get())) -> value;
        if (max_value < value) { max_value = value; }
        if (min_value > value) { min_value = value; }
    }

    double final_value = 0.0;
    if (select_operator == SelectOperator::smallest){
        final_value = min_value;
    } else {
        final_value = max_value;
    }

    for (const auto & candidate_pair : candidates) {
        auto & unit = ((QuantityValue*)(candidate_pair.second.get())) -> unit;
        if (unit != max_unit) continue;
        auto value = ((QuantityValue*)(candidate_pair.second.get())) -> value;
        if (value == final_value) {
            return_ptr -> push_back(&_entity_name[candidate_pair.first]);
        }
    }

    return return_ptr;
}



std::shared_ptr<std::vector<const std::string *>>
Engine::selectBetween(
        const std::shared_ptr<EntitiesWithFacts> & entity_ids_a,
        const std::shared_ptr<EntitiesWithFacts> & entity_ids_b,
        const std::string & attribute_key,
        const std::string & select_op) const {

    auto compare_among_entities = std::make_shared<EntitiesWithFacts>();
    compare_among_entities -> first = std::make_shared<Entities>();
    compare_among_entities -> first -> insert(compare_among_entities -> first -> end(), entity_ids_a -> first -> begin(), entity_ids_a -> first -> end());
    compare_among_entities -> first -> insert(compare_among_entities -> first -> end(), entity_ids_b -> first -> begin(), entity_ids_b -> first -> end());


    return selectAmong(compare_among_entities, attribute_key, select_op);
}


std::shared_ptr<Engine::EntitiesWithFacts>
Engine::QfilterStr(
        const std::shared_ptr<EntitiesWithFacts> & entity_with_fact,
        const std::string & qualifier_string_key,
        const std::string & qualifier_string_value) const {
    auto value_to_compare = std::make_shared<StringValue>(qualifier_string_value);
    return _filter_qualifier(entity_with_fact, qualifier_string_key, value_to_compare, "=");
}

std::shared_ptr<Engine::EntitiesWithFacts>
Engine::QfilterNum(
        const std::shared_ptr<EntitiesWithFacts> & entity_with_fact,
        const std::string & qualifier_num_key,
        const std::string & qualifier_num_value,
        const std::string & op) const {
    auto value_to_compare = std::make_shared<QuantityValue>(qualifier_num_value);
    return _filter_qualifier(entity_with_fact, qualifier_num_key, value_to_compare, op);
}

std::shared_ptr<Engine::EntitiesWithFacts>
Engine::QfilterYear(
        const std::shared_ptr<EntitiesWithFacts> & entity_with_fact,
        const std::string & qualifier_year_key,
        const std::string & qualifier_year_value,
        const std::string & op) const {
    auto value_to_compare = std::make_shared<YearValue>(qualifier_year_value);
    return _filter_qualifier(entity_with_fact, qualifier_year_key, value_to_compare, op);
}

std::shared_ptr<Engine::EntitiesWithFacts>
Engine::QfilterDate(
        const std::shared_ptr<EntitiesWithFacts> & entity_with_fact,
        const std::string & qualifier_date_key,
        const std::string & qualifier_date_value,
        const std::string & op) const {
    auto value_to_compare = std::make_shared<DateValue>(qualifier_date_value);
    return _filter_qualifier(entity_with_fact, qualifier_date_key, value_to_compare, op);
}

std::shared_ptr<Engine::EntitiesWithFacts> Engine::relateOp(
        const std::shared_ptr<EntitiesWithFacts> & entities,
        const std::string & relation_name,
        const std::string & relation_direction) {

    RelationDirection rel_dir;
    if (relation_direction == "forward") {
        rel_dir = RelationDirection::forward;
    }
    else if (relation_direction == "backward") {
        rel_dir = RelationDirection::backward;
    }
    else {
        std::cout << "Relation Direction Error!\n";
        exit(125);
    }

    auto related_entities_ptr = std::make_shared<EntitiesWithFacts>();
    related_entities_ptr -> first  = std::make_shared<Entities>();
    related_entities_ptr -> second = std::make_shared<Facts>();

    RelationIndex rel_index(relation_name, rel_dir);

    std::cout << "Here!\n";

    if (_relation_in_entity_index.find(rel_index) != _relation_in_entity_index.end()) {
        const auto & relation_index = _relation_in_entity_index.at(rel_index);

        for (const auto & head_entity : *(entities -> first)) {
            if (relation_index.find(head_entity) != relation_index.end()) {
                const auto & rel_poses = relation_index.at(head_entity);

                for (const auto & rel_pos : rel_poses) {
                    const auto & relation_info = _entity_relation[head_entity][rel_pos];
                    int tail_entity = relation_info -> relation_tail_entity;
                    related_entities_ptr -> first -> push_back(tail_entity);
                    related_entities_ptr -> second -> push_back(relation_info);
                }
            }
        }
    }
    return related_entities_ptr;
}

std::shared_ptr<Engine::EntitiesWithFacts> Engine::andOp(
        const std::shared_ptr<EntitiesWithFacts> & a,
        const std::shared_ptr<EntitiesWithFacts> & b) {

    auto intersection_entities_ptr = std::make_shared<Entities>();

    std::sort(a -> first -> begin(), a -> first -> end());
    std::sort(b -> first -> begin(), b -> first -> end());
    std::set_intersection(a -> first -> begin(), a -> first -> end(), b -> first -> begin(), b -> first -> end(), std::back_inserter(*intersection_entities_ptr));

    auto ret_ptr = std::make_shared<EntitiesWithFacts>();
    ret_ptr -> first = intersection_entities_ptr;
    return ret_ptr;
}

std::shared_ptr<Engine::EntitiesWithFacts> Engine::orOp(
        const std::shared_ptr<EntitiesWithFacts> & a,
        const std::shared_ptr<EntitiesWithFacts> & b) {
    auto union_entities_ptr = std::make_shared<Entities>();

    std::sort(a -> first -> begin(), a -> first -> end());
    std::sort(b -> first -> begin(), b -> first -> end());
    std::set_union(a -> first -> begin(), a -> first -> end(), b -> first -> begin(), b -> first -> end(), std::back_inserter(*union_entities_ptr));

    auto ret_ptr = std::make_shared<EntitiesWithFacts>();
    ret_ptr -> first = union_entities_ptr;
    return ret_ptr;
}

int Engine::countOp(
        const std::shared_ptr<EntitiesWithFacts> & entities) {
    return (int)(entities -> first -> size());
}







