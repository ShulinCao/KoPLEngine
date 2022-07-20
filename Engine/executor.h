#ifndef KOPL_EXECUTOR_H
#define KOPL_EXECUTOR_H

#include "engine.h"

using json = nlohmann::json;

const int MAX_INNER_CONTENT_NUM = 3;

class Function {
public:
    std::string                                                             function_name;
    std::vector<std::string>                                                function_args;
    int                                                                     dependencies[2] = {-1, -1};

    Function(
            const std::string &               fun_name,
            const std::vector<std::string> &  fun_args,
            int                               dep_a = -1,
            int                               dep_b = -2
            ) : function_name(fun_name), function_args(fun_args) {
        dependencies[0] = dep_a;
        dependencies[1] = dep_b;
    }
};


typedef  std::vector<std::shared_ptr<Engine::EntitiesWithFacts>>            EntityWithFactBuffer;
typedef  std::vector<std::shared_ptr<Engine::Values>>                       ValuesBuffer;


class Executor {
public:
    std::shared_ptr<Engine>                                                 executor_engine;
    explicit Executor(std::shared_ptr<Engine> engine) : executor_engine(engine) {};
    explicit Executor(std::string kb_file_name, int worker_num) {
        executor_engine = std::make_shared<Engine>(kb_file_name, worker_num);
    }

private:
    static std::string _obtain_result(int integer) {
        char number_str[50];
        sprintf(number_str, "%d", integer);
        return {number_str};
    }
    static std::string _obtain_result(const BaseValue & base_value) {
        return base_value.toPrintStr();
    }
    static std::string _obtain_result(std::shared_ptr<const BaseValue> base_value) {
        return base_value -> toPrintStr();
    }
    static std::string _obtain_result(VerifyResult verify_result) {
        if (verify_result == VerifyResult::yes) {
            return {"yes"};
        }
        else if (verify_result == VerifyResult::no) {
            return {"no"};
        }
        else {
            return {"not sure"};
        }
    }

    // TODO: complete this function
    static std::string _obtain_result(const std::shared_ptr<Engine::GraphContainer> & graph_container) {
        std::string s = "";
        std::cout << graph_container -> entity_ids.size() << std::endl;
        for (auto i = 0; i < graph_container -> entity_ids.size(); ++i){
            s = s + graph_container -> entity_ids[i] + "\t\t" + graph_container -> entity_relations[i] + "\t\t" + graph_container -> entity_attributes[i];
//            s = s + graph_container -> entity_ids[i] + "\t\t";
            s = s + "\n";
        }
        return s;
    }

    static std::string _inner_contents_relation(const std::shared_ptr<Engine::EntitiesWithFacts> & entities_with_facts, std::shared_ptr<Engine> executor_engine){
        int max_num = entities_with_facts->first->size() > MAX_INNER_CONTENT_NUM ? MAX_INNER_CONTENT_NUM : entities_with_facts->first->size();
        json entities_with_facts_array = json::array();
        for (int i = 0; i < max_num; ++i){
            json content;
            auto entities = *(entities_with_facts->first);
            auto facts = entities_with_facts->second;

            auto entity_id = entities[i];
            content["entity_label"] = executor_engine -> get_entity_name(entity_id);

            if (facts == nullptr || facts->empty()){
                entities_with_facts_array.push_back(content);
                continue;
            }
            auto fact = std::static_pointer_cast<const Relation>((*facts)[i]);
            content["relation_label"] = fact -> relation_name;
            content["relation_direction"] = fact -> relation_direction == RelationDirection::forward ? "forward" : "backward";
            content["tail_entity"] = executor_engine -> get_entity_name(fact -> relation_tail_entity);
            content["qualifiers"] = json::array();

            auto fact_qualifiers = fact -> fact_qualifiers;
            auto num1 = 0;
            for (auto &kv_pair : fact_qualifiers) {
                num1 += 1;
                if (num1 > MAX_INNER_CONTENT_NUM){
                    break;
                }
                json qualifier;
                qualifier["key"] = kv_pair.first;
                json vals = json::array();

                auto num2 = 0;
                for (auto val_ptr : kv_pair.second) {
                    num2 += 1;
                    if (num2 > MAX_INNER_CONTENT_NUM){
                        break;
                    }
                    vals.push_back(val_ptr->toPrintStr());
                }
                qualifier["vals"] = vals;
                content["qualifiers"].push_back(qualifier);
            }
            entities_with_facts_array.push_back(content);
        }
        return entities_with_facts_array.dump();
    }

    static std::string _inner_contents(const std::shared_ptr<Engine::EntitiesWithFacts> & entities_with_facts, std::shared_ptr<Engine> executor_engine) {
        if (entities_with_facts == nullptr){
            return "";
        }
        if (entities_with_facts -> second == nullptr || entities_with_facts -> second -> size() == 0 || entities_with_facts -> second -> at(0) -> fact_class == FactClass::relation){
            return Executor::_inner_contents_relation(entities_with_facts, executor_engine);
        }
        int max_num = entities_with_facts->first->size() > MAX_INNER_CONTENT_NUM ? MAX_INNER_CONTENT_NUM : entities_with_facts->first->size();
        json entities_with_facts_array = json::array();
        for (int i = 0; i < max_num; ++i){
            json content;
            auto entities = *(entities_with_facts->first);
            auto facts = entities_with_facts->second;

            auto entity_id = entities[i];
            auto temp = executor_engine -> get_entity_name(entity_id);
            content["entity_label"] = executor_engine -> get_entity_name(entity_id);

            if (facts == nullptr || facts->empty()){
                entities_with_facts_array.push_back(content);
                continue;
            }
            auto fact = std::static_pointer_cast<const Attribute>((*facts)[i]);
            content["attribute_value"] = fact -> attribute_value -> toPrintStr();
            content["qualifiers"] = json::array();

            auto fact_qualifiers = fact -> fact_qualifiers;
            auto num1 = 0;
            for (auto &kv_pair : fact_qualifiers) {
                num1 += 1;
                if (num1 > MAX_INNER_CONTENT_NUM){
                    break;
                }
                json qualifier;
                qualifier["key"] = kv_pair.first;
                json vals = json::array();

                auto num2 = 0;
                for (auto val_ptr : kv_pair.second) {
                    num2 += 1;
                    if (num2 > MAX_INNER_CONTENT_NUM){
                        break;
                    }
                    vals.push_back(val_ptr->toPrintStr());
                }
                qualifier["vals"] = vals;
                content["qualifiers"].push_back(qualifier);
            }
            entities_with_facts_array.push_back(content);
        }
        return entities_with_facts_array.dump();
    }

    static std::string _inner_contents(std::shared_ptr<std::vector<const std::string* >> names, std::shared_ptr<Engine> executor_engine){
        json content;
        content["content"] = json::array();
        for (const auto& name : *names){
            content["content"].push_back(*name);
        }
        return content.dump();
    }

    static std::string _inner_contents(int num, std::shared_ptr<Engine> executor_engine){
        json content;
        content["content"] = std::to_string(num);
        return content.dump();
    }

    static std::string _inner_contents(std::shared_ptr<Engine::Values> values, std::shared_ptr<Engine> executor_engine){
        json content;
        content["content"] = json::array();
        for (const auto& value : *values){
            auto xx = value -> toPrintStr();
            content["content"].push_back(value -> toPrintStr());
        }
        return content.dump();
    }

    static std::string _inner_contents(VerifyResult verify_result, std::shared_ptr<Engine> executor_engine){
        json content;
        if (verify_result == VerifyResult::yes){
            content["content"] = "yes";
        }else if (verify_result == VerifyResult::no){
            content["content"] = "no";
        }else{
            content["content"] = "not sure";
        }
        return content.dump();
    }

public:
    std::string execute_program(std::vector<Function> * program, bool trace = false);
};

#endif //KOPL_EXECUTOR_H
