from ctypes import *

class IntVector(object):
    lib = cdll.LoadLibrary('cmake-build-debug/libKoPL.dylib') # class level loading lib

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
        self.vector = IntVector.lib.new_vector()  # pointer to new vector

    def __del__(self):  # when reference count hits 0 in Python,
        IntVector.lib.delete_vector(self.vector)  # call C++ vector destructor

    def __len__(self):
        return IntVector.lib.vector_size(self.vector)

    def __getitem__(self, i):  # access elements in vector at index
        if 0 <= i < len(self):
            return IntVector.lib.vector_get(self.vector, c_int(i))
        raise IndexError('Vector index out of range')

    def __repr__(self):
        return '[{}]'.format(', '.join(str(self[i]) for i in range(len(self))))

    def push(self, i):  # push calls vector's push_back
        IntVector.lib.vector_push_back(self.vector, c_int(i))

    def foo(self, filename):  # foo in Python calls foo in C++
        IntVector.lib.foo(self.vector, c_char_p(filename))


class StringVector(object):
    lib = cdll.LoadLibrary('cmake-build-debug/libKoPL.dylib')

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
        self.string_vector = StringVector.lib.new_string_vector()

    def __del__(self):
        StringVector.lib.delete_string_vector(self.string_vector)

    def __len__(self):
        return StringVector.lib.string_vector_size(self.string_vector)

    def __getitem__(self, i):  # access elements in vector at index
        if 0 <= i < len(self):
            return '"' + StringVector.lib.string_vector_get(self.string_vector, c_int(i)).decode("utf-8") + '"'
        raise IndexError('String Vector index out of range')

    def __repr__(self):
        return '[{}]'.format(', '.join(str(self[i]) for i in range(len(self))))

    def push(self, x):  # push calls vector's push_back
        x = x.encode("utf-8")
        StringVector.lib.string_vector_push_back(self.string_vector, c_char_p(x))

class Function(object):
    lib = cdll.LoadLibrary('cmake-build-debug/libKoPL.dylib')

    lib.new_function.restype = c_void_p
    lib.new_function.argtypes = [c_char_p, c_void_p, c_int, c_int]

    lib.delete_function.restype = None
    lib.delete_function.argtypes = [c_void_p]

    lib.print_function.restype = c_char_p
    lib.print_function.argtypes = [c_void_p]

    def __init__(self, fun_name, fun_args, dep_a, dep_b):
        self.function = Function.lib.new_function(fun_name.encode("utf-8"), fun_args.string_vector, dep_a, dep_b)

    def __del__(self):
        Function.lib.delete_function(self.function)

    def __str__(self):
        return Function.lib.print_function(self.function).decode("utf-8")

    def __repr__(self):
        return str(self)

if __name__ == "__main__":
    fun_args = StringVector()
    fun_args.push("height")
    fun_args.push("220 m")
    function = Function("FilterNum", fun_args, 1, -2)

    print(function)
