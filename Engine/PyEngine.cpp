#include "PyEngine.h"

Executor * init(const char * kb_file_name, int worker_num) {
    std::string kb_file_name_in_cpp_string{kb_file_name};
    return new Executor(kb_file_name_in_cpp_string, worker_num);
}

const char * execute(Executor * e, std::vector<Function> * program, bool trace) {
    auto res = e -> execute_program(program, trace);
    return res.c_str();
}