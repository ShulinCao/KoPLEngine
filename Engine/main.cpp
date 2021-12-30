#include <iostream>

#include "engine.h"


int main() {
    std::cout << "Begin of Debug Program!" << std::endl;

    std::string file_name("../kb.json");
    Engine e(file_name);
    std::cout << "End of Debug Program!" << std::endl;


    auto find_italy = e.find("Italy");
    auto italy_relate = e.relateOp(find_italy, "film release region", "backward");

    auto find_batman = e.find("Batman Begins");

    auto and_res = e.andOp(italy_relate, find_batman);
    auto find_tootsie = e.find("Tootsie");

    auto two_select = std::make_shared<std::vector<int>>();
    two_select -> insert(two_select -> end(), find_tootsie -> begin(), find_tootsie -> end());
    two_select -> insert(two_select -> end(), and_res -> begin(), and_res -> end());

    std::cout << "-------\n";
    for (auto x : *two_select) {
        std::cout << "Entity: " << x << std::endl;
    }
    std::cout << "-------\n";

    auto final_result = e.selectAmong(two_select, "cost", SelectOperator::smallest);

    std::cout << (*and_res).size() << std::endl;
    std::cout << (*find_tootsie).size() << std::endl;
    std::cout << (*final_result).size() << std::endl;

    auto ans = (*final_result)[0];

    std::cout << (*ans) << std::endl;


//    for (auto x : *e.find("LeBron James")) {
//        std::cout << x << std::endl;
//    }

    return 0;
}