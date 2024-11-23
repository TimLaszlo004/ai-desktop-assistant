import pyaudio
from openai import OpenAI
import argparse
import os
import shlex

def get_finalized_arg_list(args_str:str):
    arg_list = shlex.split(args_str)
    final_arg_list = []
    is_arg = False
    for i in arg_list:
        if is_arg:
            final_arg_list.append(i)
        elif i == os.path.basename(__file__):
            is_arg = True
    return final_arg_list

def init(arguments:list):
    for i in range(len(arguments)):
        if os.path.splitext(os.path.split(__file__)[1])[0] in arguments[i]:
            arguments = arguments[i+1:]
            break
    parser = get_argument_parser()
    argums = parser.parse_args(arguments)
    client = OpenAI() if (argums.api_key == '') else OpenAI(api_key=argums.api_key)
    return {"client":client, "text":argums.text, "voice":argums.voice}

def actualize(arguments:str, data:dict):
    if "client" not in data:
        init()
    for i in range(len(arguments)):
        if os.path.split(__file__)[1] in arguments[i]:
            arguments = arguments[i+1:]
            break
    parser = get_argument_parser()
    argums = parser.parse_args(arguments)
    return {"client":data["client"], "text":argums.text, "voice":argums.voice}

def run(arguments:list, variables:dict):
    variables = actualize(arguments, variables)
    if "client" not in variables or "voice" not in variables or "text" not in variables:
        return
    if variables["text"] == "":
        return 
    p = pyaudio.PyAudio()
    stream = p.open(format=8,
                    channels=1,
                    rate=24_000,
                    output=True)

    with variables["client"].audio.speech.with_streaming_response.create(
            model='tts-1',
            voice=variables["voice"],
            input=variables["text"],
            response_format="pcm"
    ) as response:
        for chunk in response.iter_bytes(1024):
            stream.write(chunk)

def get_argument_parser():
    parser = argparse.ArgumentParser(description='Reads the given text using OpenAI text to speech model')
    parser.add_argument('text', metavar='TEXT', type=str, help='text to be read')
    parser.add_argument('--api_key', metavar='API_KEY', default='', type=str, help='api key for OpenAI')
    parser.add_argument('--voice', metavar='voice', default='echo', type=str, help='voice of the reader [alloy, echo, fable, onyx, nova, shimmer]')
    return parser

if __name__ == '__main__':
    parser = get_argument_parser()
    args = parser.parse_args()
    if args.text == "":
        exit()
    client = OpenAI() if (args.api_key == '') else OpenAI(api_key=args.api_key)

    p = pyaudio.PyAudio()
    stream = p.open(format=8,
                    channels=1,
                    rate=24_000,
                    output=True)

    with client.audio.speech.with_streaming_response.create(
            model='tts-1',
            voice=args.voice,
            input=args.text,
            response_format="pcm"
    ) as response:
        for chunk in response.iter_bytes(1024):
            stream.write(chunk)
    
    