from os import path as pt
import time
import datetime
import argparse
import json
import subprocess
from collections import deque
import threading
from enum import Enum
import handler
import whisp
import reader
import chat
import read_json_with_globals
import filemanager
handled_paths = deque(maxlen=50)

DEBUG = False
LOGGING = False

import signal
import sys

def handle_signal(signum, frame):
    sys.exit(0)

signal.signal(signal.SIGTERM, handle_signal)
signal.signal(signal.SIGINT, handle_signal)

local_storage = set()
local_storage_times = set()
storage_timeout = 60 # after this time (seconds) passed, files will be deleted from local_storage
check_timeout = 5 # checks every n seconds for old files in local_storage

def get_file_wrapper(file:str, cache:bool=False):
    global local_storage

    for cached_file, cached_bytes in local_storage:
        log(f"cached: {cached_file}")
        if cached_file == file:
            log("found file in cache!")
            return cached_bytes
    log("get files from trigger wrapper")
    file_bytes = filemanager.get_file(file)

    if cache:
        local_storage.add((file, file_bytes))
        local_storage_times.add((file, datetime.datetime.now()))

    return file_bytes

def delete_from_cache(file: str):
    global local_storage, local_storage_times
    log(f"delete {file} from cache")
    local_storage = {item for item in local_storage if item[0] != file}
    local_storage_times = {item for item in local_storage_times if item[0] != file}

def delete_old_from_cache():
    for cached_file, cached_time in local_storage_times:
        if (datetime.datetime.now() - cached_time).seconds >= storage_timeout:
            delete_from_cache(cached_file)
            

with open('logfile.txt', 'w') as file:
    if LOGGING:
        file.write('')

def log(msg:str):
    if LOGGING:
        with open('logfile.txt', 'a') as file:
            file.write(msg)
            file.write('\n')
log('trigger started')


parser = argparse.ArgumentParser(description='Manages other modules and writes log file (session.json)')
parser.add_argument('commands', metavar='JSON', type=str, help='json file that contains the commands to be executed')
parser.add_argument('globals_json', metavar='JSON', type=str, help='json file with global variables to replace in command file')
parser.add_argument('--working_dir', metavar='DIR',default='.', type=str, help='working directory to listen files on, default: script path')
parser.add_argument('--timeout', metavar='N', default=20, type=int, help='timeout to override concurrency management, default: 20')
parser.add_argument('--speaker', metavar='Bool', default='True', type=str, help='define if the speaker should be used (True) or not used (False), default: True')
args = parser.parse_args()
log('trigger continued 1')

SPEAKER_ON = True
if (args.speaker).lower() in ['false', '0', 'no', 'off']:
    SPEAKER_ON = False
log(f'SPEAKER: {SPEAKER_ON}, because it was {args.speaker}')

listen_dir = args.working_dir
if args.working_dir == '.':
    listen_dir = pt.split(__file__)[0]
class Modul(Enum):
    transcriber = 1
    sender = 2
    reader = 3
    actuator = 4
    unknown = 5
    
FUNCTIONAL = True
transcriber_data = None
sender_data = None
reader_data = None
actuator_data = None

TIMEOUT = args.timeout
listener_command = "echo hello"
transcriber_command = "echo hello"
transcriber_trigger = ""
sender_command = "echo hello"
sender_trigger = ""
reader_command = "echo hello"
reader_trigger = ""
actuator_command = "echo hello"
actuator_trigger = ""

process_list = deque()
process_waiting = deque()
leader_process = -1
leader_process_time = datetime.datetime.now()

log("initialized vars")


try:
    jfile = read_json_with_globals.read_with_replaced_globals(args.commands, args.globals_json)
    listener_command = jfile["listener"]["command"].split()
    transcriber_command = jfile["transcriber"]["command"].split()
    transcriber_trigger = jfile["transcriber"]["trigger_folder"]
    sender_command = jfile["sender"]["command"].split()
    sender_trigger = jfile["sender"]["trigger_folder"]
    reader_command = jfile["reader"]["command"].split()
    reader_trigger = jfile["reader"]["trigger_folder"]
    actuator_command = jfile["actuator"]["command"].split()
    actuator_trigger = jfile["actuator"]["trigger_folder"]
except Exception as e:
    log(f"Error occured: {str(e)}")
    exit()

def start_command(command: list, process_id: int, is_mutexed:bool = False, modul:Modul = Modul.unknown, is_delayed:bool = False):
    global reader_data
    log("\nSTART: ")
    log(f'{datetime.datetime.now()} : {command[0]} {command[1]} {command[2]} ...')
    log('------------------------------------------------------------')
    if is_mutexed:
        if len(process_list) == 0:
            process_list.append(process_id)
        elif process_list[0] != process_id:
            # do not append if already delayed
            if not is_delayed:
                process_waiting.append([command, process_id, is_mutexed, modul, True])
            return ## should schedule new call and not rejecting completely the process
    if is_delayed:
        process_waiting.remove([command, process_id, is_mutexed, modul, True])
    
    if modul == Modul.reader:
        try:
            if SPEAKER_ON:    
                log("READER1")
                if FUNCTIONAL:
                    log("READER2")
                    if reader_data == None:
                        log('INIT READER')
                        reader_data = reader.init(command)
                    log('RUN READER')
                    reader.run(command, reader_data)
                    log('READER EXECUTED')
                else:
                    subprocess.run(command, cwd=listen_dir)
        except Exception as e:
            log(str(e))
            pass
    else:
        try:
            if FUNCTIONAL:
                call_function(modul, command)
            else:
                subprocess.Popen(command, cwd=listen_dir)
        except:
            pass

def call_function(modul:Modul, command:list):
    global transcriber_data, sender_data, reader_data, actuator_data
    if modul == Modul.unknown:
        return
    match modul:
        case Modul.transcriber:
            if transcriber_data == None:
                log('INIT TRANSCRIBER')
                transcriber_data = whisp.init(command)
            log('RUN TRANSCRIBER')
            t = threading.Thread(target=whisp.run, args=(command, transcriber_data))
            t.daemon = True
            t.start()

        case Modul.sender:
            if sender_data == None:
                log('INIT SENDER')
                sender_data = chat.init(command)
            log('RUN SENDER')
            t = threading.Thread(target=chat.run, args=(command, sender_data))
            t.daemon = True
            t.start()
            # t.join()
        case Modul.actuator:
            if actuator_data == None:
                log('INIT ACTUATOR')
                actuator_data = handler.init(command)
            t = threading.Thread(target=handler.run, args=(command, actuator_data))
            log('RUN ACTUATOR')
            t.daemon = True
            t.start()
        case Modul.reader:
            if reader_data == None:
                reader_data = reader.init(command)
            t = threading.Thread(target=reader.run, args=(command, reader_data))
            t.daemon = True
            t.start()

def fill_command(command:list, file:str, auto_split:bool = True):
    new_comm = []
    for i in command:
        if '{' in i and '}' in i and 'GLOBAL:' not in i:
            before = ''
            after = ''
            inner_path = ''
            for start in range(len(i)):
                if i[start] == '{':
                    before = i[0:start]
                    for end in range(start+1, len(i)):
                        if i[end] == '}':
                            inner_path = i[start+1: end]
                            after = i[end+1:]
                            break
                    break
            if inner_path != '':
                inner_list = inner_path.split(':')
                jsonized = ''
                if DEBUG:
                    with open(file, 'rb') as f:
                        jsonized = json.load(f)
                else:
                    f = get_file_wrapper(file, True)
                    log(f'len of bytes in json: {len(f)}')
                    if f == None:
                        log(f"got None type object when reading {file}")
                        return None
                    file_str = f.decode('utf-8')
                    jsonized = json.loads(file_str)
                    filemanager.delete_file(file)

                value = jsonized
                for i in inner_list:
                    value = value[i]
                if type(value) == dict:
                    flattened = ''
                    for x in value:
                        flattened += f'{value[x]} '
                    if auto_split:
                        new_comm += (before + flattened + after).split()
                    else:
                        new_comm.append(f'{before}{value}{after}')
                else:
                    if auto_split:
                        new_comm += (before + value + after).split()
                    else:
                        new_comm.append(f'{before}{value}{after}')
            else:
                if auto_split:
                    new_comm += (before + file + after).split()
                else:
                   new_comm.append(f'{before}{value}{after}') 
        else:
            new_comm.append(i)
    log(f'filled command: {new_comm}')
    return new_comm

def triggering(file:str, trigger_info: str):
    # file = pt.normpath(file)
    # trigger_info = pt.normpath(trigger_info)
    log(f'{file} {trigger_info}')
    file_path, file_name = pt.split(file)
    trigger_path, trigger_name = pt.split(trigger_info)
    file_basename, file_ext = pt.splitext(file_name)
    trigger_basename, trigger_ext = pt.splitext(trigger_name)
    if file_path != trigger_path:
        log('path not the same')
        if trigger_path.endswith(file_path):
            log('but seems to fit (consider this as a WARNING)')
        else:
            return -2
    if trigger_ext != file_ext:
        log('extension not the same')
        return -2
    reg_list = trigger_basename.split('{}')
    try:
        for i in reg_list:
            if i in file_basename:
                file_basename = file_basename.replace(i, '|')
            else:
                log('parser error in triggering')
                return -1
        number_strings = file_basename.split('|')
        id = -1
        for i in number_strings:
            if len(i) > 0:
                try:
                    id = int(i)
                    break
                except:
                    pass
        return id
    except Exception as e:
        log(str(e))
        return -2


def on_file_event(src_path:str):
    try:
        log(f'SOMETHING HAPPENED IN DIRECTORY: {src_path}')
        global leader_process, leader_process_time, process_list
        if len(process_list) > 0:
            if leader_process != process_list[0]:
                leader_process = process_list[0]
                leader_process_time = datetime.datetime.now()
            elif (datetime.datetime.now() - leader_process_time).seconds > TIMEOUT:
                process_list.popleft()
                if len(process_list) > 0:
                    leader_process = process_list[0]
                    leader_process_time = datetime.datetime.now()
        if src_path in handled_paths:
            log(f'Already handled file: {src_path}')
            for i in handled_paths:
                log(i)
            return
        else:
            handled_paths.append(src_path)


        file = pt.basename(src_path)

        file_name, file_ext = pt.splitext(file)

        trigs = triggering(src_path, transcriber_trigger)
        if(trigs >= 0):
            process_list.append(trigs)
            comm = fill_command(transcriber_command, src_path)
            if comm == None:
                return
            start_command(comm, process_id=trigs, modul=Modul.transcriber)

        trigs = triggering(src_path, sender_trigger)
        if(trigs >= 0):
            comm = fill_command(sender_command, src_path)
            if comm == None:
                return
            start_command(comm, process_id=trigs, is_mutexed=True, modul=Modul.sender)
            
        trigs = triggering(src_path, reader_trigger)
        if(trigs >= 0):
            if len(process_list) > 0:
                if trigs in process_list:
                    process_list.remove(trigs)
                    log('removed process_id from queue')
            log('start checking for previous processes')
            for process in process_waiting:
                start_command(process[0], process[1], process[2], process[3], process[4])
            log("filling command for reader")
            comm = fill_command(reader_command, src_path, auto_split=False)
            if comm == None:
                return
            start_command(comm, process_id=trigs, modul=Modul.reader)
            #delete 
            
        trigs = triggering(src_path, actuator_trigger)
        if(trigs >= 0):
            log("filling command for actuator")
            comm = fill_command(actuator_command, src_path)
            if comm == None:
                return
            start_command(comm, process_id=trigs, modul=Modul.actuator)
    except Exception as e:
        log(str(e))


if DEBUG:
    from watchdog.observers import Observer
    from watchdog.events import LoggingEventHandler
    
    class Event(LoggingEventHandler):
        handled_files = deque(maxlen=50)
        def on_any_event(self, event):
            on_file_event(event.src_path)

        def on_deleted(self, event):
            pass

        def on_modified(self, event):
            pass
        
        def on_created(self, event):
            pass
    
    event_handler = Event()
    observer = Observer()
    observer.schedule(event_handler, listen_dir, recursive=True)
    observer.start()
    log(f'DEBUG mode: trigger started listening in directory: {listen_dir}')
    try:
        while True:
            time.sleep(0.25)
    finally:
        observer.stop()
        observer.join()
else:
    try:
        starting_list = filemanager.get_files()
        if len(starting_list) > 0:
            for file in starting_list:
                filemanager.delete_file(file)
        list_snapshot = []
        check_time = datetime.datetime.now()
        while True:
            file_list = filemanager.get_files()
            if len(file_list) > 0 and file_list != list_snapshot:
                for file in file_list:
                    log(f'file found: {file}')
                    on_file_event(src_path=file)
                list_snapshot = file_list
            if (datetime.datetime.now() - check_time).seconds > check_timeout:
                delete_old_from_cache()
                check_time = datetime.datetime.now()
            time.sleep(0.2)
    except Exception as e:
        log(str(e))