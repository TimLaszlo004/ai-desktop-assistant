from openai import OpenAI
import argparse
from os import path as pt
import filemanager
import io

DEBUG = False

def log(msg:str):
    with open('logfile.txt', 'a') as file:
        file.write(msg)
        file.write('\n')

def init(arguments:list):
    for i in range(len(arguments)):
        if pt.splitext(pt.split(__file__)[1])[0] in arguments[i]:
            arguments = arguments[i+1:]
            break
    parser = get_argument_parser()
    argums = parser.parse_args(arguments)
    client = OpenAI() if (argums.api_key == '') else OpenAI(api_key=argums.api_key)
    return {"client":client, "file_name":argums.file_name, "target_dir":argums.target_dir}

def actualize(arguments:list, data:dict):
    for i in range(len(arguments)):
        if pt.split(__file__)[1] in arguments[i]:
            arguments = arguments[i+1:]
            break
    parser = get_argument_parser()
    argums = parser.parse_args(arguments)
    return {"client":data["client"], "file_name":argums.file_name, "target_dir":argums.target_dir}

def run(arguments:list, variables:dict):
    variables = actualize(arguments, variables)
    if "client" not in variables or "file_name" not in variables or "target_dir" not in variables:
        return
    audio_file = None
    if DEBUG:
        audio_file= open(variables["file_name"], "rb")
    else:
        log(f'transcriber: getfile: ({variables["file_name"]})')
        audio_file= io.BytesIO(filemanager.get_file(variables["file_name"]))
        audio_file.name = "dummy.wav" # openai checks file extension, so a name is required
    if audio_file == None: # do not send lost audio file
        log(f'transcriber: file not found! ({variables["file_name"]})')
        return
    transcription = None
    try:
        transcription = variables["client"].audio.transcriptions.create(
            model="whisper-1", 
            file=audio_file
        )
    except Exception as e:
        log(f"transcriber error: {str(e)}")
        return
    filemanager.delete_file(variables["file_name"])

    base = pt.splitext(pt.split(variables["file_name"])[1])[0]
    if DEBUG:
        with open(f'{pt.join(variables["target_dir"], base)}.txt', 'w', encoding='utf-8') as f:
            f.write(transcription.text)
    else:
        filemanager.store_file(filename=f'{pt.join(variables["target_dir"], base)}.txt', file=transcription.text.encode('utf-8'))
        log(f"transcriber saved text: {transcription.text}")
    
def get_argument_parser():
    parser = argparse.ArgumentParser(description='Generates text from given audio file')
    parser.add_argument('file_name', metavar='FILE_NAME', type=str, help='the file to be transcripted')
    parser.add_argument('target_dir', metavar='FILE_PATH', default='', type=str, help='target directory to save file')
    parser.add_argument('--api_key', metavar='API_KEY', default='', type=str, help='api key for OpenAI')
    return parser

def get_finalized_arg_list(args_str:str):
    arg_list = args_str.split()
    final_arg_list = []
    is_arg = False
    for i in arg_list:
        if is_arg:
            final_arg_list.append(i)
        elif i == pt.basename(__file__):
            is_arg = True
    return final_arg_list


if __name__ == '__main__':
    parser = get_argument_parser()
    args = parser.parse_args()
    client = OpenAI(api_key=args.api_key) if args.api_key != '' else OpenAI()
    audio_file = None
    if DEBUG:
        audio_file= open(args.file_name, "rb")
    else:
        audio_file= io.BytesIO(filemanager.get_file(args.file_name))
        audio_file.name = "dummy.wav" # openai checks file extension, so a name is required
    
    if audio_file == None:
        # filemanager.delete_file(args.file_name)
        exit()
    transcription = client.audio.transcriptions.create(
        model="whisper-1",
        file=audio_file
    )
    filemanager.delete_file(args.file_name)
    base = pt.splitext(pt.split(args.file_name)[1])[0]
    if DEBUG:
        with open(f'{pt.join(args.target_dir, base)}.txt', 'w', encoding='utf-8') as f:
            f.write(transcription.text)
    else:
        filemanager.store_file(filename=f'{pt.join(args.target_dir, base)}.txt', file=transcription.text.encode('utf-8'))