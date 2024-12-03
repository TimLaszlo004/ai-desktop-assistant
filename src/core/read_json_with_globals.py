import json

def read_with_replaced_globals(file_name:str, globals_name:str):
    with open(file_name, 'r') as file:
        content = file.read()
        with open(globals_name, 'rb') as g_file:
            global_variables = json.load(g_file)
        for key in global_variables:
            content = content.replace('{' + f'GLOBAL:{key}' + '}', f'{global_variables[key]}')
            print(content)
    return json.loads(content)
