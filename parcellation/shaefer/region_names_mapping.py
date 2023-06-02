import json


def region_names_mapping():
    mapping = {
        "7Networks": "Shaefer.400.7",

        "LH": "left hemisphere",
        "RH": "right hemisphere",

        "Vis": "visual network",
        "SomMot": "somatomotor network",
        "DorsAttn": "dorsal attention network",
        "SalVentAttn": "ventral attention network",
        "Limbic": "limbic network and cognitive network",
        "Cont": "frontoparietal network",
        "Default": "default mode network",

        "AntTemp": "anterior temporal",
        "Aud": "auditory",
        "Cent": "central",
        "Cing": "cingulate",
        "Cinga": "cingulate anterior",
        "Cingm": "mid-cingulate",
        "Cingp": "cingulate posterior",
        "ExStr": "extrastriate cortex",
        "ExStrInf": "extra-striate inferior",
        "ExStrSup": "extra-striate superior",
        "FEF": "frontal eye fields",
        "FPole": "frontal pole",
        "FrMed": "frontal medial",
        "FrOper": "frontal operculum",
        "FrOperIns": "frontal operculum insula",
        "IFG": "inferior frontal gyrus",
        "Ins": "insula",
        "IPL": "inferior parietal lobule",
        "IPS": "intraparietal sulcus",
        "Med": "medial",
        "OFC": "orbital frontal cortex",
        "Par": "parietal",
        "ParMed": "parietal medial",
        "ParOcc": "parietal occipital",
        "ParOper": "parietal operculum",
        "pCun": "precuneus",
        "pCunPCC": "precuneus posterior cingulate cortex",
        "PFC": "prefrontal cortex",
        "PFCd": "dorsal prefrontal cortex",
        "PFCl": "lateral prefrontal cortex",
        "PFCld": "lateral dorsal prefrontal cortex",
        "PFClv": "lateral ventral prefrontal cortex",
        "PFCm": "medial prefrontal cortex",
        "PFCdPFCm": "prefrontal cortex dorsal medial",
        "PFCmp": "medial posterior prefrontal cortex",
        "PFCv": "ventral prefrontal cortex",
        "PHC": "parahippocampal cortex",
        "Post": "post central",
        "PostC": "post central",
        "PrC": "precentral",
        "PrCd": "precentral dorsal",
        "PrCv": "precentral ventral",
        "RSC": "retrosplenial cortex",
        "Rsp": "retrosplenial",
        "S2": "S2",
        "SPL": "superior parietal lobule",
        "ST": "superior temporal",
        "Striate": "striate cortex",
        "StriCal": "striate calcarine",
        "Temp": "temporal",
        "TempOcc": "temporal occipital",
        "TempPar": "temporal parietal",
        "TempOccPar": "temporal occipital parietal",
        "TempPole": "temporal pole",
    }

    with open('shaefer-400-7_region_names.txt', 'r') as f:
        region_names = f.readlines()
        region_names = [region_name.strip() for region_name in region_names]

    result = {}
    for region_name in region_names:
        items = region_name.split('_')
        for i in range(len(items)):
            if items[i] in mapping:
                items[i] = mapping[items[i]]
        result[region_name] = ' '.join(items)
    
    with open('shaefer-400-7_region_names_mapping.json', 'w') as f:
        json.dump(result, f, indent=4)


def main():
    region_names_mapping()


if __name__ == '__main__':
    main()
