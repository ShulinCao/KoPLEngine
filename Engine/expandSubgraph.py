import engine
import json

def test_expannd_from_entities(executor):
    entity_ids = engine.StringVector()
    entity_ids.push("Q786")
    return engine.expand_from(executor, entity_ids, 1)

if __name__ == "__main__":
    executor = engine.init("../kb.json")
    container = test_expannd_from_entities(executor)
    for i in container:
        print(i)