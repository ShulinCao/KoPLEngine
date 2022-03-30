import json
from ctypes import *
import time
import faulthandler
faulthandler.enable()
from tqdm import  tqdm

# lib = cdll.LoadLibrary('cmake-build-remote19/libKoPL.so')
# lib = cdll.LoadLibrary('cmake-build-debug/libKoPL.dylib')
lib = cdll.LoadLibrary('../build/libKoPL.so')

lib.new_char_p.restype = c_void_p
lib.new_char_p.argtypes = []

lib.delete_char_p.restype = None
lib.delete_char_p.argtypes = [c_void_p]


class IntVector(object):
    lib.new_vector.restype = c_void_p
    lib.new_vector.argtypes = []

    lib.delete_vector.restype = None
    lib.delete_vector.argtypes = [c_void_p]

    lib.vector_size.restype = c_int
    lib.vector_size.argtypes = [c_void_p]

    lib.vector_get.restype = c_int
    lib.vector_get.argtypes = [c_void_p, c_int]

    lib.vector_push_back.restype = None
    lib.vector_push_back.argtypes = [c_void_p, c_int]

    lib.foo.restype = None
    lib.foo.argtypes = [c_void_p]

    def __init__(self):
        self.vector = lib.new_vector()  # pointer to new vector

    def __del__(self):  # when reference count hits 0 in Python,
        lib.delete_vector(self.vector)  # call C++ vector destructor

    def __len__(self):
        return lib.vector_size(self.vector)

    def __getitem__(self, i):  # access elements in vector at index
        if 0 <= i < len(self):
            return lib.vector_get(self.vector, c_int(i))
        raise IndexError('Vector index out of range')

    def __repr__(self):
        return '[{}]'.format(', '.join(str(self[i]) for i in range(len(self))))

    def push(self, i):  # push calls vector's push_back
        lib.vector_push_back(self.vector, c_int(i))

    def foo(self, filename):  # foo in Python calls foo in C++
        lib.foo(self.vector, c_char_p(filename))



class StringVector(object):
    lib.new_string_vector.restype = c_void_p
    lib.new_string_vector.argtypes = []

    lib.delete_string_vector.restype = None
    lib.delete_string_vector.argtypes = [c_void_p]

    lib.string_vector_size.restype = c_int
    lib.string_vector_size.argtypes = [c_void_p]

    lib.string_vector_get.restype = c_char_p
    lib.string_vector_get.argtypes = [c_void_p, c_int]

    lib.string_vector_push_back.restype = None
    lib.string_vector_push_back.argtypes = [c_void_p, c_char_p]

    def __init__(self):
        self.string_vector = lib.new_string_vector()

    def __del__(self):
        lib.delete_string_vector(self.string_vector)

    def __len__(self):
        return lib.string_vector_size(self.string_vector)

    def __getitem__(self, i):  # access elements in vector at index
        if 0 <= i < len(self):
            return '"' + lib.string_vector_get(self.string_vector, c_int(i)).decode("utf-8") + '"'
        raise IndexError('String Vector index out of range')

    def __repr__(self):
        return '[{}]'.format(', '.join(str(self[i]) for i in range(len(self))))

    def push(self, x):  # push calls vector's push_back
        x = x.encode("utf-8")
        lib.string_vector_push_back(self.string_vector, c_char_p(x))

class GraphContainer(object):
    lib.new_graph_container.restype = c_void_p
    lib.new_graph_container.argtypes = []

    lib.delete_graph_container.restype = None
    lib.delete_graph_container.argtypes = [c_void_p]

    lib.get_entity_id_at.restype = c_char_p
    lib.get_entity_id_at.argtypes = [c_void_p, c_int]

    lib.get_entity_attribute_at.restype = c_char_p
    lib.get_entity_attribute_at.argtypes = [c_void_p, c_int]

    lib.get_entity_relation_at.restype = c_char_p
    lib.get_entity_relation_at.argtypes = [c_void_p, c_int]

    lib.graph_container_size.restype = c_int
    lib.graph_container_size.argtypes = [c_void_p]

    def __init__(self):
        self.container = lib.new_graph_container()

    def __del__(self):
        lib.delete_graph_container(self.container)

    def __len__(self):
        return lib.graph_container_size(self.container)

    def __getitem__(self, i):
        if 0 <= i < len(self):
            rela = json.loads(lib.get_entity_relation_at(self.container, c_int(i)).decode('utf-8'))
            attr = json.loads(lib.get_entity_attribute_at(self.container, c_int(i)).decode('utf-8'))
            item = {lib.get_entity_id_at(self.container, c_int(i)).decode('utf-8'): {'attributes': attr, 'relations': rela}}
            return item
        raise IndexError('Graph Container index out of range')

    def __repr__(self):
        return '[{}]'.format(', '.join(str(self[i]) for i in range(len(self))))


class Function(object):
    lib.new_function.restype = c_void_p
    lib.new_function.argtypes = [c_char_p, c_void_p, c_int, c_int]

    lib.delete_function.restype = None
    lib.delete_function.argtypes = [c_void_p]

    lib.print_function.restype = c_char_p
    lib.print_function.argtypes = [c_void_p, c_void_p]

    def __init__(self, fun_name, fun_args, dep_a, dep_b):
        self.function = lib.new_function(fun_name.encode("utf-8"), fun_args.string_vector, dep_a, dep_b)

    def __del__(self):
        lib.delete_function(self.function)

    def __str__(self):
        buffer = lib.new_char_p()
        ret_ptr = lib.print_function(self.function, buffer)
        ret_str = cast(ret_ptr, c_char_p).value.decode("utf-8")
        lib.delete_char_p(buffer)
        return ret_str

    def __repr__(self):
        buffer = lib.new_char_p()
        ret_ptr = lib.print_function(self.function, buffer)
        ret_str = cast(ret_ptr, c_char_p).value.decode("utf-8")
        lib.delete_char_p(buffer)
        return ret_str



class Program(object):

    lib.new_program.restype = c_void_p
    lib.new_program.argtypes = []

    lib.delete_program.restype = None
    lib.delete_program.argtypes = [c_void_p]

    lib.program_size.restype = c_int
    lib.program_size.argtypes = [c_void_p]

    lib.program_function_get.restype = c_void_p
    lib.program_function_get.argtypes = [c_void_p, c_int]

    lib.program_push_back.restype = None
    lib.program_push_back.argtypes = [c_void_p, c_void_p]

    def __init__(self):
        self.program = lib.new_program()

    def __del__(self):
        lib.delete_program(self.program)

    def __len__(self):
        return lib.program_size(self.program)

    def push(self, x):
        lib.program_push_back(self.program, x.function)

    def __getitem__(self, i):
        if 0 <= i < len(self):
            return lib.program_function_get(self.program, i)
        raise IndexError('Program index out of range')

    def __repr__(self):
        ret_str = "[\n"

        buffer = lib.new_char_p()
        for i in range(len(self)):
            ret_ptr = lib.print_function(self[i], buffer)
            ret_str += cast(ret_ptr, c_char_p).value.decode("utf-8")
        lib.delete_char_p(buffer)

        ret_str += "]"
        return ret_str

    def __str__(self):
        ret_str = "[\n"

        buffer = lib.new_char_p()
        for i in range(len(self)):
            ret_ptr = lib.print_function(self[i], buffer)
            ret_str += cast(ret_ptr, c_char_p).value.decode("utf-8")
            ret_str += "\n++++++++++++++++++++\n"
        lib.delete_char_p(buffer)

        ret_str += "]"
        return ret_str





lib.init.restype = c_void_p
lib.init.argtypes = [c_char_p, c_int]
def init(kb_file_name, worker_num = 4):
    return lib.init(kb_file_name.encode("utf-8"), worker_num)

lib.execute.restype = c_char_p
lib.execute.argtypes = [c_void_p, c_void_p, c_void_p, c_bool]
def forward(executor, program, trace):
    buffer = lib.new_char_p()
    ret_ptr = lib.execute(executor, program.program, buffer, trace)
    ret_str = cast(ret_ptr, c_char_p).value.decode("utf-8")
    lib.delete_char_p(buffer)
    return ret_str


def parse_program(prog : dict):
    program = Program()
    for j, func in enumerate(prog):
        func_name = func["function"]
        func_deps = func["dependencies"]
        func_inpt = func["inputs"]


        dep_a = -1
        dep_b = -2
        if len(func_deps) > 0:
            dep_a = func_deps[0]
        if len(func_deps) > 1:
            dep_b = func_deps[1]

        function_arguments = StringVector()
        for x in func_inpt:
            function_arguments.push(x)

        function = Function(func_name, function_arguments, dep_a, dep_b)

        program.push(function)
    return program

lib.expand_from_entities.restype = None
lib.expand_from_entities.argtypes = [c_void_p, c_void_p, c_void_p, c_int]
def expand_from(executor, entity_ids, jump_limitation):
    container = GraphContainer()
    lib.expand_from_entities(executor, container.container, entity_ids.string_vector, c_int(jump_limitation))
    return container


if __name__ == "__main__":
    executor = init("../kb.json")
#    container = test_expannd_from_entities(executor)
#    for ent in container:
#        print(ent)

#    executor = init("/data/lvxin/kopl/KoPL/src/en_zh_wikipedia_entities_with_concept_filter_final_with_kqa_kb_with_reverse.json")

    # programs = json.load(open("kopl_sample.json"))

    programs = json.load(open("../problem.json"))


    functions = []

    s = time.time()
    total_num = error_num = 0
    for i in tqdm(range(len(programs))):
        total_num += 1
    # for i in range(len(programs)):

        ansr = programs[i]["answer"]
        prog = parse_program(programs[i]["program"])

        pred = forward(executor, prog, False)
        if (ansr != pred):
            print(i, ansr, pred)
            error_num += 1

    print(error_num / total_num)

    e = time.time()
    print(e - s)

