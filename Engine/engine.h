#ifndef KOPL_ENGINE_H
#define KOPL_ENGINE_H

#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <nlohmann/json.hpp>
#include "value.h"

using json = nlohmann::json;

typedef     std::map<std::string, std::vector<BaseValue*>>      Qualifiers;

enum RelationDirection{
    forward,
    backward
};


typedef struct _Attribute{
    BaseValue *                                                 attribute_value;
    Qualifiers                                                  attribute_qualifiers;
} Attribute;

typedef struct _Relation{
    std::string                                                 relation_name;
    RelationDirection                                           relation_direction;
    int                                                         relation_tail_entity;
    Qualifiers                                                  relation_qualifier;
} Relation;



class Engine {
private:
    std::vector<std::string>                                    _concept_name;
    std::vector<std::string>                                    _entity_name;
    std::map<std::string, int>                                  _concept_name_to_number;
    std::map<std::string, int>                                  _entity_name_to_number;

    std::vector<std::string>                                    _concept_id;
    std::vector<std::string>                                    _entity_id;
    std::map<std::string, int>                                  _concept_id_to_number;
    std::map<std::string, int>                                  _entity_id_to_number;


    std::vector<std::set<int>>                                  _concept_sub_class_of;
    std::vector<std::set<int>>                                  _entity_is_instance_of;


    std::vector<std::map<std::string, std::vector<Attribute>>>  _entity_attribute;
    std::vector<std::vector<Relation>>                          _entity_relation;


    // Record entities that have some attribute (not the attribute value)
    std::map<std::string, std::set<int>>                        _attribute_key_to_entities;

    // Record entities that are pointed to by the relation
    std::map<std::string, std::set<int>>                        _forward_relation_to_entities;
    std::map<std::string, std::set<int>>                        _backward_relation_to_entities;

    static void _parseQualifier(Qualifiers & qualifier_output, const json & qualifier_json);
public:
    explicit Engine(std::string & kb_file_name);
};


#endif //KOPL_ENGINE_H
