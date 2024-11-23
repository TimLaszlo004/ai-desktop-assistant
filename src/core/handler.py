import json
import subprocess
import argparse
from on_destructive import on_destructive_toast
import os
## Remember: this is a one-shot system, if you want to execute multiple commands, you need multiple instances of this


def init(arguments:list):
    pass

def actualize(arguments:list, variables:dict):
    print()
    for i in range(len(arguments)):
        if os.path.splitext(os.path.split(__file__)[1])[0] in arguments[i]:
            arguments = arguments[i+1:]
            break
    print(arguments)
    parser = get_argument_parser()
    argums = parser.parse_args(arguments)
    syscomm = read_command_from_map(argums.command, argums.map, argums.parameters)
    filtered = get_filtered_function(syscomm, argums.parameters)
    run_dict = {"filtered_command": filtered}
    return run_dict

def read_command_from_map(func:str, map:str, parameters:list):
    # func = args.command
    comm = ''
    with open(map, 'rb') as file:
        jfile = json.load(file)
        comm = jfile[func]["command"]
        if jfile[func]["destructive"]:
            accepted = on_destructive_toast(f'{func} with parameters: {parameters}')
            if not accepted:
                exit()

    syscomm = comm.split()
    return syscomm

def get_filtered_function(unfiltered:list, parameters:list):
    for i in range(len(unfiltered)):
        if len(unfiltered[i]) >= 3:
            if unfiltered[i][0] == '{' and unfiltered[i][-1] == '}':
                try:
                    unfiltered[i] = parameters[int(unfiltered[i][1:-1])]
                    print(unfiltered)
                except:
                    unfiltered[i] = ''
            elif '{' in unfiltered[i] and '}' in unfiltered[i]:
                for start in range(len(unfiltered[i])):
                    if unfiltered[i][start] == '{':
                        for end in range(start, len(unfiltered[i])):
                            if unfiltered[i][end] == '}':
                                try:
                                    unfiltered[i] = unfiltered[i][0:start] + parameters[int(unfiltered[i][start+1: end])] + unfiltered[i][end+1:]
                                except Exception as e:
                                    pass
                                break

    filtered_command = [string for string in unfiltered if string.strip()]
    return filtered_command

def run(arguments:list, variables:dict):
    print(arguments)
    variables = actualize(arguments, variables)
    if "filtered_command" not in variables:
        return 
    sp = subprocess.Popen(variables["filtered_command"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.STDOUT)

def get_argument_parser():
    parser = argparse.ArgumentParser(description='Calls the command presented in the given json file according to the input variables')
    parser.add_argument('map', metavar='FILE_PATH', type=str, help='the json file which contains the commands')
    parser.add_argument('command', metavar='COMMAND', type=str, help='the command wanted to be called')
    parser.add_argument('--parameters', metavar='arg1.2.3', nargs='*', help='arguments to replace the parameters')
    return parser

if __name__ == '__main__':
    parser = get_argument_parser()
    args = parser.parse_args()
    syscomm = read_command_from_map(args.command, args.map, args.parameters)
    filtered = get_filtered_function(syscomm, args.parameters)
    sp = subprocess.Popen(filtered,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.STDOUT)
    