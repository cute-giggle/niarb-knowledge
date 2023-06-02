import json


with open('relations.json', 'r') as f:
    relations = json.load(f)

relation_triples = []
for region_name, relation in relations.items():
    relation_triples.extend(relation)

with open('relation_triples.json', 'w') as f:
    json.dump(relation_triples, f, indent=4)