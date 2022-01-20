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