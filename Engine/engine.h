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

typedef std::map<std::string, std::vector<std::shared_ptr<BaseValue>>> Qualifiers;

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



typedef struct _Attribute{
    std::shared_ptr<BaseValue>                                  attribute_value;
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

    typedef std::vector<int>                                    Entities;
    typedef std::vector<const Attribute*>                       Facts;      // we cannot use shared ptr, because the pointer will point to an existing memory block, which is not initiated by a smart pointer. Thus, the destruction of the fact will falsely destruct the memory block.
    typedef std::pair<Entities, Facts>                          EntitiesWithFacts;

    int                                                         _worker_num;

    // Concept Name
    std::vector<std::string>                                    _concept_name;
    std::map<std::string, std::vector<int>>                     _concept_name_to_number;

    // Concept Wikidata ID
    std::vector<std::string>                                    _concept_id;
    std::map<std::string, int>                                  _concept_id_to_number;

    // Entity Name
    std::vector<std::string>                                    _entity_name;
    std::map<std::string, std::shared_ptr<Entities>>            _entity_name_to_number;

    // Entity Wikidata ID
    std::vector<std::string>                                    _entity_id;
    std::map<std::string, int>                                  _entity_id_to_number;

    // TODO: Attribute Name
    std::vector<std::string>                                    _attribute_name;
    std::map<std::string, int>                                  _attribute_name_to_number;

    // TODO: Relation Name
    std::vector<std::string>                                    _relation_name;
    std::map<std::string, int>                                  _relation_name_to_number;

    // Sub Class Of, and Instance Of Relation
    std::vector<std::set<int>>                                  _concept_sub_class_of;
    std::vector<std::set<int>>                                  _entity_is_instance_of;


    std::vector<std::map<std::string, std::vector<Attribute>>>  _entity_attribute;
    std::vector<std::vector<Relation>>                          _entity_relation;
    std::map<EntityPairIndex, std::vector<int>>                 _entity_forward_relation_index;


    // Record entities that have some attribute (not the attribute value)
    std::map<std::string, std::set<int>>                        _attribute_key_to_entities;

    // Record entities that are pointed to by the relation
    std::map<RelationIndex, std::vector<EntityPairIndex>>        _relation_to_entity_pair;
    std::map<EntityPairIndex, std::vector<RelationIndex>>        _entity_pair_to_relation;

    // Record all entities
    std::shared_ptr<Entities>                                   _all_entities;

    // Record entities of each instance
    std::vector<std::set<int>>                                  _concept_has_instance_entities;

    static void _parseQualifier(Qualifiers & qualifier_output, const json & qualifier_json);

    std::shared_ptr<EntitiesWithFacts>
    _filter_qualifier(
            const std::shared_ptr<EntitiesWithFacts> & entity_with_fact,

            const std::string & filter_key,
            const std::shared_ptr<BaseValue> & value_to_compare,
            const std::string & op
    ) const;

    std::shared_ptr<EntitiesWithFacts>
    _filter_attribute(
            const std::shared_ptr<Entities> & entity_ids,

            const std::string & key,
            const std::shared_ptr<BaseValue> & value_to_compare,
            const std::string & op
    ) const;

    VerifyResult
    _verify(
            const std::shared_ptr<std::vector<std::shared_ptr<BaseValue>>> &input_str_value,
            const std::shared_ptr<BaseValue> & verify_value,
            const std::string & verify_op
            ) const;
public:
    explicit Engine(std::string & kb_file_name, int worker_num = 4);

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
    std::shared_ptr<Entities>
    findAll() const;

    // TODO: may return const reference of Entities
    std::shared_ptr<Entities>
    find(
            const std::string & find_entity_name
            ) const;


    std::shared_ptr<Entities>
    filterConcept(
            const std::shared_ptr<Entities> &entity_ids,

            const std::string & concept_name
            ) const;

    std::shared_ptr<EntitiesWithFacts>
    filterStr(
            const std::shared_ptr<Entities> &entity_ids,

            const std::string & string_key,
            const std::string & string_value
            ) const;

    std::shared_ptr<EntitiesWithFacts>
    filterNum(
            const std::shared_ptr<Entities> &entity_ids,

            const std::string & number_key,
            const std::string & number_value,
            const std::string & op
            ) const;

    std::shared_ptr<EntitiesWithFacts>
    filterYear(
            const std::shared_ptr<Entities> &entity_ids,

            const std::string & year_key,
            const std::string & year_value,
            const std::string & op
            ) const;

    std::shared_ptr<EntitiesWithFacts>
    filterDate(
            const std::shared_ptr<Entities> &entity_ids,

            const std::string & date_key,
            const std::string & date_value,
            const std::string & op
            ) const;


    std::shared_ptr<EntitiesWithFacts>
    QfilterStr(
            const std::shared_ptr<EntitiesWithFacts> & entity_with_fact,

            const std::string & qualifier_string_key,
            const std::string & qualifier_string_value
            ) const;

    std::shared_ptr<EntitiesWithFacts>
    QfilterNum(
            const std::shared_ptr<EntitiesWithFacts> & entity_with_fact,

            const std::string & qualifier_num_key,
            const std::string & qualifier_num_value,
            const std::string & op
            ) const;

    std::shared_ptr<EntitiesWithFacts>
    QfilterYear(
            const std::shared_ptr<EntitiesWithFacts> & entity_with_fact,

            const std::string & qualifier_year_key,
            const std::string & qualifier_year_value,
            const std::string & op
            ) const;

    std::shared_ptr<EntitiesWithFacts>
    QfilterDate(
            const std::shared_ptr<EntitiesWithFacts> & entity_with_fact,

            const std::string & qualifier_date_key,
            const std::string & qualifier_date_value,
            const std::string & op
            ) const;

    // TODO: may define these operators as static
    static Entities
    relateOp(
            const Entities & entities,

            const std::string & relation_name,
            const std::string & relataion_direction
            );

    static Entities
    andOp(
            const Entities & a,
            const Entities & b
            );

    static Entities
    orOp(
            const Entities & a,
            const Entities & b
            );

    static int
    countOp(const Entities & entities);


    std::shared_ptr<std::vector<const std::string*>>
    queryName(
            const std::shared_ptr<Entities> & entity_ids
            ) const;

    std::shared_ptr<std::vector<std::shared_ptr<BaseValue>>>
    queryAttr(
            const std::shared_ptr<Entities> & entity_ids,

            const std::string & query_attribute_key
            ) const;

    std::shared_ptr<std::vector<std::shared_ptr<BaseValue>>>
    queryAttrUnderCondition(
            const std::shared_ptr<Entities> & entity_ids,

            const std::string & query_attribute_key,
            const std::string & qualifier_key,
            const std::shared_ptr<BaseValue>& qualifier_value
            ) const;

    std::shared_ptr<std::vector<const std::string*>>
    queryRelation(
            const std::shared_ptr<Entities> & entity_ids_a,
            const std::shared_ptr<Entities> & entity_ids_b
            ) const;

    std::shared_ptr<std::vector<std::shared_ptr<BaseValue>>>
    queryAttrQualifier(
            const std::shared_ptr<Entities> & entity_ids,

            const std::string & attribute_key,
            const std::shared_ptr<BaseValue> & attribute_value,
            const std::string & qualifier_key
    ) const;

    std::shared_ptr<std::vector<std::shared_ptr<BaseValue>>>
    queryRelationQualifier(
            const std::shared_ptr<Entities> & entity_ids_a,
            const std::shared_ptr<Entities> & entity_ids_b,

            const std::string & relation_name,
            const std::string & qualifier_key
    ) const;


    std::shared_ptr<std::vector<const std::string *>>
    selectAmong(
            const std::shared_ptr<Entities> & entity_ids,

            const std::string & attribute_key,
            const SelectOperator & select_operator
            ) const;

//    int selectBetween(
//            const Entities & entities,
//
//            const std::string & attribute_key,
//            const SelectOperator & select_operator
//            ) const;


    VerifyResult
    verifyStr(
            const std::shared_ptr<std::vector<std::shared_ptr<BaseValue>>> & input_str_value,

            const std::string & verify_str_value
            ) const;

    VerifyResult
    verifyNum(
            const std::shared_ptr<std::vector<std::shared_ptr<BaseValue>>> & input_num_value,

            const std::string & verify_num_value,
            const std::string & verify_num_op
            ) const;

    VerifyResult
    verifyYear(
            const std::shared_ptr<std::vector<std::shared_ptr<BaseValue>>> & input_year_value,

            const std::string & verify_year_value,
            const std::string & verify_year_op
            ) const;

    VerifyResult
    verifyDate(
            const std::shared_ptr<std::vector<std::shared_ptr<BaseValue>>> & input_date_value,

            const std::string & verify_date_value,
            const std::string & verify_date_op
            ) const;

    void programExec(std::vector<std::string> programs) const;

};


#endif //KOPL_ENGINE_H
