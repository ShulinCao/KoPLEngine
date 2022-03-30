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
#include <algorithm>
#include <nlohmann/json.hpp>
#include "value.h"

using json = nlohmann::json;

typedef std::unordered_map<std::string, std::vector<std::shared_ptr<BaseValue>>> Qualifiers;

enum RelationDirection{
    forward,
    backward
};


enum VerifyResult{
    yes,
    no,
    not_sure
};


enum SelectOperator{
    smallest,
    largest
};


class Fact {
public:
    Qualifiers                                                  fact_qualifiers;
    
    ~Fact() {
        for (auto &kv_pair : fact_qualifiers) {
            for (auto val_ptr : kv_pair.second) {
                val_ptr.reset();
            }
        }
    }
};


class Attribute : public Fact {
public:
    std::shared_ptr<BaseValue>                                  attribute_value;
//    Qualifiers                                                  fact_qualifiers;
};


class Relation : public Fact {
public:
    std::string                                                 relation_name;
    RelationDirection                                           relation_direction;
    int                                                         relation_tail_entity;
//    Qualifiers                                                  fact_qualifiers;
};


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
    int                                                                         head_entity;
    int                                                                         tail_entity;

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
public:

    typedef std::vector<int>                                                    Entities;
    // we cannot use shared ptr, because the pointer will point to an existing memory block, which is not initiated by a smart pointer. Thus, the destruction of the fact will falsely destruct the memory block.
    typedef std::vector<std::shared_ptr<const Fact>>                            Facts;
    typedef std::pair<std::shared_ptr<Entities>, std::shared_ptr<Facts>>        EntitiesWithFacts;

    typedef std::vector<std::shared_ptr<BaseValue>>                             Values;

    typedef struct {
        std::vector<std::string> entity_ids;
        std::vector<std::string> entity_attributes;
        std::vector<std::string> entity_relations;
    } GraphContainer;

private:

    int                                                                         _worker_num;

    // Concept Name
    std::vector<std::string>                                                    _concept_name;
    std::map<std::string, std::vector<int>>                                     _concept_name_to_number;

    // Concept Wikidata ID
    std::vector<std::string>                                                    _concept_id;
    std::map<std::string, int>                                                  _concept_id_to_number;

    // Entity Name
    std::vector<std::string>                                                    _entity_name;
    std::map<std::string, std::shared_ptr<Entities>>                            _entity_name_to_number;

    // Entity Wikidata ID
    std::vector<std::string>                                                    _entity_id;
    std::map<std::string, int>                                                  _entity_id_to_number;

    // TODO: Attribute Name
    std::vector<std::string>                                                    _attribute_name;
    std::map<std::string, int>                                                  _attribute_name_to_number;
    std::unordered_map<std::string, unsigned short>                             _key_type;                  // Modify in _parseQualifier, and when parsing attribute values

    // TODO: Relation Name
    std::vector<std::string>                                                    _relation_name;
    std::map<std::string, int>                                                  _relation_name_to_number;

    // Sub Class Of, and Instance Of Relation
    std::vector<std::set<int>>                                                  _concept_sub_class_of;
    std::vector<std::set<int>>                                                  _entity_is_instance_of;


    std::vector<std::unordered_map<std::string, std::vector<std::shared_ptr<Attribute>>>> _entity_attribute;
    std::vector<std::vector<std::shared_ptr<Relation>>>                         _entity_relation;


    // Record entities that have some attribute (not the attribute value)
    std::unordered_map<std::string, std::set<int>>                              _attribute_key_to_entities;

    // Record entities that are pointed to by the relation
//    std::map<RelationIndex, std::vector<EntityPairIndex>>                       _relation_to_entity_pair;
    std::map<EntityPairIndex, std::vector<RelationIndex>>                       _entity_pair_to_relation;

    std::map<RelationIndex, std::unordered_map<int, std::vector<int>>>          _relation_in_entity_index;
    std::map<EntityPairIndex, std::vector<int>>                                 _entity_forward_relation_index;

    // Record all entities
    std::shared_ptr<EntitiesWithFacts>                                          _all_entities;

    // FindAll + Filter Index
    // Record entities of each instance
    std::vector<std::set<int>>                                                  _concept_has_instance_entities;
    std::map<std::string, std::shared_ptr<EntitiesWithFacts>>                                    _find_all_filter_str_index;
    typedef std::map<double, std::shared_ptr<EntitiesWithFacts>> NumIndex;
    std::map<std::string, std::shared_ptr<NumIndex>>             _find_all_filter_num_index;

    typedef std::map<YearValue, std::shared_ptr<EntitiesWithFacts>> YearIndex;
    typedef std::map<DateValue, std::shared_ptr<EntitiesWithFacts>> DateIndex;
    std::map<std::string, std::shared_ptr<YearIndex>>       _find_all_filter_year_index;
    std::map<std::string, std::shared_ptr<DateIndex>>       _find_all_filter_date_index;

    // reserve the serialized string of origin json object
    std::vector<std::string>              serialStringsOfAttributes;
    std::vector<std::map<int, json>>      jsonsOfRelations;

    // Will modify key type
    void _parseQualifier(Qualifiers & qualifier_output, const json & qualifier_json);
    void _addKeyType(const std::string & key, unsigned short type);
    void _addFindAllFilterIndex(const std::string & attribute_key, const std::shared_ptr<Attribute> & attribute, int cur_entity_number);

    std::shared_ptr<EntitiesWithFacts>
    _filter_qualifier(
            const std::shared_ptr<EntitiesWithFacts> & entity_with_fact,

            const std::string & filter_key,
            const std::shared_ptr<BaseValue> & value_to_compare,
            const std::string & op
    ) const;

    std::shared_ptr<EntitiesWithFacts>
    _filter_attribute(
            const std::shared_ptr<EntitiesWithFacts> & entity_ids,

            const std::string & key,
            const std::shared_ptr<BaseValue> & value_to_compare,
            const std::string & op
    ) const;

    VerifyResult
    _verify(
            const std::shared_ptr<Values> &input_str_value,
            const std::shared_ptr<BaseValue> & verify_value,
            const std::string & verify_op
            ) const;
public:
    explicit Engine(std::string & kb_file_name, int worker_num = 4);

    ~Engine();

    void examineEntityAttribute()   const;
    void examineRelation()          const;
    void examineAttributeKeyIndex() const;
    void examineRelationIndex()     const;
    void examineEntityPairIndex()   const;


    // Execution Logic Here

    void programExec(std::string program) const;
    void programBatchExec(std::vector<std::string> programs) const;

    // Operators

    // Use move semantic when assigning to an intermediate variable
    // TODO: may return const reference of Entities
    std::shared_ptr<EntitiesWithFacts>
    findAll() const;

    // TODO: may return const reference of Entities
    std::shared_ptr<EntitiesWithFacts>
    find(
            const std::string & find_entity_name                        // arg1
            ) const;


    std::shared_ptr<EntitiesWithFacts>
    filterConcept(
            const std::shared_ptr<EntitiesWithFacts> & entity_ids,

            const std::string & concept_name                            // arg1
            ) const;

    std::shared_ptr<EntitiesWithFacts>
    findAllFilterConcept(
            const std::string & concept_name                            // arg1
    ) const;

    std::shared_ptr<EntitiesWithFacts>
    filterStr(
            const std::shared_ptr<EntitiesWithFacts> & entity_ids,

            const std::string & string_key,                             // arg1
            const std::string & string_value                            // arg2
            ) const;

    std::shared_ptr<EntitiesWithFacts>
    findAllFilterStr(
            const std::string & string_key,                             // arg1
            const std::string & string_value                            // arg2
    ) const;

    std::shared_ptr<EntitiesWithFacts>
    filterNum(
            const std::shared_ptr<EntitiesWithFacts> & entity_ids,

            const std::string & number_key,                             // arg1
            const std::string & number_value,                           // arg2
            const std::string & op                                      // arg3
            ) const;

    std::shared_ptr<EntitiesWithFacts>
    findAllFilterNum(
            const std::string & number_key,                             // arg1
            const std::string & number_value,                           // arg2
            const std::string & op                                      // arg3
            ) const;

    std::shared_ptr<EntitiesWithFacts>
    filterYear(
            const std::shared_ptr<EntitiesWithFacts> & entity_ids,

            const std::string & year_key,                               // arg1
            const std::string & year_value,                             // arg2
            const std::string & op                                      // arg3
            ) const;

    std::shared_ptr<EntitiesWithFacts>
    findAllFilterYear(
            const std::string & year_key,                               // arg1
            const std::string & year_value,                             // arg2
            const std::string & op                                      // arg3
            ) const;

    std::shared_ptr<EntitiesWithFacts>
    filterDate(
            const std::shared_ptr<EntitiesWithFacts> & entity_ids,

            const std::string & date_key,                               // arg1
            const std::string & date_value,                             // arg2
            const std::string & op                                      // arg3
            ) const;

    std::shared_ptr<EntitiesWithFacts>
    findAllFilterDate(
            const std::string & date_key,                               // arg1
            const std::string & date_value,                             // arg2
            const std::string & op                                      // arg3
            ) const;

    std::shared_ptr<EntitiesWithFacts>
    QfilterStr(
            const std::shared_ptr<EntitiesWithFacts> & entity_with_fact,

            const std::string & qualifier_string_key,                   // arg1
            const std::string & qualifier_string_value                  // arg2
            ) const;

    std::shared_ptr<EntitiesWithFacts>
    QfilterNum(
            const std::shared_ptr<EntitiesWithFacts> & entity_with_fact,

            const std::string & qualifier_num_key,                      // arg1
            const std::string & qualifier_num_value,                    // arg2
            const std::string & op                                      // arg3
            ) const;

    std::shared_ptr<EntitiesWithFacts>
    QfilterYear(
            const std::shared_ptr<EntitiesWithFacts> & entity_with_fact,

            const std::string & qualifier_year_key,                     // arg1
            const std::string & qualifier_year_value,                   // arg2
            const std::string & op                                      // arg3
            ) const;

    std::shared_ptr<EntitiesWithFacts>
    QfilterDate(
            const std::shared_ptr<EntitiesWithFacts> & entity_with_fact,

            const std::string & qualifier_date_key,                     // arg1
            const std::string & qualifier_date_value,                   // arg2
            const std::string & op                                      // arg3
            ) const;

    // TODO: may define these operators as static
    std::shared_ptr<EntitiesWithFacts>
    relateOp(
            const std::shared_ptr<EntitiesWithFacts> & entities,

            const std::string & relation_name,                          // arg1
            const std::string & relation_direction                      // arg2
            );

    static std::shared_ptr<EntitiesWithFacts>
    andOp(
            const std::shared_ptr<EntitiesWithFacts> & a,
            const std::shared_ptr<EntitiesWithFacts> & b
            );

    static std::shared_ptr<EntitiesWithFacts>
    orOp(
            const std::shared_ptr<EntitiesWithFacts> & a,
            const std::shared_ptr<EntitiesWithFacts> & b
            );

    static int
    countOp(const std::shared_ptr<EntitiesWithFacts> & entities);


    std::shared_ptr<std::vector<const std::string*>>
    queryName(
            const std::shared_ptr<EntitiesWithFacts> & entity_ids
            ) const;

    std::shared_ptr<Values>
    queryAttr(
            const std::shared_ptr<EntitiesWithFacts> & entity_ids,

            const std::string & query_attribute_key                     // arg1
            ) const;

    std::shared_ptr<Values>
    queryAttrUnderCondition(
            const std::shared_ptr<EntitiesWithFacts> & entity_ids,

            const std::string & query_attribute_key,                    // arg1
            const std::string & qualifier_key,                          // arg2
            const std::string & qualifier_value                         // arg3
            ) const;

    std::shared_ptr<std::vector<const std::string*>>
    queryRelation(
            const std::shared_ptr<EntitiesWithFacts> & entity_ids_a,
            const std::shared_ptr<EntitiesWithFacts> & entity_ids_b
            ) const;

    std::shared_ptr<Values>
    queryAttrQualifier(
            const std::shared_ptr<EntitiesWithFacts> & entity_ids,

            const std::string & attribute_key,                          // arg1
            const std::string & attribute_value,                        // arg2
            const std::string & qualifier_key                           // arg3
    ) const;

    std::shared_ptr<Values>
    queryRelationQualifier(
            const std::shared_ptr<EntitiesWithFacts> & entity_ids_a,
            const std::shared_ptr<EntitiesWithFacts> & entity_ids_b,

            const std::string & relation_name,                          // arg1
            const std::string & qualifier_key                           // arg2
    ) const;


    std::shared_ptr<std::vector<const std::string *>>
    selectAmong(
            const std::shared_ptr<EntitiesWithFacts> & entity_ids,

            const std::string & attribute_key,                          // arg1
            const std::string & select_op                               // arg2
            ) const;

    std::shared_ptr<std::vector<const std::string *>>
    selectBetween(
            const std::shared_ptr<EntitiesWithFacts> & entity_ids_a,
            const std::shared_ptr<EntitiesWithFacts> & entity_ids_b,

            const std::string & attribute_key,                          // arg1
            const std::string & select_op                               // arg2
            ) const;


    VerifyResult
    verifyStr(
            const std::shared_ptr<Values> & input_str_value,

            const std::string & verify_str_value                        // arg1
            ) const;

    VerifyResult
    verifyNum(
            const std::shared_ptr<Values> & input_num_value,

            const std::string & verify_num_value,                       // arg1
            const std::string & verify_num_op                           // arg2
            ) const;

    VerifyResult
    verifyYear(
            const std::shared_ptr<Values> & input_year_value,

            const std::string & verify_year_value,                      // arg1
            const std::string & verify_year_op                          // arg2
            ) const;

    VerifyResult
    verifyDate(
            const std::shared_ptr<Values> & input_date_value,

            const std::string & verify_date_value,                      // arg1
            const std::string & verify_date_op                          // arg2
            ) const;

    void programExec(std::vector<std::string> programs) const;

    std::shared_ptr<GraphContainer>
    expandFromEntities(
            const std::vector<std::string> * entity_ids,
            const int jump_limitation
            ) const;

    void dfsTraversal(
            int entity,
            int from_entity,
            std::map<int, int> & from_index,
            int depth_limitation,
            int depth,
            std::vector<std::vector<json>> & subsetOfRelations,
            std::shared_ptr<std::set<int>> & visited_index,
            std::shared_ptr<GraphContainer> & subgraph_ptr
            ) const;


};


#endif //KOPL_ENGINE_H
