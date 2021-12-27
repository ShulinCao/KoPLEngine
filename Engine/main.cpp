#include <iostream>

#include "engine.h"


int main() {
    std::cout << "Begin of Debug Program!" << std::endl;

    std::string file_name("../kb.json");
    Engine e(file_name);
    std::cout << "End of Debug Program!" << std::endl;

    for (auto x : e.find("Mxay")) {
        std::cout << x << std::endl;
    }

    return 0;
}