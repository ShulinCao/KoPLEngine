#include "engine.h"

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


    unsigned long concept_number = concept_json.size();
    unsigned long entity_number  = entity_json.size();
    std::cout << "number of concepts " << concept_number << std::endl;
    std::cout << "number of entities " << entity_number << std::endl;

    // Reserve Space for vectors
//    _concept_name.reserve(concept_number);
//    _concept_sub_class_of.reserve(concept_number);
//
//    _entity_name.reserve(entity_number);
//    _entity_is_instance_of.reserve(entity_number);
//    _entity_attribute.reserve(entity_number);
//    _entity_relation.reserve(entity_number);


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
        for (auto sup_class : concept_super_class) {
            int sup_class_number = _concept_id_to_number[sup_class];
            _concept_sub_class_of[cur_number].insert(sup_class_number);
        }
    }

    // Construct "entity_id", "entity_id_to_number", "entity_name", "entity_name_to_number"
    // Construct "_entity_is_instance_of"
    // Construct "_entity_attribute"
    for (auto & entity : entity_json.items()) {
        std::string entity_id(entity.key());
        std::string entity_name(entity.value().at("name"));

        _entity_id.push_back(entity_id);
        _entity_id_to_number[entity_id] = _entity_id.size() - 1;

        _entity_name.push_back(entity_name);
        _entity_name_to_number[entity_name] = _entity_name.size() - 1;

        // For "_entity_is_instance_of"
        int cur_entity_number = _entity_name.size() - 1;
        _entity_is_instance_of.push_back(std::set<int>());
        for (auto concept_id : entity.value().at("instanceOf")) {
            std::string concept_id_in_string(concept_id);
            int concept_number = _concept_id_to_number[concept_id_in_string];
            _entity_is_instance_of[cur_entity_number].insert(concept_number);
        }

        // For "_entity_attribute"
        for (auto attribute_json : entity.value().at("attributes")) {
            Attribute attribute;

            // obtain value key
            auto attribute_key = attribute_json.at("key");

            // obtain value, value type, value unit
            std::string attribute_type(attribute_json.at("value").at("type"));
            auto attribute_value = attribute_json.at("value").at("value");
            auto attribute_value_type = BaseValue::convertStringTypeToShort(attribute_type, attribute_value);
            std::string unit;
            if (attribute_value_type == BaseValue::int_type || attribute_value_type == BaseValue::float_type) {
                unit = attribute_json.at("value").at("unit");
            }

            // Create value object via value, value type, value unit, and value_key
            if (attribute_value_type == BaseValue::string_type) {
                std::string val = attribute_value.get<std::string>();
                attribute.attribute_value = new StringValue(val, attribute_value_type);
            }
            else if (attribute_value_type == BaseValue::int_type) {
                int val = attribute_value.get<int>();
                attribute.attribute_value = new QuantityValue(val, unit, attribute_value_type);
            }
            else if (attribute_value_type == BaseValue::float_type) {
                double val = attribute_value.get<float>();
                attribute.attribute_value = new QuantityValue(val, unit, attribute_value_type);
            }
            else if (attribute_value_type == BaseValue::date_type) {
                std::string val = attribute_value.get<std::string>();
                attribute.attribute_value = new DateValue(val, attribute_value_type);
            }
            else if (attribute_value_type == BaseValue::year_type) {
                short val = attribute_value.get<short>();
                attribute.attribute_value = new YearValue(val, attribute_value_type);
            }

            auto attribute_qualifier_json = attribute_json.at("qualifiers");
        }
    }

    std::cout << "End of initialize a new KB\n";
}
