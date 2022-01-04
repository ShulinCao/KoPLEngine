#include <iostream>

#include "engine.h"
#include "executor.h"
#include <chrono>

int main() {
    std::cout << "Begin of Debug Program!" << std::endl;

    std::string file_name("../kb.json");

    Engine e(file_name);

    std::cout << "End of Debug Program!" << std::endl;

    auto executor = Executor(e);


    json kopl;
    std::ifstream kopl_file;
    kopl_file.open("../kopl_sample.json", std::ios::in);

    std::string kopl_str;

    kopl_file >> kopl;




    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;




    const auto & program = kopl.at(0);
    std::vector<Function> function;
    for (const auto & funct : program.at("program")) {
        const auto funct_name = funct.at("function").get<std::string>();

        const auto funct_args = funct.at("inputs");

        std::vector<std::string> args;
        for (const auto & arg : funct_args) {
            args.push_back(arg.get<std::string>());
        }

        const auto funct_deps = funct.at("dependencies");
        int dep_a = -1, dep_b = -1;
        if (funct_deps.size() > 0) {
            dep_a = funct_deps.at(0).get<int>();
        }
        if (funct_deps.size() == 2) {
            dep_b = funct_deps.at(1).get<int>();
        }

        function.emplace_back(funct_name, args, dep_a, dep_b);
    }


    std::vector<std::string> answers;

    auto t1 = high_resolution_clock::now();
    for (int i = 0; i < 5000; i++) {
        auto ans = executor.execute_program(function);
        answers.push_back(ans);
    }
    auto t2 = high_resolution_clock::now();
    auto ms_int = duration_cast<milliseconds>(t2 - t1);
    duration<double, std::milli> ms_double = t2 - t1;

    std::cout << ms_int.count() << "ms\n";
    std::cout << ms_double.count() << "ms\n";



    exit(0);



    for (const auto & program : kopl) {

        std::vector<Function> function;



        for (const auto & funct : program.at("program")) {
            const auto funct_name = funct.at("function").get<std::string>();

            const auto funct_args = funct.at("inputs");

            std::vector<std::string> args;
            for (const auto & arg : funct_args) {
                args.push_back(arg.get<std::string>());
            }

            const auto funct_deps = funct.at("dependencies");
            int dep_a = -1, dep_b = -1;
            if (funct_deps.size() > 0) {
                dep_a = funct_deps.at(0).get<int>();
            }
            if (funct_deps.size() == 2) {
                dep_b = funct_deps.at(1).get<int>();
            }

            function.emplace_back(funct_name, args, dep_a, dep_b);
        }

        std::cout << "Gold Answer: " << program.at("answer").get<std::string>() << std::endl;
        std::cout << "Number of Functions: " << function.size() << std::endl;
        auto ans = executor.execute_program(function);
        std::cout << ans << std::endl;

        std::cout << std::endl;
    }

//    std::vector<Function> func;
//
//
//    func.emplace_back("Find", "", "Georgia national football team", "");
//    func.emplace_back("QueryAttrQualifier", "ranking", "78", "review score by", 0);
//
//    std::cout << "End of emplace back\n";
//
//    auto res = executor.execute_program(func);
//    std::cout << res << std::endl;

    return 0;
}