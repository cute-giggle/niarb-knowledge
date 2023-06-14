import json

from neo4j import GraphDatabase


uri = "bolt://localhost:7687"
username = "neo4j"
password = "12345678"


def insert_relation_triples(subject, relation, object):
    with GraphDatabase.driver(uri, auth=(username, password)) as driver:
        with driver.session() as session:
            cypher_query = "MERGE (s:Entity {name: $subject}) " \
                           "MERGE (o:Entity {name: $object}) " \
                           "MERGE (s)-[r:RELATION {name: $relation}]->(o)"
            session.run(cypher_query, subject=subject, relation=relation, object=object)

def batch_insert_relationship_triples(triples):
    with GraphDatabase.driver(uri, auth=(username, password)) as driver:
        with driver.session() as session:
            with session.begin_transaction() as tx:
                for triple in triples:
                    if len(triple) != 3:
                        continue
                    cypher_query = "CALL { " \
                                   "MERGE (s:Entity {name: $subject}) " \
                                   "MERGE (o:Entity {name: $object}) " \
                                   "MERGE (s)-[r:RELATION {name: $relation}]->(o) " \
                                   "}"
                    tx.run(cypher_query, subject=triple[0], relation=triple[1], object=triple[2])


batch_insert_relationship_triples(json.load(open('data/relation_triples_0.json')))
