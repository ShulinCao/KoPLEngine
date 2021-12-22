#include "engine.h"

void Engine::_parseQualifier(Qualifiers & qualifier_output, const json & qualifier_json) {
    for (const auto& qualifier : qualifier_json.items()) {
        std::string qualifier_key_string(qualifier.key());

        for (const auto& single_qualifier : qualifier.value()) {
            BaseValue * val_ptr;
            BaseValue::parseValue(val_ptr, single_qualifier);
            qualifier_output[qualifier_key_string].push_back(val_ptr);
        }
    }
}

Engine::Engine(std::string & kb_file_name) {
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

    unsigned long total_concept_num = concept_json.size();
    unsigned long total_entity_num  = entity_json.size();
    std::cout << "number of concepts " << total_concept_num << std::endl;
    std::cout << "number of entities " << total_entity_num << std::endl;

    // Reserve Space for vectors
    _concept_name.reserve(total_concept_num);
    _concept_sub_class_of.reserve(total_concept_num);

    _entity_name.reserve(total_entity_num);
    _entity_is_instance_of.reserve(total_entity_num);
    _entity_attribute.reserve(total_entity_num);
    _entity_relation.reserve(total_entity_num);


    // Construct "_concept_id", "_concept_id_to_number", "_concept_name", "_concept_name_to_number"
    for (auto & concept : concept_json.items()) {
        std::string concept_id(concept.key());
        std::string concept_name(concept.value().at("name"));

        _concept_id.push_back(concept_id);
        _concept_id_to_number[concept_id] = _concept_id.size() - 1;

        _concept_name.push_back(concept_name);
        _concept_name_to_number[concept_name] = _concept_name.size() - 1;
    }

    // Construct "_concept_sub_class_of"
    for (auto & c: concept_json.items()) {
        std::string concept_id(c.key());
        int cur_number = _concept_id_to_number[concept_id];

        _concept_sub_class_of.push_back(std::set<int>());

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
        _entity_id_to_number[entity_id] = _entity_id.size() - 1;
        _entity_name.push_back(entity_name);
        _entity_name_to_number[entity_name] = _entity_name.size() - 1;

        // For "_entity_is_instance_of"
        int cur_entity_number = _entity_name.size() - 1;
        _entity_is_instance_of.push_back(std::set<int>());
        for (const auto& concept_id : entity.value().at("instanceOf")) {
            auto concept_id_in_string = concept_id.get<std::string>();
            int concept_number = _concept_id_to_number[concept_id_in_string];
            _entity_is_instance_of[cur_entity_number].insert(concept_number);
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


            RelationIndex relation_index(relation_name_string, relation_direction);
            EntityPairIndex entity_pair_index(_entity_relation.size(), tail_entity_number);

            _relation_to_entity_pair[relation_index].insert(entity_pair_index);
            _entity_pair_to_relation[entity_pair_index].insert(relation_index);
        }
        _entity_relation.push_back(relation);
    }
    std::cout << "End of initialize a new KB\n";
}
