#ifndef KOPL_PYENGINE_H
#define KOPL_PYENGINE_H

#include "executor.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

using namespace std;

extern "C" {
    void foo(vector<int>* v, const char* FILE_NAME){
        string line;
        ifstream myfile(FILE_NAME);
        while (getline(myfile, line)) v->push_back(1);
    }

    vector<int>* new_vector(){
        return new vector<int>;
    }
    void delete_vector(vector<int>* v){
        cout << "destructor called in C++ for " << v << endl;
        delete v;
    }
    int vector_size(vector<int>* v){
        return v->size();
    }
    int vector_get(vector<int>* v, int i){
        return v->at(i);
    }
    void vector_push_back(vector<int>* v, int i){
        v->push_back(i);
    }
}


extern "C" {
    vector<string> * new_string_vector(){
        return new vector<string>;
    }
    void delete_string_vector(vector<string> * v){
        delete v;
    }
    int string_vector_size(vector<string> * v){
        return v -> size();
    }
    const char * string_vector_get(vector<string> * v, int i){
        return (*v).at(i).c_str();
    }
    void string_vector_push_back(vector<string> * v, char * s){
        v -> push_back(string(s));
    }
}


extern "C" {
    Function * new_function(const char * fun_name, vector<string> * fun_args, int dep_a, int dep_b) {
        return new Function(std::string(fun_name), *fun_args, dep_a, dep_b);
    }
    void delete_function(Function * f) {
        delete f;
    }
    const char * print_function(Function * f) {
        char print_fun[500];
        sprintf(print_fun, "Function: %s\n", f -> function_name.c_str());

        char print_args[500];
        sprintf(print_args, "Arguments:\n");
        for (auto x : f -> function_args) {
            char p_args[500];
            sprintf(p_args, "- %s\n", x.c_str());
            strcat(print_args, p_args);
        }

        strcat(print_fun, print_args);
        sprintf(print_args, "Dependency:\n");
        strcat(print_fun, print_args);

        for (auto x : f -> dependencies) {
            if (x > 0) {
                sprintf(print_args, "%d\n", x);
                strcat(print_fun, print_args);
            }
        }
        return print_fun;
    }
}


extern "C" {
    vector<Function> * new_program() {
        return new vector<Function>;
    }
    void delete_program(vector<Function> * p) {
        delete p;
    }
    int program_size(vector<Function> * v) {
        return v -> size();
    }
    Function * program_function_get(vector<Function> * v, int i) {
        return &(v -> at(i));
    }
    void program_push_back(vector<Function> * v, Function * x){
        v -> emplace_back(x -> function_name, x -> function_args, x -> dependencies[0], x -> dependencies[1]);
    }
}


extern "C" {
    Executor * init(const char * kb_file_name, int worker_num);
    void execute(Executor * e, vector<Function> * program, char * result, bool trace = false);
}


#endif //KOPL_PYENGINE_H
