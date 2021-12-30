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
    int                                                         _worker_num;

    std::vector<std::string>                                    _concept_name;
    std::vector<std::string>                                    _entity_name;
    std::map<std::string, std::vector<int>>                     _concept_name_to_number;
    std::map<std::string, std::vector<int>>                     _entity_name_to_number;

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
    std::map<RelationIndex, std::set<EntityPairIndex>>          _relation_to_entity_pair;
    std::map<EntityPairIndex, std::set<RelationIndex>>          _entity_pair_to_relation;

    // Record all entities
    typedef std::vector<int>                                    Entities;
    typedef std::pair<Entities, std::vector<std::shared_ptr<BaseValue>>>        EntitiesWithFact;
    Entities                                                    _all_entities;

    // Record entities of each instance
    std::vector<std::set<int>>                                  _concept_has_instance_entities;

    static void _parseQualifier(Qualifiers & qualifier_output, const json & qualifier_json);

    std::shared_ptr<EntitiesWithFact>
    _filter_qualifier(
            const std::shared_ptr<std::vector<EntitiesWithFact>> & entity_with_fact,

            const std::string & tgt_key,
            const std::string & tgt_value,
            const std::string & op,
            const std::shared_ptr<BaseValue> & value_to_compare
    ) const;

    std::shared_ptr<EntitiesWithFact>
    _filter_attribute(
            const Engine::Entities & entities,

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
    Entities
    findAll() const;

    // TODO: may return const reference of Entities
    Entities
    find(
            const std::string & find_entity_name
            ) const;


    Entities
    filterConcept(
            const Entities & entities,

            const std::string & concept_name
            ) const;

    std::shared_ptr<EntitiesWithFact>
    filterStr(
            const Entities & entities,

            const std::string & string_key,
            const std::string & string_value
            ) const;

    std::shared_ptr<EntitiesWithFact>
    filterNum(
            const Entities & entities,

            const std::string & number_key,
            const std::string & number_value,
            const std::string & op
            ) const;

    std::shared_ptr<EntitiesWithFact>
    filterYear(
            const Entities & entities,

            const std::string & year_key,
            const std::string & year_value,
            const std::string & op
            ) const;

    std::shared_ptr<EntitiesWithFact>
    filterDate(
            const Entities & entities,

            const std::string & date_key,
            const std::string & date_value,
            const std::string & op
            ) const;


    std::shared_ptr<EntitiesWithFact>
    QfilterStr(
            const std::shared_ptr<std::vector<EntitiesWithFact>> & entity_with_fact,

            const std::string & qualifier_string_key,
            const std::string & qualifier_string_value
            ) const;

    std::vector<EntitiesWithFact>
    QfilterNum(
            const std::vector<EntitiesWithFact> & entity_with_fact,

            const std::string & qualifier_num_key,
            const std::string & qualifier_string_value
            ) const;

    std::vector<EntitiesWithFact>
    QfilterYear(
            const std::vector<EntitiesWithFact> & entity_with_fact,

            const std::string & qualifier_year_key,
            const std::string & qualifier_year_value
            ) const;

    std::vector<EntitiesWithFact>
    QfilterDate(
            const std::vector<EntitiesWithFact> & entity_with_fact,

            const std::string & qualifier_date_key,
            const std::string & qualifier_date_value
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
            const std::shared_ptr<std::vector<int>>& entity_list
            ) const;

    std::shared_ptr<std::vector<std::shared_ptr<BaseValue>>>
    queryAttr(
            const std::shared_ptr<std::vector<int>>& entity_list,

            const std::string & query_attribute_key
            ) const;

    std::shared_ptr<std::vector<std::shared_ptr<BaseValue>>>
    queryAttrUnderCondition(
            const std::shared_ptr<std::vector<int>>& entity_list,

            const std::string & query_attribute_key,
            const std::string & qualifier_key,
            const std::shared_ptr<BaseValue>& qualifier_value
            ) const;

    std::shared_ptr<std::vector<const std::string*>>
    queryRelation(
            const std::shared_ptr<std::vector<int>>& entity_list_a,
            const std::shared_ptr<std::vector<int>>& entity_list_b
            ) const;

    std::shared_ptr<std::vector<std::shared_ptr<BaseValue>>>
    queryAttrQualifier(
            const std::shared_ptr<std::vector<int>>& entity_list,

            const std::string & attribute_key,
            const std::shared_ptr<BaseValue>& attribute_value,
            const std::string & qualifier_key
    ) const;

    std::shared_ptr<std::vector<std::shared_ptr<BaseValue>>>
    queryRelationQualifier(
            const std::shared_ptr<std::vector<int>>& entity_list_a,
            const std::shared_ptr<std::vector<int>>& entity_list_b,

            const std::string & relation_name,
            const std::string & qualifier_key
    ) const;


    std::shared_ptr<std::vector<const std::string *>>
    selectAmong(
            const std::shared_ptr<std::vector<int>> & entities,

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
