import json


def read_json(file):
    with open(file, 'r') as f:
        return json.load(f)
    

relation_triples = []
for file in ['aparc-brodmann.json', 'aparc-shaefer-400-7.json', 'brodmann-shaefer-400-7.json']:
    data = read_json(file)
    for first, value in data.items():
        for second, relation in value.items():
            relation_triples.append([first, relation['forward'], second])
            relation_triples.append([second, relation['backward'], first])

with open('relation_triples.json', 'w') as f:
    json.dump(relation_triples, f, indent=4)