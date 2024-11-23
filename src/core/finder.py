import os
import argparse
import json
import re
import random
import subprocess

INTERESTING_DIR_COUNT = 2
INTERESTING_ROOT_LEN = 3
INDEX_FILE = 'index.json'

def is_matching(name_a:str, name_b:str, acceptance:float = 10) -> str:
    if acceptance == 0:
        return name_a == name_b
    
    if name_a == name_b:
        return True

    if name_a == "" or name_b == "":
        return False
    
    points = 0
    len_a = len(name_a)
    len_b = len(name_b)
    
    # punish for additional letters
    points += pow((abs(len_a - len_b)), 2) / min(len_a, len_b) * 20
    
    if points > acceptance:
        return False
    
    diff = abs(len_a - len_b) + 2
    shorter = name_a
    longer = name_b
    if len_a > len_b:
        shorter = name_b
        longer = name_a
    plus_letters = 0
    
    punishment_for_wrong_capitals = 10 / len(shorter)
    punishment_for_shifting = 60 / (pow(len(shorter), 2) / 2)
    punishment_for_typo = 40 / len(shorter)
    punishment_for_typo_multiplier = 2
    
    shift_direction = 0
    
    # punish for wrong letters
    for i in range(len(shorter)):
        if i + plus_letters >= len(longer):
            if plus_letters > 0:
                plus_letters -= 1
            else:
                break

        if shorter[i] != longer[plus_letters + i]:
            punished = False
            if shorter[i].lower() == longer[plus_letters + i].lower():
                points += punishment_for_wrong_capitals
                punished = True
                continue
            for p in range(diff - plus_letters):
                if i+plus_letters+p >= len(longer) or i+plus_letters+p < 0:
                    break
                if longer[i+plus_letters+p] == shorter[i]:
                    if shift_direction >= 0:
                        points += punishment_for_shifting
                    shift_direction = 1
                    plus_letters = p
                    punished = True
                    break
                if longer[i+plus_letters+p].lower() == shorter[i].lower():
                    if shift_direction >= 0:
                        points += punishment_for_shifting
                    shift_direction = 1    
                    points += punishment_for_wrong_capitals
                    plus_letters = p
                    punished = True
                    
            if punished:
                continue
            for p in range(plus_letters+3):
                if p == 0:
                    continue
                if i+plus_letters-p >= len(longer) or i+plus_letters-p < 0:
                    break
                if longer[i+plus_letters-p] == shorter[i]:
                    if shift_direction <= 0:
                        points += punishment_for_shifting
                    shift_direction = -1
                    plus_letters -= p
                    punished = True
                    break
                if longer[i+plus_letters-p].lower() == shorter[i].lower():
                    if shift_direction <= 0:
                        points += punishment_for_shifting
                    shift_direction = -1
                    points += punishment_for_wrong_capitals
                    plus_letters -= p
                    punished = True
                    break
            if not punished:
                points += punishment_for_typo
                punishment_for_typo *= punishment_for_typo_multiplier
        
        if points > acceptance:
            return False
    return True
    

def file_finding_with_preprocess(filename, directory, acceptance, index):
    found_file = ""
    found_dir = ""
    if index != None:
        # check for directory matches
        if directory != "":
            for path in index["dirs"]:
                dirs = re.split(r"[\\/]", path)
                for dir in dirs:
                    if is_matching(dir, directory, acceptance=30):
                        found_dir, found_file = find_file(filename=filename, search_path=path, acceptance=acceptance)
                        if found_file != "":
                            return found_dir, found_file
        
        # check in recent directories
        for path in index["recent"]:
            found_dir, found_file = find_file(filename=filename, search_path=path, acceptance=acceptance)
            if found_file != "":
                return found_dir, found_file
        
        # check in interesting dirs
        for path in index["dirs"]:
            found_dir, found_file = find_file(filename=filename, search_path=path, acceptance=acceptance)
            if found_file != "":
                return found_dir, found_file
                        
        
    # check for everything
    return find_file(filename=filename, acceptance=acceptance)

def get_interesting_dirs(search_path='\\Users\\'):
    interesting_paths = []
    for root, dirs, files in os.walk(search_path):
        splits = re.split(r"[\\/]", root)
        if len(dirs) > INTERESTING_DIR_COUNT and len(splits) > INTERESTING_ROOT_LEN and "AppData" not in root and 'build' not in root and 'Lib' not in root and 'lib' not in root and os.path.dirname(root) not in interesting_paths:
            hidden = False
            for part in splits:
                if len(part):
                    if part[0] == '.' or part.count('.') > 1:
                        hidden = True
                        break
            if not hidden:
                interesting_paths.append(root)
    sorted_strings = sorted(interesting_paths, key=len, reverse=True)
    return sorted_strings

def find_file(filename, search_path='\\', acceptance = 10):
    for root, dirs, files in os.walk(search_path):
        for f in files:
            if is_matching(filename, f, acceptance):
                return root, os.path.join(root, f)

    return "", ""
    
    
parser = argparse.ArgumentParser(description='This is a file searcher script: finds the given file in the file system')
parser.add_argument('filename', metavar='FILE_NAME', type=str, help='the name of the file to be found')
parser.add_argument('--directory', metavar='DIRECTORY_NAME', default='', type=str, help='a directory name which is present in the filepath of the file')
parser.add_argument('--acceptance', metavar='N', default=0, type=int, help='percentage of the acceptance: 0: exact match - 100+: accept almost anything as a match, default: 0')
parser.add_argument('--update', action='store_true', help='after finding the file it updates the index file (advised to use time to time)')
parser.add_argument('--explorer', action='store_true', help='after finding the file, the directory will be opened in the file explorer')
args = parser.parse_args()

# load or init index file
index = None
try:
    with open('index.json', 'rb') as file:
        index = json.load(file)
except:
    pass

# find file
path, found_file = file_finding_with_preprocess(args.filename, args.directory, args.acceptance, index)
print(f'Found file: {found_file} in {path}')

if args.explorer:
    f'explorer "{path}"'
    sp = subprocess.Popen(["explorer", path],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.STDOUT)
else:
    print(f'explorer "{found_file}"')
    sp = subprocess.Popen(["explorer", found_file],
         cwd=os.path.dirname(os.path.realpath(__file__)),
        stdout=subprocess.DEVNULL,
        stderr=subprocess.STDOUT)

# optional: update index file
if index == None:
    with open(INDEX_FILE, 'w') as file:
        recent = []
        recent.append(path)
        data = {"recent": recent, "dirs": get_interesting_dirs()}
        index = json.dump(data, file)
    exit()

in_recent = False

recent = index["recent"]
dirs = index["dirs"]

for current in recent:
    if current == path:
        in_recent = True
        break

if not in_recent:
    if len(recent) < 200:
        recent.append(path)
    else:
        recent[random.randint(0, len(recent)-1)] = path

if args.update:
    dirs = get_interesting_dirs()

with open(INDEX_FILE, 'w') as file:
    data = {"recent": recent, "dirs": dirs}
    index = json.dump(data, file)