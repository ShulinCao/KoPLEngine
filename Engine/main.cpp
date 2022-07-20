#include <iostream>

#include "engine.h"
#include "executor.h"
#include <chrono>

int main() {
    std::cout << "Begin of Debug Program!" << std::endl;

    std::string file_name("../kb.json");
//    std::string file_name("/data/lvxin/kopl/KoPL/src/en_zh_wikipedia_entities_with_concept_filter_final_with_kqa_kb_with_reverse.json");
//    std::string file_name("/data/lvxin/kopl/KoPL/src/en_zh_wikipedia_entities_with_concept_filter_final.json");

    auto executor = Executor(file_name, 4);
    std::cout << "End of Debug Program!\n" << std::endl;

//    Engine e(file_name);
//    std::cout << "End of Debug Program!\n" << std::endl;
//    auto executor = Executor(e);


    json kopl;
    std::ifstream kopl_file;
    kopl_file.open("../kopl_sample.json", std::ios::in);

    std::string kopl_str;

    kopl_file >> kopl;

    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;


//    for (const auto & program : kopl) {

//    std::set<int> train_problem_program{10663, 32750, 37732, 43382, 43965, 48275, 61297, 68401, 73447,
//                                        80537,  // Python Bug need to be fixed
//                                        86875
//    };

    std::set<int> train_problem_program{};

    std::ofstream debug_file("debug.txt", std::ios::out);

    bool trace = true;

    auto t1 = high_resolution_clock::now();
//    for (auto i : train_problem_program) {
    for (std::size_t i = 0; i < kopl.size(); i++) {
        const auto & program = kopl.at(i);
//            if (program.at("answer").get<std::string>() != "no" &&
//                    train_problem_program.find(i) == train_problem_program.end()) {
        if (true) {

            if (trace) {
                std::cout << "Program i = " << i << std::endl;
            }
            else if (i % 100 == 0) {
                std::cout << "Program i = " << i << std::endl;
            }
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


//                std::cout << "Number of Functions: " << function.size() << std::endl;
            auto ans = executor.execute_program(&function, trace);
//                std::cout << ans << std::endl;

            if (trace) {
                std::cout << "Gold Answer: " << program.at("answer").get<std::string>() << std::endl;
                std::cout << "Pred Answer: " << ans << std::endl;
                std::cout << std::endl;
            }

            if (ans != program.at("answer").get<std::string>()) {
                debug_file << "Program i = " << i << std::endl;
                debug_file << "Gold Answer: " << program.at("answer").get<std::string>() << std::endl;
                debug_file << "Pred Answer: " << ans << std::endl;
                debug_file << std::endl;
            }
        }
    }
    auto t2 = high_resolution_clock::now();
    auto ms_int = duration_cast<milliseconds>(t2 - t1);
    duration<double, std::milli> ms_double = t2 - t1;

    std::cout << ms_int.count() << "ms\n";
    std::cout << ms_double.count() << "ms\n";

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