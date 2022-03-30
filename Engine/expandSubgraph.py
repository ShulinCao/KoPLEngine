import engine
import json

def gen_small_graph():
    graph = {
        'concepts': {
            'c1': {
                'name': 'A',
                'subclassOf': []
            },
            'c2': {
                'name': 'B',
                'subclassOf': []
            },
            'c3': {
                'name': 'C',
                'subclassOf': []
            },
            'c4': {
                'name': 'D',
                'subclassOf': []
            },
            'c5': {
                'name': 'E',
                'subclassOf': []
            },
        },
        'entities': {
            'e1': {
                'name': 'a',
                'instanceOf': ['c1', 'c2'],
                'attributes': [
                    {
                        'key': 'len',
                        'value': {
                            'type': 'quantity',
                            'value': 1,
                            'unit': 'cm'
                        },
                        'qualifiers':{}
                    },
                ],
                'relations': [
                    {
                        'relation': 'captain of',
                        'direction': 'backward',
                        'object': 'c1',
                        'qualifiers': {}
                    },
                    {
                        'relation': 'in the front of',
                        'direction': 'backward',
                        'object': 'e2',
                        'qualifiers': {}
                    },
                    {
                        'relation': 'one in three',
                        'direction': 'forward',
                        'object': 'e3',
                        'qualifiers': {}
                    }
                ]

            },
            'e2': {
                'name': 'b',
                'instanceOf': ['c2'],
                'attributes': [
                    {
                        'key': 'wide',
                        'value': {
                            'type': 'quantity',
                            'value': 1,
                            'unit': 'km'
                        },
                        'qualifiers':{}
                    },
                ],
                'relations': [
                    {
                        'relation': 'big guy of',
                        'direction': 'backward',
                        'object': 'c1',
                        'qualifiers': {}
                    },
                    {
                        'relation': 'elder brother of',
                        'direction': 'forward',
                        'object': 'e3',
                        'qualifiers': {}
                    },
                ]

            },
            'e3': {
                'name': 'c',
                'instanceOf': ['c3','c4','c5'],
                'attributes': [
                    {
                        'key': 'height',
                        'value': {
                            'type': 'quantity',
                            'value': 1,
                            'unit': 'meter'
                        },
                        'qualifiers':{}
                    },
                ],
                'relations': []

            },
            'e4': {
                'name': 'd',
                'instanceOf': ['c4'],
                'attributes': [{
                        'key': 'first name',
                        'value': {
                            'type': 'string',
                            'value': 'jordan'
                        },
                        'qualifiers':{}
                    },],
                'relations': [
                    {
                        'relation': 'is father of',
                        'direction': 'backward',
                        'object': 'e5',
                        'qualifiers': {}
                    },
                ]

            },
            'e5': {
                'name': 'e',
                'instanceOf': ['c1','c5'],
                'attributes': [
                    {
                        'key': 'len',
                        'value': {
                            'type': 'year',
                            'value': 1999
                        },
                        'qualifiers':{}
                    },
                ],
                'relations': []

            }
        }
    }

    f = open('data.json', 'w')
    f.write(json.dumps(graph))
    f.close()

def test_expannd_from_entities(executor):
    entity_ids = engine.StringVector()
    entity_ids.push("e1")
    entity_ids.push("e4")
    return engine.expand_from(executor, entity_ids, 2)

if __name__ == "__main__":
    gen_small_graph()
    executor = engine.init("data.json")
    container = test_expannd_from_entities(executor)
    for i in container:
        print(i)