#include "engine.h"

void Engine::_parseQualifier(Qualifiers & qualifier_output, const json & qualifier_json) {
    for (const auto& qualifier : qualifier_json.items()) {
        std::string qualifier_key_string(qualifier.key());

        for (const auto & single_qualifier : qualifier.value()) {
            std::shared_ptr<BaseValue> val_ptr;
            BaseValue::parseValue(val_ptr, single_qualifier);
            qualifier_output[qualifier_key_string].push_back(val_ptr);
//            _key_type[qualifier_key_string] = val_ptr -> type;
            _addKeyType(qualifier_key_string, val_ptr -> type);
        }
    }
}

void Engine::_addKeyType(const std::string &key, unsigned short type) {
    if (type != BaseValue::year_type) {
        _key_type[key] = type;
    }
    else {
        _key_type[key] = BaseValue::date_type;
    }
}

void Engine::_addFindAllFilterIndex(
        const std::string & attribute_key,
        const std::shared_ptr<Attribute> & attribute,
        int cur_entity_number) {
    if (attribute -> attribute_value -> type == BaseValue::string_type) {
        std::string index_key{attribute_key};
        index_key += "__$$__";
        index_key += (attribute -> attribute_value -> toPrintStr());

        if (_find_all_filter_str_index.find(index_key) == _find_all_filter_str_index.end()) {
            _find_all_filter_str_index[index_key] = std::make_shared<EntitiesWithFacts>();
            _find_all_filter_str_index[index_key] -> first = std::make_shared<Entities>();
            _find_all_filter_str_index[index_key] -> second = std::make_shared<Facts>();
        }

        _find_all_filter_str_index[index_key] -> first -> push_back(cur_entity_number);
        _find_all_filter_str_index[index_key] -> second -> push_back(attribute);
    }
    else if (attribute -> attribute_value -> type == BaseValue::int_type || attribute -> attribute_value -> type == BaseValue::float_type) {
        auto quan_ptr = std::dynamic_pointer_cast<QuantityValue>(attribute -> attribute_value);

        double num_value = quan_ptr -> value;
        auto num_unit = quan_ptr -> unit;
        
        std::string index_key{attribute_key};
        index_key += "__$$__";
        index_key += num_unit;
        
        if (_find_all_filter_num_index.find(index_key) == _find_all_filter_num_index.end()) {
            _find_all_filter_num_index[index_key] = std::make_shared<NumIndex>();
            
        }
        if (_find_all_filter_num_index[index_key] -> find(num_value) == _find_all_filter_num_index[index_key] -> end()) {
            (*_find_all_filter_num_index[index_key])[num_value] = std::make_shared<EntitiesWithFacts>();
            (*_find_all_filter_num_index[index_key])[num_value] -> first = std::make_shared<Entities>();
            (*_find_all_filter_num_index[index_key])[num_value] -> second = std::make_shared<Facts>();
        }

        (*_find_all_filter_num_index[index_key])[num_value] -> first -> push_back(cur_entity_number);
        (*_find_all_filter_num_index[index_key])[num_value] -> second -> push_back(attribute);
    }
    else if (attribute -> attribute_value -> type == BaseValue::year_type) {
        auto year_key = attribute_key;
        auto year_value_ptr = std::dynamic_pointer_cast<YearValue>(attribute -> attribute_value);

        if (_find_all_filter_year_index.find(year_key) == _find_all_filter_year_index.end()) {
            _find_all_filter_year_index[year_key] = std::make_shared<YearIndex>();
        }
        if (_find_all_filter_year_index[year_key] -> find(*year_value_ptr) == _find_all_filter_year_index[year_key] -> end()) {
            (*_find_all_filter_year_index[year_key])[*year_value_ptr] = std::make_shared<EntitiesWithFacts>();
            (*_find_all_filter_year_index[year_key])[*year_value_ptr] -> first = std::make_shared<Entities>();
            (*_find_all_filter_year_index[year_key])[*year_value_ptr] -> second = std::make_shared<Facts>();
        }
        (*_find_all_filter_year_index[year_key])[*year_value_ptr] -> first -> push_back(cur_entity_number);
        (*_find_all_filter_year_index[year_key])[*year_value_ptr] -> second -> push_back(attribute);


    }
    else if (attribute -> attribute_value -> type == BaseValue::date_type) {
        auto date_key = attribute_key;
        auto date_value_ptr = std::dynamic_pointer_cast<DateValue>(attribute -> attribute_value);

        if (_find_all_filter_date_index.find(date_key) == _find_all_filter_date_index.end()) {
            _find_all_filter_date_index[date_key] = std::make_shared<DateIndex>();
        }
        if (_find_all_filter_date_index[date_key] -> find(*date_value_ptr) == _find_all_filter_date_index[date_key] -> end()) {
            (*_find_all_filter_date_index[date_key])[*date_value_ptr] = std::make_shared<EntitiesWithFacts>();
            (*_find_all_filter_date_index[date_key])[*date_value_ptr] -> first = std::make_shared<Entities>();
            (*_find_all_filter_date_index[date_key])[*date_value_ptr] -> second = std::make_shared<Facts>();
        }
        (*_find_all_filter_date_index[date_key])[*date_value_ptr] -> first -> push_back(cur_entity_number);
        (*_find_all_filter_date_index[date_key])[*date_value_ptr] -> second -> push_back(attribute);
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
    _concept_name           .reserve(total_concept_num);
    _concept_sub_class_of   .reserve(total_concept_num);

    _entity_name            .reserve(total_entity_num + total_concept_num);
    _entity_is_instance_of  .reserve(total_entity_num + total_concept_num);
    _entity_attribute       .reserve(total_entity_num + total_concept_num);
    _entity_relation        .reserve(total_entity_num + total_concept_num);
    serialStringsOfAttributes.reserve(total_entity_num + total_concept_num);
    jsonsOfRelations .reserve(total_entity_num + total_concept_num);


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

        // Treat Concepts as Entities !!! Version 2.0 KG
        _entity_id.push_back(concept_id);
        _entity_id_to_number[concept_id] = (int)_entity_id.size() - 1;

        _entity_name.push_back(concept_name);
        if (_entity_name_to_number.find(concept_name) == _entity_name_to_number.end()) {
            _entity_name_to_number[concept_name] = std::make_shared<Entities>();
        }
        _entity_name_to_number[concept_name] -> push_back((int)_entity_name.size() - 1);

        // Reserve Space for _entity_attribute, _entity_relation, _entity_is_instance_of
        _entity_attribute.emplace_back();
        _entity_relation.emplace_back();
        serialStringsOfAttributes.emplace_back("[]");
        jsonsOfRelations.emplace_back();
        _entity_is_instance_of.emplace_back();
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
        for (const auto & concept_id : entity.value().at("instanceOf")) {
            auto concept_id_in_string = concept_id.get<std::string>();
            int concept_number = _concept_id_to_number[concept_id_in_string];
            _entity_is_instance_of[cur_entity_number].insert(concept_number);

            // Fix - Add to super concepts
            std::set<int> sup_concept_set = {concept_number};
//            sup_concept_set.insert(concept_number);
            bool find_sup = true;
            while (find_sup) {
                find_sup = false;
                for (const auto & con : sup_concept_set) {
                    for (const auto & sup_con : _concept_sub_class_of[con]) {
                        if (sup_concept_set.find(sup_con) == sup_concept_set.end()) {
                            find_sup = true;
                            sup_concept_set.insert(sup_con);
                        }
                    }
                }
            }
            for (const auto & con : sup_concept_set) {
                _concept_has_instance_entities[con].insert(cur_entity_number);
            }
        }

        // For "_entity_attribute"
        std::unordered_map<std::string, std::vector<std::shared_ptr<Attribute>>> ent_attrs;
        for (const auto& attribute_json : entity.value().at("attributes")) {
            auto attribute = std::make_shared<Attribute>();
            // obtain key
            auto attribute_key = attribute_json.at("key").get<std::string>();

            // construct inverted index for attribute keys
            _attribute_key_to_entities[attribute_key].insert(cur_entity_number);
            // obtain value
            auto & type_value_unit = attribute_json.at("value");
            BaseValue::parseValue(attribute -> attribute_value, type_value_unit);
            _addKeyType(attribute_key, attribute -> attribute_value -> type);
            _addFindAllFilterIndex(attribute_key, attribute, cur_entity_number);
            // obtain qualifiers
            _parseQualifier(attribute -> fact_qualifiers, attribute_json.at("qualifiers"));

            ent_attrs[attribute_key].push_back(attribute);
        }
        _entity_attribute.push_back( ent_attrs );
        serialStringsOfAttributes.push_back( entity.value().at("attributes").dump() );
    }

    // Construct "_entity_relation"
    for (const auto & entity : entity_json.items()) {
        std::vector<std::shared_ptr<Relation>> relation;
        std::map<int, json> relationJsons;

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
            relationJsons[tail_entity_number] = relation_json;

            RelationIndex relation_index(relation_name_string, relation_direction);
            EntityPairIndex entity_pair_index((int)_entity_relation.size(), tail_entity_number);

//            _relation_to_entity_pair[relation_index].push_back(entity_pair_index);
            _entity_pair_to_relation[entity_pair_index].push_back(relation_index);

            auto rel_idx = (int)relation.size() - 1;
            _relation_in_entity_index[relation_index][entity_pair_index.head_entity].push_back(rel_idx);
            // Fix Logic bug: _entity_forward_relation stores **forward** relation only!!!
            if (rel -> relation_direction == RelationDirection::forward) {
                _entity_forward_relation_index[entity_pair_index].push_back(rel_idx);
            }
        }
        _entity_relation.push_back(relation);
        jsonsOfRelations.push_back( relationJsons );
    }

    std::cout << "Construct The Inverse Relation\n";
    // Construct Inverse Relation TODO: Test
    for (std::size_t head_entity = _concept_id.size(); head_entity < _entity_relation.size(); head_entity++) {  // 前半部分都是 concept
        const auto & relations = _entity_relation[head_entity];

        for (const auto & relation : relations) {
            // Get Inverse Direction and Tail Entities
            RelationDirection inv_direction;
            if (relation -> relation_direction == RelationDirection::forward)   inv_direction = RelationDirection::backward;
            else                                                                inv_direction = RelationDirection::forward;
            const auto tail_entity = relation -> relation_tail_entity;

            // Check Whether the inverse relation exists already
            EntityPairIndex inv_entity_pair_index((int)tail_entity, (int)head_entity);
            RelationIndex inv_relation_index(relation -> relation_name, inv_direction);

            bool inv_exist = false;
            if (_entity_pair_to_relation.find(inv_entity_pair_index) != _entity_pair_to_relation.end()) {
                for (const auto relation_index : _entity_pair_to_relation.at(inv_entity_pair_index)) {
                    if (relation_index.relation_name == inv_relation_index.relation_name &&
                        relation_index.relation_direction == inv_relation_index.relation_direction) {
                        inv_exist = true;
                        break;
                    }
                }
            }

            // Add the inverse relation if it does not exist
            if (!inv_exist) {
                auto inv_rel = std::make_shared<Relation>();
                inv_rel -> relation_name = relation -> relation_name;
                inv_rel -> relation_direction = inv_direction;
                inv_rel -> relation_tail_entity = head_entity;

                for (const auto & qualifiers : relation -> fact_qualifiers) {
                    const auto & qualifier_key = qualifiers.first;
                    for (const auto & qualifier_value : qualifiers.second) {
                        (inv_rel -> fact_qualifiers)[qualifier_key].push_back(qualifier_value);
                    }
                }

                json inverse_relation_json;
                inverse_relation_json["relation"] = inv_rel -> relation_name;
                inverse_relation_json["direction"] = inv_rel -> relation_direction == RelationDirection::forward ?
                        "forward" : "backward";
                inverse_relation_json["object"] = _entity_id.at(head_entity);
                inverse_relation_json["qualifiers"] = jsonsOfRelations.at(head_entity).at(tail_entity).at("qualifiers");

                jsonsOfRelations[tail_entity][head_entity] = inverse_relation_json;

                _entity_relation[tail_entity].push_back(inv_rel);
                auto inv_rel_idx = (int)_entity_relation[tail_entity].size() - 1;


                // Add Index for the inverse relation as well
//                _relation_to_entity_pair[inv_relation_index].push_back(inv_entity_pair_index);
                _entity_pair_to_relation[inv_entity_pair_index].push_back(inv_relation_index);


                _relation_in_entity_index[inv_relation_index][inv_entity_pair_index.head_entity].push_back(inv_rel_idx);

                // Fix Logic bug: _entity_forward_relation stores **forward** relation only!!!
                if (inv_direction == RelationDirection::forward) {
                    _entity_forward_relation_index[inv_entity_pair_index].push_back(inv_rel_idx);
                }
            }
        }
    }


    // Sort Entities of the same name by their frequencies
    std::cout << "Sort Entities\n";
    for (auto & entity_name_num : _entity_name_to_number) {
        auto & entity_nums = entity_name_num.second;
        if (entity_nums -> size() > 1) {
            std::sort(entity_nums -> begin(), entity_nums -> end(),
                      [this](int const & a, int const & b) {
                          return _entity_relation[a].size() > _entity_relation[b].size();
                      });
            entity_nums -> erase(entity_nums -> begin() + 1, entity_nums -> end());
        }
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
/*    std::ofstream year_index("date_index.txt", std::ios::out);
    for (const auto year_key_entry : _find_all_filter_date_index) {
        year_index << year_key_entry.first << std::endl;
        if (year_key_entry.second.get()) {
            for (const auto year_value_entry : *year_key_entry.second) {
                auto tmp = year_value_entry.first.toPrintStr();
                year_index << "    " << tmp;
                if (year_value_entry.second.get() && year_value_entry.second->first.get()) {
                    for (const auto _id : *(year_value_entry.second->first)) {
                        year_index << " " << _id;
                    }
                }
                year_index << std::endl;
            }
        }
        year_index << std::endl;
    }
    year_index.close();
    */
}

Engine::~Engine() {
    // Release _entity_name_to_number
    for (auto entity_name_number_kv_pair : _entity_name_to_number) {
        entity_name_number_kv_pair.second.reset();
    }

    // Release _entity_attribute
    for (auto str_attr_map : _entity_attribute) {
        for (auto str_attr_kv_pair : str_attr_map) {
            for (auto attr_ptr : str_attr_kv_pair.second) {
                attr_ptr->attribute_value.reset();
                attr_ptr.reset();
            }
        }
    }
    
    // Release _entity_relation
    for (auto rela_vec : _entity_relation) {
        for (auto rela_ptr : rela_vec) {
            rela_ptr.reset();
        }
    }

    // Release _all_entities
    _all_entities->first.reset();
    _all_entities->second.reset();
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
                        // Zijun: add break to Fix Bug, to avoid repeating the same attributes
                        break;
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

    std::set<int> entity_ids_set;
    for (const auto x : *(entity_ids -> first)) {
        entity_ids_set.insert(x);
    }

    std::vector<int> valid_entity_ids;
    std::set_intersection(entity_ids_set.begin(), entity_ids_set.end(), _attribute_key_to_entities.at(key).begin(), _attribute_key_to_entities.at(key).end(), std::back_inserter(valid_entity_ids));

    // enumerate over all entities
//    for (const auto & ent : *(entity_ids -> first)) {
    for (const auto & ent : valid_entity_ids) {
//        if (_attribute_key_to_entities.at(key).find(ent) != _attribute_key_to_entities.at(key).end()) {

            const auto & entity_attributes = _entity_attribute[ent];

            // enumerate over all attributes with satisfying key
//            if (entity_attributes.find(key) != entity_attributes.end()) {
                for (const auto & entity_att : entity_attributes.at(key)) {

                    // save the fact that satisfying the filtering condition
                    if (entity_att -> attribute_value -> valueCompare(value_to_compare.get(), op)) {
                        entity_with_fact_ptr -> first -> push_back(ent);
                        entity_with_fact_ptr -> second -> push_back(entity_att);
                    }
                }
//            }

//        }
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
    auto res_ptr = std::make_shared<EntitiesWithFacts>();
    if (_entity_name_to_number.find(find_entity_name) != _entity_name_to_number.end()) {
        res_ptr -> first = _entity_name_to_number.at(find_entity_name);
    }
    else {
        res_ptr -> first = std::make_shared<Entities>();
    }
    return res_ptr;
}

std::shared_ptr<Engine::EntitiesWithFacts> Engine::findLinking(const std::string &find_entity_name) const {
    auto res_ptr = std::make_shared<EntitiesWithFacts>();
    res_ptr -> first = std::make_shared<Entities>();
    std::string s = "";
    for (auto i = 0; i < find_entity_name.length(); ++i){
        if(find_entity_name[i] == ' '){
            if (_entity_id_to_number.find(s) != _entity_id_to_number.end()) {
                res_ptr -> first ->push_back(_entity_id_to_number.at(s));
            }
            s = "";
        }else{
            s += find_entity_name[i];
        }
        if (_entity_id_to_number.find(s) != _entity_id_to_number.end()) {
            res_ptr -> first ->push_back(_entity_id_to_number.at(s));
        }
    }
    return res_ptr;
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
    for (const auto & concept_num : concept_numbers) {
        auto concept_inst = _concept_has_instance_entities.at(concept_num);
        entity_set.insert(concept_inst.begin(), concept_inst.end());
    }

    auto output_entities = std::make_shared<EntitiesWithFacts>();
    output_entities -> first = std::make_shared<Entities>();

    std::set<int> entity_ids_set;
    entity_ids_set.insert(entity_ids -> first -> begin(), entity_ids -> first -> end());

    std::set_intersection(entity_ids_set.begin(), entity_ids_set.end(), entity_set.begin(), entity_set.end(), std::back_inserter(* output_entities -> first));

    return output_entities;
}

std::shared_ptr<Engine::EntitiesWithFacts>
Engine::findAllFilterConcept(
        const std::string & concept_name) const {

    auto concept_numbers = std::vector<int>();
    if (_concept_name_to_number.find(concept_name) != _concept_name_to_number.end()) {
        concept_numbers = _concept_name_to_number.at(concept_name);
    }

    auto output_entities = std::make_shared<EntitiesWithFacts>();
    output_entities -> first = std::make_shared<Entities>();

    for (const auto & concept_num : concept_numbers) {
        auto concept_inst = _concept_has_instance_entities.at(concept_num);
        output_entities -> first -> insert(output_entities -> first -> end(), concept_inst.begin(), concept_inst.end());
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
Engine::findAllFilterStr(
        const std::string & string_key,
        const std::string & string_value) const {
    std::shared_ptr<EntitiesWithFacts> entity_with_fact_ptr;

    std::string index_key{string_key};
    index_key += "__$$__";
    index_key += string_value;

    if (_find_all_filter_str_index.find(index_key) != _find_all_filter_str_index.end()) {
        return _find_all_filter_str_index.at(index_key);
    }
    else {
        entity_with_fact_ptr = std::make_shared<EntitiesWithFacts>();
        entity_with_fact_ptr -> first = std::make_shared<Entities>();
        entity_with_fact_ptr -> second = std::make_shared<Facts>();
    }

    return entity_with_fact_ptr;
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
Engine::findAllFilterNum(
        const std::string & number_key,
        const std::string & number_value,
        const std::string & op) const {
    auto pos = number_value.find_first_of(' ');
    std::string number_digits, number_unit;
    if (pos != std::string::npos) {
        number_digits = number_value.substr(0, pos);
        number_unit = number_value.substr(pos + 1);
    }
    else {
        number_digits = number_value;
        number_unit = "1";
    }
    
    auto digits = std::atof(number_digits.c_str());

    std::string index_key{number_key};
    index_key += "__$$__";
    index_key += number_unit;

    std::shared_ptr<EntitiesWithFacts> entity_with_fact_ptr;
    entity_with_fact_ptr = std::make_shared<EntitiesWithFacts>();
    entity_with_fact_ptr -> first = std::make_shared<Entities>();
    entity_with_fact_ptr -> second = std::make_shared<Facts>();

    if (_find_all_filter_num_index.find(index_key) != _find_all_filter_num_index.end()) {
        if (op[0] == '=') {
            if(_find_all_filter_num_index.at(index_key) -> find(digits) != _find_all_filter_num_index.at(index_key) -> end()) {
                return _find_all_filter_num_index.at(index_key) -> at(digits);
            }
        }
        else {
            // To process "!=", combine methods of processing "<" and ">"
            if (op == "<" || op == "!=") {
                auto lowerBound = _find_all_filter_num_index.at(index_key) -> lower_bound(digits);
                for (auto it = _find_all_filter_num_index.at(index_key) -> begin(); it != lowerBound; ++it) {
                    entity_with_fact_ptr -> first -> insert(
                        entity_with_fact_ptr -> first -> end(),
                        it -> second -> first -> begin(),
                        it -> second -> first -> end()
                    );
                    entity_with_fact_ptr -> second -> insert(
                        entity_with_fact_ptr -> second -> end(),
                        it -> second -> second -> begin(),
                        it -> second -> second -> end()
                    );
                }
            }
            if (op == ">" || op == "!=") {
                auto upperBound = _find_all_filter_num_index.at(index_key) -> upper_bound(digits);
                for (auto it = upperBound; it != _find_all_filter_num_index.at(index_key) -> end(); ++it) {
                    entity_with_fact_ptr -> first -> insert(
                        entity_with_fact_ptr -> first -> end(),
                        it -> second -> first -> begin(),
                        it -> second -> first -> end()
                    );
                    entity_with_fact_ptr -> second -> insert(
                        entity_with_fact_ptr -> second -> end(),
                        it -> second -> second -> begin(),
                        it -> second -> second -> end()
                    );
                }
            }
        }
    }

    return entity_with_fact_ptr;
}

std::shared_ptr<Engine::EntitiesWithFacts>
Engine::filterYear(
        const std::shared_ptr<EntitiesWithFacts> & entity_ids,
        const std::string & year_key,
        const std::string & year_value,
        const std::string & op) const {
//    std::cout << year_key << " " << year_value << " " << op << std::endl;
    auto value_to_compare = std::make_shared<YearValue>(year_value);
    auto return_pairs = _filter_attribute(entity_ids, year_key, value_to_compare, op);
    return return_pairs;
}

std::shared_ptr<Engine::EntitiesWithFacts>
Engine::findAllFilterYear(
        const std::string & year_key,
        const std::string &year_value,
        const std::string &op) const {
    std::shared_ptr<EntitiesWithFacts> entity_with_fact_ptr;
    entity_with_fact_ptr = std::make_shared<EntitiesWithFacts>();
    entity_with_fact_ptr -> first = std::make_shared<Entities>();
    entity_with_fact_ptr -> second = std::make_shared<Facts>();

//    std::ofstream search_year_pair("search_year_pair.txt", std::ios::app);
//    search_year_pair << year_key << " " << year_value << " " << op << std::endl;
//    search_year_pair.close();

    auto value_to_compare = YearValue(year_value);

    if (_find_all_filter_year_index.find(year_key) != _find_all_filter_year_index.end()) {
        if (op[0] == '=') {
            if (_find_all_filter_year_index.at(year_key) -> find(value_to_compare) != _find_all_filter_year_index.at(year_key) -> end()) {
                entity_with_fact_ptr -> first -> insert(
                        entity_with_fact_ptr -> first -> end(),
                        _find_all_filter_year_index.at(year_key) -> at(value_to_compare) -> first -> begin(),
                        _find_all_filter_year_index.at(year_key) -> at(value_to_compare) -> first -> end()
                );
                entity_with_fact_ptr -> second -> insert(
                        entity_with_fact_ptr -> second -> end(),
                        _find_all_filter_year_index.at(year_key) -> at(value_to_compare) -> second -> begin(),
                        _find_all_filter_year_index.at(year_key) -> at(value_to_compare) -> second -> end()
                );
            }
        }
        else {
            // To process "!=", combine methods of processing "<" and ">"
            if (op == "<" || op == "!=") {
                auto lowerBound = _find_all_filter_year_index.at(year_key) -> lower_bound(value_to_compare);
                for (auto it = _find_all_filter_year_index.at(year_key) -> begin(); it != lowerBound; ++it) {
                    entity_with_fact_ptr -> first -> insert(
                            entity_with_fact_ptr -> first -> end(),
                            it -> second -> first -> begin(),
                            it -> second -> first -> end()
                    );
                    entity_with_fact_ptr -> second -> insert(
                            entity_with_fact_ptr -> second -> end(),
                            it -> second -> second -> begin(),
                            it -> second -> second -> end()
                    );
                }
            }
            if (op == ">" || op == "!=") {
                auto upperBound = _find_all_filter_year_index.at(year_key) -> upper_bound(value_to_compare);
                for (auto it = upperBound; it != _find_all_filter_year_index.at(year_key) -> end(); ++it) {
                    entity_with_fact_ptr -> first -> insert(
                            entity_with_fact_ptr -> first -> end(),
                            it -> second -> first -> begin(),
                            it -> second -> first -> end()
                    );
                    entity_with_fact_ptr -> second -> insert(
                            entity_with_fact_ptr -> second -> end(),
                            it -> second -> second -> begin(),
                            it -> second -> second -> end()
                    );
                }
            }
        }
    }

    if (year_value[0] == '-') {
        return entity_with_fact_ptr;
    }

    auto dateLowerBound = DateValue(year_value + "-01-01");
    auto dateUpperBound = DateValue(year_value + "-12-31");
    auto date_key = year_key;

    if (_find_all_filter_date_index.find(date_key) != _find_all_filter_date_index.end()) {
        if (op[0] == '=') {
            auto lowerBound = _find_all_filter_date_index.at(date_key) -> lower_bound(dateLowerBound);
            auto upperBound = _find_all_filter_date_index.at(date_key) -> upper_bound(dateUpperBound);
            for (auto it = lowerBound; it != upperBound; ++it) {
                entity_with_fact_ptr -> first -> insert(
                        entity_with_fact_ptr -> first -> end(),
                        it -> second -> first -> begin(),
                        it -> second -> first -> end()
                );
                entity_with_fact_ptr -> second -> insert(
                        entity_with_fact_ptr -> second -> end(),
                        it -> second -> second -> begin(),
                        it -> second -> second -> end()
                );
            }
        }
        else {
            // To process "!=", combine methods of process "<" and ">"
            if (op == "<" || op == "!=") {
                auto lowerBound = _find_all_filter_date_index.at(date_key) -> lower_bound(dateLowerBound);
                for (auto it = _find_all_filter_date_index.at(date_key) -> begin(); it != lowerBound; ++it) {
                    entity_with_fact_ptr -> first -> insert(
                            entity_with_fact_ptr -> first -> end(),
                            it -> second -> first -> begin(),
                            it -> second -> first -> end()
                    );
                    entity_with_fact_ptr -> second -> insert(
                            entity_with_fact_ptr -> second -> end(),
                            it -> second -> second -> begin(),
                            it -> second -> second -> end()
                    );
                }
            }
            if (op == ">" || op == "!=") {
                auto upperBound = _find_all_filter_date_index.at(date_key) -> upper_bound(dateUpperBound);
                for (auto it = upperBound; it != _find_all_filter_date_index.at(date_key) -> end(); ++it) {
                    entity_with_fact_ptr -> first -> insert(
                            entity_with_fact_ptr -> first -> end(),
                            it -> second -> first -> begin(),
                            it -> second -> first -> end()
                    );
                    entity_with_fact_ptr -> second -> insert(
                            entity_with_fact_ptr -> second -> end(),
                            it -> second -> second -> begin(),
                            it -> second -> second -> end()
                    );
                }
            }
        }
    }

    return entity_with_fact_ptr;
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

std::shared_ptr<Engine::EntitiesWithFacts>
Engine::findAllFilterDate(
        const std::string & date_key,
        const std::string & date_value,
        const std::string & op) const {
    std::shared_ptr<EntitiesWithFacts> entity_with_fact_ptr;
    entity_with_fact_ptr = std::make_shared<EntitiesWithFacts>();
    entity_with_fact_ptr -> first = std::make_shared<Entities>();
    entity_with_fact_ptr -> second = std::make_shared<Facts>();

    auto value_to_compare = DateValue(date_value);

    if (_find_all_filter_date_index.find(date_key) != _find_all_filter_date_index.end()) {
        if (op[0] == '=') {
            if (_find_all_filter_date_index.at(date_key) -> find(value_to_compare) != _find_all_filter_date_index.at(date_key) -> end()) {
                return _find_all_filter_date_index.at(date_key) -> at(value_to_compare);
            }
        }
        else {
            // To process "!=", combine methods of process "<" and ">"
            if (op == "<" || op == "!=") {
                auto lowerBound = _find_all_filter_date_index.at(date_key) -> lower_bound(value_to_compare);
                for (auto it = _find_all_filter_date_index.at(date_key) -> begin(); it != lowerBound; ++it) {
                    entity_with_fact_ptr -> first -> insert(
                            entity_with_fact_ptr -> first -> end(),
                            it -> second -> first -> begin(),
                            it -> second -> first -> end()
                    );
                    entity_with_fact_ptr -> second -> insert(
                            entity_with_fact_ptr -> second -> end(),
                            it -> second -> second -> begin(),
                            it -> second -> second -> end()
                    );
                }
            }
            if (op == ">" || op == "!=") {
                auto upperBound = _find_all_filter_date_index.at(date_key) -> upper_bound(value_to_compare);
                for (auto it = upperBound; it != _find_all_filter_date_index.at(date_key) -> end(); ++it) {
                    entity_with_fact_ptr -> first -> insert(
                            entity_with_fact_ptr -> first -> end(),
                            it -> second -> first -> begin(),
                            it -> second -> first -> end()
                    );
                    entity_with_fact_ptr -> second -> insert(
                            entity_with_fact_ptr -> second -> end(),
                            it -> second -> second -> begin(),
                            it -> second -> second -> end()
                    );
                }
            }
        }
    }

    return entity_with_fact_ptr;
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

        if (_entity_forward_relation_index.find(entity_pair) != _entity_forward_relation_index.end()) {
        for (const auto & forward_relation_index: _entity_forward_relation_index.at(entity_pair)) {
                const auto & relation = _entity_relation[entity_a][forward_relation_index];
                return_ptr ->push_back(&relation->relation_name);
        }}
    }}
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
        const std::string & relation_name,
        const std::string & qualifier_key) const {
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
    if (select_op == "less" || select_op == "smallest") {
        select_operator = SelectOperator::smallest;
    }
    else if (select_op == "greater" || select_op == "largest") {
        select_operator = SelectOperator::largest;
    }
    else {
        std::cout << "Unsupported Operator " << select_op << "\n";
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

    double max_value = std::numeric_limits<double>::lowest(), min_value = std::numeric_limits<double>::max();
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

std::shared_ptr<Engine::GraphContainer>
Engine::expandFromEntities(
        const std::shared_ptr<Engine::EntitiesWithFacts>& entities_with_fact,
        const int jump_limitation) const {
    auto subgraph_ptr = std::make_shared<GraphContainer>();

    auto visited_index = std::make_shared<std::set<int>>();
    auto entities = std::vector<int>();
    entities.reserve(entities_with_fact-> first -> size());
    //std::cout << _entity_relation.size() << std::endl;

    for (const auto & id : *(entities_with_fact -> first)) {
        entities.push_back(id);
    }

    //for (auto i : entities) {
    //    std::cout << i << " " << _entity_id[i] << std::endl;
    //}

    std::vector<std::vector<json>>   subsetOfRelations;
    std::map<int, int>               entityToIndex;

    // First, add given ids into the subgraph
    for (const auto ent : entities) {
        visited_index -> insert(ent);
        subgraph_ptr -> entity_ids.push_back(_entity_id.at(ent));
        subgraph_ptr -> entity_attributes.push_back(serialStringsOfAttributes.at(ent));
        subgraph_ptr -> entity_relations.push_back("[]");
        subsetOfRelations.emplace_back();
        entityToIndex[ent] = subsetOfRelations.size() - 1;
    }

    if (jump_limitation <= 0) {
        return subgraph_ptr;
    }

    for (int i = 0; i < entities.size(); ++i) {
        for (const auto & relation : _entity_relation.at(entities[i])) {
            dfsTraversal(relation -> relation_tail_entity, entities[i], entityToIndex, jump_limitation, 1, subsetOfRelations, visited_index, subgraph_ptr);
        }
    }

    subgraph_ptr -> entity_relations.clear();
    
    for (int i = 0; i < subsetOfRelations.size(); ++i) {
        json relations = json::array();
        for (const auto & relation_json : subsetOfRelations.at(i)) {
            relations.push_back(relation_json);
        }
        subgraph_ptr -> entity_relations.push_back(relations.dump());
    }

    return subgraph_ptr;
}

void Engine::dfsTraversal(
        int entity,
        int from_entity,
        std::map<int, int> & entityToIndex,
        int depth_limitation,
        int depth,
        std::vector<std::vector<json>> & subsetOfRelations,
        std::shared_ptr<std::set<int>> & visited_index,
        std::shared_ptr<GraphContainer> & subgraph_ptr) const {
    if (visited_index -> find(entity) == visited_index -> end()) {
        visited_index -> insert(entity);

        auto entity_id = _entity_id.at(entity);
        //std::cout << entity << " " << entity_id << std::endl;
        subgraph_ptr -> entity_ids.push_back(entity_id);
        const auto & attributes = serialStringsOfAttributes.at(entity);
        subgraph_ptr -> entity_attributes.push_back(attributes);
        subsetOfRelations.emplace_back();
        int len = subsetOfRelations.size();
        entityToIndex[entity] = len - 1;
        subsetOfRelations[len - 1].push_back(jsonsOfRelations.at(entity).at(from_entity));
        int from_index = entityToIndex[from_entity];
        subsetOfRelations[from_index].push_back(jsonsOfRelations.at(from_entity).at(entity));
    }

    if (depth >= depth_limitation) {
        return;
    }

    const auto & relations = _entity_relation.at(entity);
    for (const auto & rela : relations) {
        dfsTraversal(rela -> relation_tail_entity, entity, entityToIndex, depth_limitation, depth + 1, subsetOfRelations, visited_index, subgraph_ptr);
    }
}







