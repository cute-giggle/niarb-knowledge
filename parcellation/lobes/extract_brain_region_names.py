import json


def extract_brain_region_names(annot: dict):
    brain_region_names = []
    for item in annot['color_table']:
        brain_region_names.append(item[1])
    return brain_region_names


def main():
    annot_path = 'surface/lobes.annot.json'
    annot = json.load(open(annot_path, 'r'))
    brain_region_names = extract_brain_region_names(annot)
    brain_region_names.remove('???')
    save_path = 'lobes_region_names.txt'
    with open(save_path, 'w') as f:
        f.write('\n'.join(brain_region_names))


if __name__ == '__main__':
    main()
