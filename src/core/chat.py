from openai import OpenAI
import json
from os import path as pt
import argparse
import filemanager
import io

DEBUG = False
LOGGING = False

def log(msg:str):
    if LOGGING:
        with open('logfile.txt', 'a') as file:
            file.write(msg)
            file.write('\n')

def init(arguments:list):
    try:
        for i in range(len(arguments)):
            if pt.splitext(pt.split(__file__)[1])[0] in arguments[i]:
                arguments = arguments[i+1:]
                break
        parser = get_argument_parser()
        log(str(arguments))
        args = parser.parse_args(arguments)
        history_file = get_output_file_name(args.output_dir)
        client = OpenAI() if args.api_key == '' else OpenAI(api_key=args.api_key)
        function_map, original_data = init_function_list(args.function_map) # also reinitialize if in dynamic mode
        return {"history_file":history_file, "client":client, "function_map":function_map, "original_data":original_data}
    except Exception as e:
        log(f"SENDER INIT ERROR: {str(e)}")
        return None


def run(arguments:list, variables:dict):
    try:
        for i in range(len(arguments)):
            if pt.split(__file__)[1] in arguments[i]:
                arguments = arguments[i+1:]
                break
        log("CALLED chat.run()")
        dynamic = True
        if "client" not in variables or "history_file" not in variables or "function_map" not in variables or "original_data" not in variables:
            log("SENDER NOT INITIALIZED!")
            return
        parser = get_argument_parser()
        args = parser.parse_args(arguments)
        text = read_text(args.text_file)
        log(f"sender received: {text}")
        if text == '':
            return # do not send empty message
        history_file = variables["history_file"]
        history = load_history(history_file)
        client = variables["client"]
        log("SENDER running")
        function_map = original_data = None
        if dynamic:
            function_map, original_data = init_function_list(args.function_map)
        else:
            function_map = variables["function_map"]
            original_data = variables["original_data"]
        history.append({"role": "user", "content": text})
        log("SENDER send message")
        chat_response = chat_completion_request(
            client=client, messages=history, tools=function_map, max_tks=args.max_tokens
        )
        small_json = format_output(history, chat_response, original_data, args.function_answer)
        log("SENDER save files")
        update_history_file(pt.join(args.output_dir, 'output.json'), history)
        save_small_json(small_json, args.text_file, args.output_dir)
        log("SENDER saved files")
    except Exception as e:
        log(f"SENDER RUN ERROR: {str(e)}")


def get_argument_parser():
    parser = argparse.ArgumentParser(description='Sends the given text to OpenAI ChatGPT (gpt-4o) and answers')
    parser.add_argument('text_file', metavar='TEXT_FILE', type=str, help='the text file that should be sent')
    parser.add_argument('output_dir', metavar='FILE_PATH',type=str, help='output directory for response and history')
    parser.add_argument('--api_key', metavar='API_KEY', default='', type=str, help='api key for OpenAI')
    parser.add_argument('--max_tokens', metavar='N', default=1200, type=int, help='maximum token count for conversation, low tokens mean faster and shorter answers, default: 1200')
    parser.add_argument('--function_map', default='', metavar='FILE_PATH', type=str, help='json file with callable functions')
    parser.add_argument('--function_answer', default='Function called', metavar='STRING', type=str, help='Function calling response, default: Function called')
    return parser


def read_text(file_name:str):
    if DEBUG:
        txt = ''
        with open(file_name, 'r', encoding='utf-8') as file:
            temp = file.readlines()
            for i in temp:
                txt += f'{i} '
        return txt
    else:
        file_bytes = filemanager.get_file(file_name)
        if file_bytes != None:
            filemanager.delete_file(file_name)
            return file_bytes.decode('utf-8')
        else:
            # filemanager.delete_file(file_name)
            return ''


def update_history_file(output_file:str, data):
    with open(output_file, 'w', encoding='utf-8') as file:
        json.dump(data, file, ensure_ascii=False, indent=4)


def get_output_file_name(output_dir:str):
    return pt.join(output_dir, 'output.json')


def load_history(output_file_name:str):
    messages = []
    try:
        with open(output_file_name, 'r', encoding='utf-8') as file:
            # global messages
            messages = json.load(file)
    except Exception as e:
        pass
    return messages


def init_function_list(function_map:str):
    with open(function_map, 'rb') as file:
        original_data = json.load(file)

    formatted_data = []

    for function_name, function_data in original_data.items():
        new_function = {
            "type": "function",
            "function": {
                "name": function_name,
                "description": function_data["description"],
                "parameters": {
                    "type": "object",
                    "properties": function_data["parameters"],
                    "required": function_data["required"]
                }
            }
        }
        formatted_data.append(new_function)
    return formatted_data, original_data


def chat_completion_request(client, messages, tools=None, tool_choice=None, model='gpt-4o-mini', max_tks:int=1024):
    try:
        response = client.chat.completions.create(
            model=model,
            messages=messages,
            tools=tools,
            tool_choice=tool_choice,
            max_tokens=max_tks
        )
        return response
    except Exception as e:
        log(e)
        return e


def format_output(history:list, chat_response, original_data, function_answer:str='Called succesfully'):
    small_json = dict()
    if(chat_response.choices[0].message.content):
        history.append(
            {"role": "assistant", "content": chat_response.choices[0].message.content},
        )
        assistant_response = {
                                "text": {
                                    "role": "assistant",
                                    "content": chat_response.choices[0].message.content
                                }
                            }
        small_json.update(assistant_response)
    else:
        history.append(
            {"role": "assistant", "content": function_answer},
        )
        assistant_response = {
                                "text": {
                                    "role": "assistant",
                                    "content": ""
                                }
                            }
        small_json.update(assistant_response)

    successful = False
    try:
        for func in chat_response.choices[0].message.tool_calls:
            params = json.loads(func.function.arguments)
            param_string = ''
            try:
                for param in original_data[func.function.name]["parameters"]:
                    if len(params[param].split()) > 1 and 'http' in original_data[func.function.name]["command"]:
                        params[param] = params[param].replace(' ', '+')

                    param_string += f'{params[param]} '
                history.append(
                    {"role": "function", "name": func.function.name, "arguments": params, "content": "Executed successfully"}
                )
                function_response = {
                                        "function": {
                                            "name": func.function.name,
                                            "arguments": params,
                                        }
                                    }
                small_json.update(function_response)
                successful = True
            except:
                pass
    except Exception as e:
        pass

    if not successful:
        function_response = {
                                "function": {
                                    "name": "",
                                    "arguments": "",
                                }
                            }
        small_json.update(function_response)
    return small_json


def save_small_json(small_json, text_file:str, output_dir:str):
    base = pt.splitext(pt.split(text_file)[1])[0]
    if DEBUG:
        with open(f'{pt.join(output_dir, base)}.json', 'w', encoding='utf-8') as file:
            json.dump(small_json, file, ensure_ascii=False, indent=4)
    else:
        try:
            json_bytes = json.dumps(small_json, ensure_ascii=False, indent=4).encode('utf-8')
            filemanager.store_file(filename=f'{pt.join(output_dir, base)}.json', file=json_bytes)
        except Exception as e:
            log(f"sender save error: {str(e)}")

if __name__ == '__main__':
    parser = get_argument_parser()
    args = parser.parse_args()
    text = read_text(args.text_file)
    if text == '': # do not send empty message
        exit()
    history_file = get_output_file_name(args.output_dir)
    history = load_history(history_file)
    client = OpenAI() if args.api_key == '' else OpenAI(api_key=args.api_key)
    function_map, original_data = init_function_list(args.function_map)
    history.append({"role": "user", "content": text})
    chat_response = chat_completion_request( 
        client=client, messages=history, tools=function_map, max_tks=args.max_tokens
    )
    small_json = format_output(history, chat_response, original_data, args.function_answer)
    update_history_file(pt.join(args.output_dir, 'output.json'), history)
    save_small_json(small_json, args.text_file, args.output_dir)