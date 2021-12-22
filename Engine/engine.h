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


class RelationIndex {
public:
    std::string                                                 relation_name;
    RelationDirection                                           relation_direction;
    friend bool operator< (const RelationIndex & a, const RelationIndex & b) {
        if (a.relation_direction < b.relation_direction) {
            return true;
        }
        else if (a.relation_direction == b.relation_direction) {
            return a.relation_name < b.relation_name;
        }
        return false;
    }
    explicit RelationIndex(const std::string & rel_name, const RelationDirection & rel_dir):
        relation_name(rel_name), relation_direction(rel_dir) {};
    RelationIndex(const RelationIndex & rel):
        relation_name(rel.relation_name), relation_direction(rel.relation_direction) {
    };
};

class EntityPairIndex {
public:
    int                                                         head_entity;
    int                                                         tail_entity;
    friend bool operator< (const EntityPairIndex & a, const EntityPairIndex & b) {
        if (a.head_entity < b.head_entity) {
            return true;
        }
        else if (a.head_entity == b.head_entity) {
            return a.tail_entity < b.tail_entity;
        }
        return false;
    }
    explicit EntityPairIndex(int head, int tail):
        head_entity(head), tail_entity(tail) {};
    EntityPairIndex(const EntityPairIndex & ent_pair):
        head_entity(ent_pair.head_entity), tail_entity(ent_pair.tail_entity) {
    };
};

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
    std::map<RelationIndex, std::set<EntityPairIndex>>       _relation_to_entity_pair;
    std::map<EntityPairIndex, std::set<RelationIndex>>       _entity_pair_to_relation;

    static void _parseQualifier(Qualifiers & qualifier_output, const json & qualifier_json);
public:
    explicit Engine(std::string & kb_file_name);
};


#endif //KOPL_ENGINE_H
