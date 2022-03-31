#ifndef KOPL_EXECUTOR_H
#define KOPL_EXECUTOR_H

#include "engine.h"


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

public:
    std::string execute_program(std::vector<Function> * program, bool trace = false);
};


#endif //KOPL_EXECUTOR_H
