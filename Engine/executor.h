#ifndef KOPL_EXECUTOR_H
#define KOPL_EXECUTOR_H

#include "engine.h"


class Function {
public:
    std::string                                                             function_name;
    std::vector<std::string>                                                function_args;
    int                                                                     dependencies[2] = {-1, -1};

    Function(
            std::string                 fun_name,
            std::vector<std::string>    fun_args,
            int                         dep_a = -1,
            int                         dep_b = -2
            ) : function_name(fun_name), function_args(fun_args) {
        dependencies[0] = dep_a;
        dependencies[1] = dep_b;
    }
};


typedef  std::vector<std::shared_ptr<Engine::EntitiesWithFacts>>            EntityWithFactBuffer;
typedef  std::vector<std::shared_ptr<Engine::Values>>                       ValuesBuffer;


class Executor {
public:
    Engine &                                                                executor_engine;
    explicit Executor(Engine & engine) : executor_engine(engine) {};

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

public:
    std::string execute_program(std::vector<Function> & program, bool trace = false);
};


#endif //KOPL_EXECUTOR_H
