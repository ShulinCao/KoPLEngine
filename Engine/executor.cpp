#include "executor.h"

std::string Executor::execute_program(std::vector<Function> * program, bool trace) {
    EntityWithFactBuffer    entity_with_fact_buffer;
    ValuesBuffer            value_buffer;
    std::vector<int> entity_with_fact_indicator;
    std::vector<int> value_indicator;

    entity_with_fact_buffer     .resize(program -> size());
    value_buffer                .resize(program -> size());
    entity_with_fact_indicator  .resize(program -> size());
    value_indicator             .resize(program -> size());
    for (int & x : entity_with_fact_indicator)     x = 0;
    for (int & x : value_indicator)                x = 0;

    std::string             answer;

    try {
        for (std::size_t i = 0; i < program -> size(); i++) {
            auto cur_function = (*program)[i];

            if (trace) {
                std::cout << "Exe " << i << " " << cur_function.function_name << ", Dep ";
                std::cout << cur_function.dependencies[0] << " " << cur_function.dependencies[1] << std::endl;
            }

            if ((cur_function.dependencies[0] >= 0 && cur_function.dependencies[0] > i) ||
                (cur_function.dependencies[1] >= 0 && cur_function.dependencies[1] > i)) {
                std::cerr << "Dependency Error, i = " << cur_function.function_name << std::endl;
                answer = "Dependency Error!\n";
                break;
            }

            // 1. Get Buffer    (optional)
            // 2. Execution
            // 3. Store Buffer  (optional)
            // 4. Save Answer   (optional)

            if (cur_function.function_name == "FindAll") {
                if (i < program -> size() - 1 && (*program)[i + 1].function_name == "FilterStr") {
                    (*program)[i + 1].function_name = "FindAllFilterStr";
                    continue;
                }

                auto function_res = executor_engine -> findAll();

                entity_with_fact_buffer[i] = function_res;
                entity_with_fact_indicator[i] = 1;
            }
            else if (cur_function.function_name == "Find") {
                auto function_res = executor_engine -> find(cur_function.function_args[0]);

                entity_with_fact_buffer[i] = function_res;
                entity_with_fact_indicator[i] = 1;
            }
            else if (cur_function.function_name == "FilterConcept") {
                if (!entity_with_fact_indicator[cur_function.dependencies[0]]) {
                    std::cerr << "Dependency Error, i = " << cur_function.function_name << std::endl;
                    answer = "Dependency Error!\n";
                    break;
                }
                auto dependency_a = entity_with_fact_buffer[cur_function.dependencies[0]];

                auto function_res = executor_engine -> filterConcept(
                        dependency_a,

                        cur_function.function_args[0]
                );

                entity_with_fact_buffer[i] = function_res;
                entity_with_fact_indicator[i] = 1;
            }
            else if (cur_function.function_name == "FilterStr") {
                if (!entity_with_fact_indicator[cur_function.dependencies[0]]) {
                    std::cerr << "Dependency Error, i = " << cur_function.function_name << std::endl;
                    answer = "Dependency Error!\n";
                    break;
                }
                auto dependency_a = entity_with_fact_buffer[cur_function.dependencies[0]];

                auto function_res = executor_engine -> filterStr(
                        dependency_a,

                        cur_function.function_args[0],
                        cur_function.function_args[1]
                );

                entity_with_fact_buffer[i] = function_res;
                entity_with_fact_indicator[i] = 1;
            }
            else if (cur_function.function_name == "FindAllFilterStr") {
                auto function_res = executor_engine -> findAllFilterStr(
                        cur_function.function_args[0],
                        cur_function.function_args[1]
                );

//                std::cout << "Hey I am Here\n";

                entity_with_fact_buffer[i] = function_res;
                entity_with_fact_indicator[i] = 1;
            }
            else if (cur_function.function_name == "FilterNum") {
                if (!entity_with_fact_indicator[cur_function.dependencies[0]]) {
                    std::cerr << "Dependency Error, i = " << cur_function.function_name << std::endl;
                    answer = "Dependency Error!\n";
                    break;
                }
                auto dependency_a = entity_with_fact_buffer[cur_function.dependencies[0]];

                auto function_res = executor_engine -> filterNum(
                        dependency_a,

                        cur_function.function_args[0],
                        cur_function.function_args[1],
                        cur_function.function_args[2]
                );

                entity_with_fact_buffer[i] = function_res;
                entity_with_fact_indicator[i] = 1;
            }
            else if (cur_function.function_name == "FilterYear") {
                if (!entity_with_fact_indicator[cur_function.dependencies[0]]) {
                    std::cerr << "Dependency Error, i = " << cur_function.function_name << std::endl;
                    answer = "Dependency Error!\n";
                    break;
                }
                auto dependency_a = entity_with_fact_buffer[cur_function.dependencies[0]];

                auto function_res = executor_engine -> filterYear(
                        dependency_a,

                        cur_function.function_args[0],
                        cur_function.function_args[1],
                        cur_function.function_args[2]
                );

                entity_with_fact_buffer[i] = function_res;
                entity_with_fact_indicator[i] = 1;
            }
            else if (cur_function.function_name == "FilterDate") {
                if (!entity_with_fact_indicator[cur_function.dependencies[0]]) {
                    std::cerr << "Dependency Error, i = " << cur_function.function_name << std::endl;
                    answer = "Dependency Error!\n";
                    break;
                }
                auto dependency_a = entity_with_fact_buffer[cur_function.dependencies[0]];

                auto function_res = executor_engine -> filterDate(
                        dependency_a,

                        cur_function.function_args[0],
                        cur_function.function_args[1],
                        cur_function.function_args[2]
                );

                entity_with_fact_buffer[i] = function_res;
                entity_with_fact_indicator[i] = 1;
            }
            else if (cur_function.function_name == "QFilterStr") {
                if (!entity_with_fact_indicator[cur_function.dependencies[0]]) {
                    std::cerr << "Dependency Error, i = " << cur_function.function_name << std::endl;
                    answer = "Dependency Error!\n";
                    break;
                }
                auto dependency_a = entity_with_fact_buffer[cur_function.dependencies[0]];

                auto function_res = executor_engine -> QfilterStr(
                        dependency_a,

                        cur_function.function_args[0],
                        cur_function.function_args[1]
                );

                entity_with_fact_buffer[i] = function_res;
                entity_with_fact_indicator[i] = 1;
            }
            else if (cur_function.function_name == "QFilterNum") {
                if (!entity_with_fact_indicator[cur_function.dependencies[0]]) {
                    std::cerr << "Dependency Error, i = " << cur_function.function_name << std::endl;
                    answer = "Dependency Error!\n";
                    break;
                }
                auto dependency_a = entity_with_fact_buffer[cur_function.dependencies[0]];

                auto function_res = executor_engine -> QfilterNum(
                        dependency_a,

                        cur_function.function_args[0],
                        cur_function.function_args[1],
                        cur_function.function_args[2]
                );

                entity_with_fact_buffer[i] = function_res;
                entity_with_fact_indicator[i] = 1;
            }
            else if (cur_function.function_name == "QFilterYear") {
                if (!entity_with_fact_indicator[cur_function.dependencies[0]]) {
                    std::cerr << "Dependency Error, i = " << cur_function.function_name << std::endl;
                    answer = "Dependency Error!\n";
                    break;
                }
                auto dependency_a = entity_with_fact_buffer[cur_function.dependencies[0]];

                auto function_res = executor_engine -> QfilterYear(
                        dependency_a,

                        cur_function.function_args[0],
                        cur_function.function_args[1],
                        cur_function.function_args[2]
                );

                entity_with_fact_buffer[i] = function_res;
                entity_with_fact_indicator[i] = 1;
            }
            else if (cur_function.function_name == "QFilterDate") {
                if (!entity_with_fact_indicator[cur_function.dependencies[0]]) {
                    std::cerr << "Dependency Error, i = " << cur_function.function_name << std::endl;
                    answer = "Dependency Error!\n";
                    break;
                }
                auto dependency_a = entity_with_fact_buffer[cur_function.dependencies[0]];

                auto function_res = executor_engine -> QfilterDate(
                        dependency_a,

                        cur_function.function_args[0],
                        cur_function.function_args[1],
                        cur_function.function_args[2]
                );

                entity_with_fact_buffer[i] = function_res;
                entity_with_fact_indicator[i] = 1;
            }
            else if (cur_function.function_name == "Relate") {
                if (!entity_with_fact_indicator[cur_function.dependencies[0]]) {
                    std::cerr << "Dependency Error, i = " << cur_function.function_name << std::endl;
                    answer = "Dependency Error!\n";
                    break;
                }
                auto dependency_a = entity_with_fact_buffer[cur_function.dependencies[0]];

                auto function_res = executor_engine -> relateOp(
                        dependency_a,

                        cur_function.function_args[0],
                        cur_function.function_args[1]
                );

                entity_with_fact_buffer[i] = function_res;
                entity_with_fact_indicator[i] = 1;
            }
            else if (cur_function.function_name == "And") {
                if (!entity_with_fact_indicator[cur_function.dependencies[0]]) {
                    std::cerr << "Dependency Error, i = " << cur_function.function_name << std::endl;
                    answer = "Dependency Error!\n";
                    break;
                }
                if (!entity_with_fact_indicator[cur_function.dependencies[1]]) {
                    std::cerr << "Dependency Error, i = " << cur_function.function_name << std::endl;
                    answer = "Dependency Error!\n";
                    break;
                }
                auto dependency_a = entity_with_fact_buffer[cur_function.dependencies[0]];
                auto dependency_b = entity_with_fact_buffer[cur_function.dependencies[1]];

                auto function_res = executor_engine -> andOp(
                        dependency_a,
                        dependency_b
                );

                entity_with_fact_buffer[i] = function_res;
                entity_with_fact_indicator[i] = 1;
            }
            else if (cur_function.function_name == "Or") {
                if (!entity_with_fact_indicator[cur_function.dependencies[0]]) {
                    std::cerr << "Dependency Error, i = " << cur_function.function_name << std::endl;
                    answer = "Dependency Error!\n";
                    break;
                }
                if (!entity_with_fact_indicator[cur_function.dependencies[1]]) {
                    std::cerr << "Dependency Error, i = " << cur_function.function_name << std::endl;
                    answer = "Dependency Error!\n";
                    break;
                }
                auto dependency_a = entity_with_fact_buffer[cur_function.dependencies[0]];
                auto dependency_b = entity_with_fact_buffer[cur_function.dependencies[1]];

                auto function_res = executor_engine -> orOp(
                        dependency_a,
                        dependency_b
                );

                entity_with_fact_buffer[i] = function_res;
                entity_with_fact_indicator[i] = 1;
            }
            else if (cur_function.function_name == "What") { // Query Name
                if (!entity_with_fact_indicator[cur_function.dependencies[0]]) {
                    std::cerr << "Dependency Error, i = " << cur_function.function_name << std::endl;
                    answer = "Dependency Error!\n";
                    break;
                }
                auto dependency_a = entity_with_fact_buffer[cur_function.dependencies[0]];

                auto function_res = executor_engine -> queryName(
                        dependency_a
                );

                // TODO: Ugly Now
                if (function_res -> empty()) {
                    answer = "no";
                }
                else {
                    answer = *((*function_res)[0]);
                }
            }
            else if (cur_function.function_name == "Count") {
                if (!entity_with_fact_indicator[cur_function.dependencies[0]]) {
                    std::cerr << "Dependency Error, i = " << cur_function.function_name << std::endl;
                    answer = "Dependency Error!\n";
                    break;
                }
                auto dependency_a = entity_with_fact_buffer[cur_function.dependencies[0]];

                auto function_res = executor_engine -> countOp(
                        dependency_a
                );

                // Answer
                answer = _obtain_result(function_res);
            }
            else if (cur_function.function_name == "QueryAttr") {
                auto dependency_a = entity_with_fact_buffer[cur_function.dependencies[0]];

                auto function_res = executor_engine -> queryAttr(
                        dependency_a,

                        cur_function.function_args[0]
                );

                value_buffer[i] = function_res;
                value_indicator[i] = 1;

                // Answer
                if (function_res -> empty()) {
                    answer = "no";
                }
                else {
                    answer = _obtain_result((*function_res)[0]);
                }
            }
            else if (cur_function.function_name == "QueryAttrUnderCondition") {
                auto dependency_a = entity_with_fact_buffer[cur_function.dependencies[0]];

                auto function_res = executor_engine -> queryAttrUnderCondition(
                        dependency_a,

                        cur_function.function_args[0],
                        cur_function.function_args[1],
                        cur_function.function_args[2]
                );

                value_buffer[i] = function_res;
                value_indicator[i] = 1;
            }
            else if (cur_function.function_name == "QueryRelation") {
                auto dependency_a = entity_with_fact_buffer[cur_function.dependencies[0]];
                auto dependency_b = entity_with_fact_buffer[cur_function.dependencies[1]];

                auto function_res = executor_engine -> queryRelation(
                        dependency_a,
                        dependency_b
                );

                // TODO: Ugly Now
                if (function_res -> empty()) {
                    answer = "no";
                }
                else {
                    answer = *((*function_res)[0]);
                }
            }
            else if (cur_function.function_name == "SelectBetween") {
                auto dependency_a = entity_with_fact_buffer[cur_function.dependencies[0]];
                auto dependency_b = entity_with_fact_buffer[cur_function.dependencies[1]];

                auto function_res = executor_engine -> selectBetween(
                        dependency_a,
                        dependency_b,

                        cur_function.function_args[0],
                        cur_function.function_args[1]
                );

                // TODO: Ugly Now
                if (function_res -> empty()) {
                    answer = "no";
                }
                else {
                    answer = *((*function_res)[0]);
                }
            }
            else if (cur_function.function_name == "SelectAmong") {
                auto dependency_a = entity_with_fact_buffer[cur_function.dependencies[0]];

                auto function_res = executor_engine -> selectAmong(
                        dependency_a,

                        cur_function.function_args[0],
                        cur_function.function_args[1]
                );

                // TODO: Ugly Now
                if (function_res -> empty()) {
                    answer = "no";
                }
                else {
                    answer = *((*function_res)[0]);
                }

            }
            else if (cur_function.function_name == "VerifyStr") {
                auto dependency_a = value_buffer[cur_function.dependencies[0]];

                auto function_res = executor_engine -> verifyStr(
                        dependency_a,

                        cur_function.function_args[0]
                );

                answer = _obtain_result(function_res);
            }
            else if (cur_function.function_name == "VerifyNum") {
                auto dependency_a = value_buffer[cur_function.dependencies[0]];

                auto function_res = executor_engine -> verifyNum(
                        dependency_a,

                        cur_function.function_args[0],
                        cur_function.function_args[1]
                );

                answer = _obtain_result(function_res);
            }
            else if (cur_function.function_name == "VerifyYear") {
                auto dependency_a = value_buffer[cur_function.dependencies[0]];

                auto function_res = executor_engine -> verifyYear(
                        dependency_a,

                        cur_function.function_args[0],
                        cur_function.function_args[1]
                );

                answer = _obtain_result(function_res);
            }
            else if (cur_function.function_name == "VerifyDate") {
                auto dependency_a = value_buffer[cur_function.dependencies[0]];

                auto function_res = executor_engine -> verifyDate(
                        dependency_a,

                        cur_function.function_args[0],
                        cur_function.function_args[1]
                );

                answer = _obtain_result(function_res);
            }
            else if (cur_function.function_name == "QueryAttrQualifier") {
                auto dependency_a = entity_with_fact_buffer[cur_function.dependencies[0]];

                auto function_res = executor_engine -> queryAttrQualifier(
                        dependency_a,

                        cur_function.function_args[0],
                        cur_function.function_args[1],
                        cur_function.function_args[2]
                );

                // Answer
//            answer = _obtain_result((*function_res)[0]);
                if (function_res -> empty()) {
                    answer = "no";
                }
                else {
                    answer = _obtain_result((*function_res)[0]);
                }
            }
            else if (cur_function.function_name == "QueryRelationQualifier") {
                auto dependency_a = entity_with_fact_buffer[cur_function.dependencies[0]];
                auto dependency_b = entity_with_fact_buffer[cur_function.dependencies[1]];

                auto function_res = executor_engine -> queryRelationQualifier(
                        dependency_a,
                        dependency_b,

                        cur_function.function_args[0],
                        cur_function.function_args[1]
                );

                // Answer
//            answer = _obtain_result((*function_res)[0]);
                if (function_res -> empty()) {
                    answer = "no";
                }
                else {
                    answer = _obtain_result((*function_res)[0]);
                }
            }
            else {
                answer = "Unsupported Operator!";
                break;
            }
        }
    }
    catch (const std::out_of_range & oor) {
        std::cerr << "Out of Range error: " << oor.what() << '\n';
        answer = "Not Found in KB";
    }

    return answer;
}
