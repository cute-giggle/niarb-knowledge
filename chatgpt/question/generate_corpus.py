import openai
import os
import json
import glob
import time

from openai.error import RateLimitError

os.environ['HTTP_PROXY'] = '192.168.1.34:7890'
os.environ['HTTPS_PROXY'] = '192.168.1.34:7890'

openai.api_key_path = '../data/api-key.txt'

def call(content: str):
    message = [
        {'role': 'user', 'content': content},
    ]
    response = openai.ChatCompletion.create(
        model='gpt-3.5-turbo',
        messages=message,
        temperature=0,
    )
    return response['choices'][0]['message']['content']

def read_corpus():
    if not os.path.exists('data/corpus.json'):
        return {}
    with open('data/corpus.json', 'r') as f:
        return json.load(f)

def save_corpus(corpus):
    with open('data/corpus.json', 'w') as f:
        json.dump(corpus, f)

def read_region_names():
    region_names = []
    for file in glob.glob('data/region_names/*_region_names.txt'):
        with open(file, 'r') as f:
            region_names += f.read().split('\n')
    return region_names

corpus = read_corpus()
region_names = read_region_names()

no_names = []
for region_name in region_names:
    if region_name not in corpus:
        no_names.append(region_name)

region_names = no_names

i = 0
while i < len(region_names):
    region_name = region_names[i]
    format_request = json.dumps({"Name":"...","Synonym":["...",],"Description":"...","Relational corpus":{"Brain regions":"...","Diseases":"...","Cognition":"...","Emotions":"...","Behaviors":"...","Drugs":"..."}})
    prompt = f"""
    Your task is to give a detailed description associate with specified brain region about other brain regions, diseases, cognition, emotions, drugs, behaviors.
    The specified brain region name is as follows: '''{region_name}'''.
    Please anwser in this json format: {format_request}
    If you can't answer, you must only output empty json data.
    """

    try:
        response = call(prompt)
        corpus[region_name] = json.loads(response)
    except RateLimitError:
        time.sleep(10)
        continue
    except:
        save_corpus(corpus)
        raise

    i += 1
    print(f'count: {i}, region_name: {region_name}, response: {response}')

save_corpus(corpus)
