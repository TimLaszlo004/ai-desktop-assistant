import webrtcvad
import pyaudio
from collections import deque
from os import path
import wave
import argparse
from pynput import keyboard
# import audioop
from pycaw.pycaw import AudioUtilities, ISimpleAudioVolume
import io
import filemanager

DEBUG = False

count = 0


def get_default_device_index(p):
    """Retrieve the current default input device index."""
    try:
        default_device = p.get_default_input_device_info()
        print(default_device)
        return default_device['index']
    except IOError:
        # If there's no default input device, return None
        print("No default input device found.")
        return None

# p = pyaudio.PyAudio()
# print(get_default_device_index(p))
# exit()

def detect_speech_from_microphone():
    global count
    # Initialize the VAD
    vad = webrtcvad.Vad()
    vad.set_mode(filter_mode)  # 0 is the least aggressive, 3 is the most aggressive

    # Initialize PyAudio
    p = pyaudio.PyAudio()

    # Define audio stream parameters
    sample_rate = 16000
    frame_duration = 30  # ms
    frame_size = int(sample_rate * frame_duration / 1000)

    stream = p.open(format=pyaudio.paInt16,
                    channels=1,
                    rate=sample_rate,
                    input=True,
                    frames_per_buffer=frame_size,
                    input_device_index=get_default_device_index(p)
                    )

    ring_buffer = deque(maxlen=30)  # to store a short history of frames
    triggered = False
    
    buffer_file = []

    wasKilled = False
    while True:
        if input_handling_mode == 1:
            if is_audio_playing():
                wasKilled = True
                continue
        elif input_handling_mode == 2:
            if not key_pressed:
                wasKilled = True
                continue
        if wasKilled:
            buffer_file = []
        wasKilled = False
        frame = stream.read(frame_size)
        is_speech = vad.is_speech(frame, sample_rate)
        ring_buffer.append((frame, is_speech))
        buffer_file.append(frame)
        num_voiced = len([f for f, speech in ring_buffer if speech])

        if not triggered:
            # print(f'{num_voiced} vs {0.5 * len(ring_buffer)}: is the first bigger?')
            if num_voiced > 0.5 * len(ring_buffer):
                triggered = True
                print('LISTENER::triggered')
                buffer_file = buffer_file[max(len(buffer_file)-50, 0):]
        else:
            if num_voiced < 0.1 * len(ring_buffer):
                print("it was speech")
                triggered = False

                new_file = path.join(target_directory,f'{file_name}{count}.wav')
                count += 1
                count = count % 100
                
                if DEBUG:
                    with wave.open(new_file, 'wb') as wf:
                        wf.setnchannels(1)
                        wf.setsampwidth(p.get_sample_size(pyaudio.paInt16))
                        wf.setframerate(sample_rate)
                        wf.writeframes(b''.join(buffer_file))
                
                else:
                    wav_data = io.BytesIO()
                    with wave.open(wav_data, 'wb') as wf:
                        wf.setnchannels(1)
                        wf.setsampwidth(p.get_sample_size(pyaudio.paInt16))
                        wf.setframerate(sample_rate)
                        wf.writeframes(b''.join(buffer_file))
                    filemanager.store_file(new_file, file=wav_data.getvalue())

                buffer_file.clear()
                # is_audio_playing()



# def is_audio_playing():
#     sessions = AudioUtilities.GetAllSessions()
#     for session in sessions:
#         if session.State == 1 and session.Process and 'py' not in session.Process.name():  # State == 1 means the audio session is active
#             print(session.Process.name())
#             return True
#     return False
def is_audio_playing():
    sessions = AudioUtilities.GetAllSessions()
    for session in sessions:
        volume = session._ctl.QueryInterface(ISimpleAudioVolume)
        # Check if the session is an output session and is active with a volume > 0
        if session.State == 1 and volume.GetMasterVolume() > 0:
            return True
    return False

parser = argparse.ArgumentParser(description='Creates wave files from the audio input filtered by webRTC voice activity detection algorithm')
parser.add_argument('--target_directory', metavar='FILE_PATH', default='', type=str, help='target folder for output audio, default: same folder as executable')
parser.add_argument('--file_name', metavar='FILE_NAME', default='temp', type=str, help='name of the output files (like: [FILE_PATH][count].wav -> temp0.wav when FILE_PATH = temp), default: temp')
parser.add_argument('--filter_scale', metavar='n', default=3, type=int, help='on a scale from 0-3 how aggressive filtering can be applied (3 -> hard filtering), default: 3')
parser.add_argument('--input_mute_level', metavar='n', default=0, type=int, help='0 - always listen, 1 - listen when no sound is played, 2 - listen when key pressed, default: 0')
parser.add_argument('--input_key', metavar='KEY', default='space', type=str, help='if input_mute_level is 2, this key will be the trigger, default: space')
args = parser.parse_args()

target_directory = args.target_directory
if target_directory == '':
    target_directory = path.abspath(path.dirname(__file__))
file_name  = args.file_name
filter_mode = args.filter_scale
input_handling_mode = args.input_mute_level
### TODO
if input_handling_mode == 1:
    input_handling_mode = 0
###
trigger_key = args.input_key

key_pressed = False

def stringify_key(key):
    strkey = str(key)
    strkey = strkey.replace('Key.', '')
    strkey = strkey.replace('\'', '')
    return strkey

def on_press(key):
    global key_pressed
    if stringify_key(key) == args.input_key:
        key_pressed = True

def on_release(key):
    global key_pressed
    if stringify_key(key) == args.input_key:
        key_pressed = False

listener = keyboard.Listener(
    on_press=on_press,
    on_release=on_release)

listener.start()

detect_speech_from_microphone()