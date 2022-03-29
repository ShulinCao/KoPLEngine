#include "PyEngine.h"
#include <cstring>

Executor * init(const char * kb_file_name, int worker_num) {
    std::string kb_file_name_in_cpp_string{kb_file_name};
    return new Executor(kb_file_name_in_cpp_string, worker_num);
}

const char * execute(Executor * e, std::vector<Function> * program, char * print_buffer, bool trace) {
    auto res = e -> execute_program(program, trace);
    strcpy(print_buffer, res.c_str());
    return print_buffer;
}

void expand_from_entities(Executor * e, Engine::GraphContainer * container, vector<string> * entity_ids, int jump_limitation) {
    auto res = e -> expand_from_entities(entity_ids, jump_limitation);
    container -> entity_ids.insert(container -> entity_ids.end(),
                                   res -> entity_ids.begin(),
                                   res -> entity_ids.end());
    //for (auto i : container -> entity_ids) {
    //    std::cout << i << ' ';
    //}
    //std::cout << std::endl;
    container -> entity_attributes.insert(container -> entity_attributes.end(),
                                          res -> entity_attributes.begin(),
                                          res -> entity_attributes.end());
    //for (auto i : container -> entity_attributes) {
    //    std::cout << i << ' ';
    //}
    //std::cout << std::endl;
    container -> entity_relations.insert(container -> entity_relations.end(),
                                         res -> entity_relations.begin(),
                                         res -> entity_relations.end());
    //for (auto i : container -> entity_relations) {
    //    std::cout << i << ' ';
    //}
    //std::cout << std::endl;
    //std::cout << container -> entity_ids.size() << " " << container -> entity_attributes.size() << " " 
    //    << container -> entity_relations.size() << std::endl;
    res.reset();
}
