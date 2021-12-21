from ctypes import *


class Entity(Structure):
    _fields_ = [

    ]


class Relation(Structure):
    _fields_ = [

    ]


class Node(Structure):
    _fields_ = [
        ("ID", c_long),                   # Node ID in the entity array
        ("out_edges", ),
        ("in_edges", )
    ]


class DataBase(Structure):
    _fields_ = [
        ("entities", ),
        ("relations", ),
        ("", )
    ]


class POINT(Structure):
    _fields_ = [
        ("x", c_int),
        ("y", c_int)
    ]


point = POINT(10, 11)


print("test")
print(point.x)
print(point.y)