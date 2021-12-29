#include "engine.h"

void Engine::_parseQualifier(Qualifiers & qualifier_output, const json & qualifier_json) {
    for (const auto& qualifier : qualifier_json.items()) {
        std::string qualifier_key_string(qualifier.key());

        for (const auto& single_qualifier : qualifier.value()) {
            std::shared_ptr<BaseValue> val_ptr;
            BaseValue::parseValue(val_ptr, single_qualifier);
            qualifier_output[qualifier_key_string].push_back(val_ptr);
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
        exit(0);
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
    for (auto & concept : concept_json.items()) {
        std::string concept_id(concept.key());
        std::string concept_name(concept.value().at("name"));

        _concept_id.push_back(concept_id);
        _concept_id_to_number[concept_id] = (int)_concept_id.size() - 1;

        _concept_name.push_back(concept_name);
        _concept_name_to_number[concept_name].push_back((int)_concept_name.size() - 1);
    }

    // Construct "_concept_sub_class_of"
    for (auto & c: concept_json.items()) {
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
        _entity_name_to_number[entity_name].push_back((int)_entity_name.size() - 1);

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
        std::map<std::string, std::vector<Attribute>> ent_attrs;
        for (auto attribute_json : entity.value().at("attributes")) {
            Attribute attribute;
            // obtain key
            auto attribute_key = attribute_json.at("key").get<std::string>();
            // construct inverted index for attribute keys
            _attribute_key_to_entities[attribute_key].insert(cur_entity_number);
            // obtain value
            BaseValue::parseValue(attribute.attribute_value, attribute_json.at("value"));
            // obtain qualifiers
            _parseQualifier(attribute.attribute_qualifiers, attribute_json.at("qualifiers"));

            ent_attrs[attribute_key].push_back(attribute);
        }
        _entity_attribute.push_back(ent_attrs);
    }

    // Construct "_entity_relation"
    for (const auto & entity : entity_json.items()) {
        std::vector<Relation> relation;
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

            Relation rel;
            rel.relation_name = relation_name_string;
            rel.relation_direction = relation_direction;
            rel.relation_tail_entity = tail_entity_number;
            _parseQualifier(rel.relation_qualifier, relation_json.at("qualifiers"));

            relation.push_back(rel);

            RelationIndex relation_index(relation_name_string, relation_direction);
            EntityPairIndex entity_pair_index((int)_entity_relation.size(), tail_entity_number);

            _relation_to_entity_pair[relation_index].insert(entity_pair_index);
            _entity_pair_to_relation[entity_pair_index].insert(relation_index);
        }
        _entity_relation.push_back(relation);
    }

    _all_entities.reserve(total_entity_num);
    for (std::size_t i = 0; i < _entity_name.size(); i++) {
        _all_entities.push_back((int)i);
    }

    examineEntityAttribute();
    examineRelation();
    examineAttributeKeyIndex();
    examineRelationIndex();
    examineEntityPairIndex();

    std::cout << "End of initialize a new KB\n";
}

void Engine::examineEntityAttribute() const {
    for (std::size_t i = 0; i < _entity_name.size(); i++) {
        std::cout << "Name: " << _entity_name[i] << ", Taxonomy: ";
        for (auto tax : _entity_is_instance_of[i]) {
            std::cout << _concept_name[tax] << "|";
        }
        std::cout << std::endl;
        for (const auto & attr : _entity_attribute[i]) {
            auto attr_key = attr.first;
            auto attr_vals = attr.second;
            std::cout << "-- " << attr_key << ": " << std::endl;
            for (const auto & attr_val : attr_vals) {
                std::cout << "  -- " << attr_val.attribute_value->toPrintStr() << std::endl;
                for (const auto & qualifier : attr_val.attribute_qualifiers) {
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
            auto tail_entity_number = relation.relation_tail_entity;
            auto tail_entity = _entity_name[tail_entity_number];
            auto relation_name = relation.relation_name;
            auto relation_direction = relation.relation_direction;
            if (relation_direction == RelationDirection::forward) {
                std::cout << "  -->";
            }
            else {
                std::cout << "  <--";
            }
            std::cout << "(" << head_entity << ", " << relation_name << ", " << tail_entity << ")" << std::endl;
            for (const auto & qualifiers : relation.relation_qualifier) {
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

std::shared_ptr<Engine::EntitiesWithFact>
Engine::_filter_attribute(const Engine::Entities & entities, const std::string &key, const BaseValue* value_to_compare, const std::string & op) const {
    auto entity_with_fact_ptr = std::make_shared<Engine::EntitiesWithFact>();

    for (auto ent : entities) {
        const auto & entity_attributes = _entity_attribute[ent];
        if (entity_attributes.find(key) != entity_attributes.end()) {
            for (const auto & entity_att : entity_attributes.at(key)) {
                if (entity_att.attribute_value -> valueCompare(value_to_compare, op)) {
                    entity_with_fact_ptr -> first.push_back(ent);
                    entity_with_fact_ptr -> second.push_back(entity_att.attribute_value);
                }
            }
        }
    }
    return entity_with_fact_ptr;
}

VerifyResult
Engine::_verify(const std::shared_ptr<std::vector<std::shared_ptr<BaseValue>>> &input_str_value, const BaseValue &verify_value,
                const std::string &verify_op) const {
    int match_num = 0;
    for (const auto& attr_value : *input_str_value) {
        if (attr_value -> valueCompare(&verify_value, verify_op)) {
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

Engine::Entities
Engine::findAll() const {
    return _all_entities;
}

Engine::Entities
Engine::find(const std::string & find_entity_name) const {
    if (_entity_name_to_number.find(find_entity_name) != _entity_name_to_number.end()) {
        return _entity_name_to_number.at(find_entity_name);
    }
    else {
        return {};
    }

}

Engine::Entities
Engine::filterConcept(
        const Engine::Entities & entities,
        const std::string & concept_name) const {

    auto concept_numbers = std::vector<int>();
    if (_concept_name_to_number.find(concept_name) != _concept_name_to_number.end()) {
        concept_numbers = _concept_name_to_number.at(concept_name);
    }

    std::set<int> entity_set;
    for (auto concept_num : concept_numbers) {
        auto concept_inst = _concept_has_instance_entities.at(concept_num);
        entity_set.insert(concept_inst.begin(), concept_inst.end());
    }

    Engine::Entities  output_entities;
    for (auto ent : entities) {
        if (entity_set.find(ent) != entity_set.end())   output_entities.push_back(ent);
    }

    return output_entities;
}

std::shared_ptr<Engine::EntitiesWithFact>
Engine::filterStr(
        const Engine::Entities & entities,
        const std::string & string_key,
        const std::string & string_value) const {
    auto value_to_compare = StringValue(string_value, BaseValue::string_type);
    auto return_pairs = _filter_attribute(entities, string_key, &value_to_compare, "=");
    return return_pairs;
}

std::shared_ptr<Engine::EntitiesWithFact>
Engine::filterNum(const Engine::Entities &entities, const std::string &number_key, const std::string &number_value,
                  const std::string &op) const {
    auto value_to_compare = QuantityValue(number_value);
    auto return_pairs = _filter_attribute(entities, number_key, &value_to_compare, op);
    return return_pairs;
}

std::shared_ptr<Engine::EntitiesWithFact>
Engine::filterYear(const Engine::Entities &entities, const std::string &year_key, const std::string &year_value,
                   const std::string &op) const {
    auto value_to_compare = YearValue(year_value);
    auto return_pairs = _filter_attribute(entities, year_key, &value_to_compare, op);
    return return_pairs;
}

std::shared_ptr<Engine::EntitiesWithFact>
Engine::filterDate(const Engine::Entities &entities, const std::string &date_key, const std::string &date_value,
                   const std::string &op) const {
    auto value_to_compare = DateValue(date_value);
    auto return_pairs = _filter_attribute(entities, date_key, &value_to_compare, op);
    return return_pairs;
}

VerifyResult
Engine::verifyStr(const std::shared_ptr<std::vector<std::shared_ptr<BaseValue>>> &input_str_value,
                               const std::string &verify_str_value) const {
    auto value_to_compare = StringValue(verify_str_value);
    return _verify(input_str_value, value_to_compare, "=");
}

VerifyResult
Engine::verifyNum(const std::shared_ptr<std::vector<std::shared_ptr<BaseValue>>> &input_num_value, const std::string &verify_num_value,
                  const std::string &verify_num_op) const {
    auto value_to_compare = QuantityValue(verify_num_value);
    return _verify(input_num_value, value_to_compare, verify_num_op);
}

VerifyResult
Engine::verifyYear(const std::shared_ptr<std::vector<std::shared_ptr<BaseValue>>> &input_year_value, const std::string &verify_year_value,
                                const std::string &verify_year_op) const {
    auto value_to_compare = YearValue(verify_year_value);
    return _verify(input_year_value, value_to_compare, verify_year_op);
}

VerifyResult
Engine::verifyDate(const std::shared_ptr<std::vector<std::shared_ptr<BaseValue>>> &input_date_value, const std::string &verify_date_value,
                                const std::string &verify_date_op) const {
    auto value_to_compare = DateValue(verify_date_value);
    return _verify(input_date_value, value_to_compare, verify_date_op);
}

std::shared_ptr<std::vector<const std::string* >>
Engine::queryName(const std::shared_ptr<std::vector<int>> & entity_list) const {
    auto return_ptr = std::make_shared<std::vector<const std::string* >>();
    for (auto entity_id : *entity_list) {
        return_ptr -> push_back(&(_entity_name[entity_id]));
    }
    return return_ptr;
}







