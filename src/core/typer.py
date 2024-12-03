from pynput.keyboard import Key, Controller
import sys
import time

command_string = sys.argv[-1]

def string_to_key(string:str):
    match string:
        case 'alt':
            return Key.alt
        case 'alt_gr':
            return Key.alt_gr
        case 'alt_l':
            return Key.alt_l
        case 'alt_r':
            return Key.alt_r
        case 'backspace':
            return Key.backspace
        case 'caps_lock':
            return Key.caps_lock
        case 'cmd':
            return Key.cmd
        case 'cmd_l':
            return Key.cmd_l
        case 'cmd_r':
            return Key.cmd_r
        case 'ctrl':
            return Key.ctrl
        case 'ctrl_l':
            return Key.ctrl_l
        case 'ctrl_r':
            return Key.ctrl_r
        case 'delete':
            return Key.delete
        case 'down':
            return Key.down
        case 'end':
            return Key.end
        case 'enter':
            return Key.enter
        case 'esc':
            return Key.esc
        case 'f1':
            return Key.f1
        case 'f10':
            return Key.f10
        case 'f11':
            return Key.f11
        case 'f12':
            return Key.f12
        case 'f13':
            return Key.f13
        case 'f14':
            return Key.f14
        case 'f15':
            return Key.f15
        case 'f16':
            return Key.f16
        case 'f17':
            return Key.f17
        case 'f18':
            return Key.f18
        case 'f19':
            return Key.f19
        case 'f20':
            return Key.f20
        case 'f21':
            return Key.f21
        case 'f22':
            return Key.f22
        case 'f23':
            return Key.f23
        case 'f24':
            return Key.f24
        case 'f2':
            return Key.f2
        case 'f3':
            return Key.f3
        case 'f4':
            return Key.f4
        case 'f5':
            return Key.f5
        case 'f6':
            return Key.f6
        case 'f7':
            return Key.f7
        case 'f8':
            return Key.f8
        case 'f9':
            return Key.f9
        case 'home':
            return Key.home
        case 'insert':
            return Key.insert
        case 'left':
            return Key.left
        case 'media_next':
            return Key.media_next
        case 'media_play_pause':
            return Key.media_play_pause
        case 'media_previous':
            return Key.media_previous
        case 'media_volume_down':
            return Key.media_volume_down
        case 'media_volume_up':
            return Key.media_volume_up
        case 'media_volume_mute':
            return Key.media_volume_mute
        case 'menu':
            return Key.menu
        case 'num_lock':
            return Key.num_lock
        case 'page_down':
            return Key.page_down
        case 'page_up':
            return Key.page_up
        case 'pause':
            return Key.pause
        case 'print_screen':
            return Key.print_screen
        case 'right':
            return Key.right
        case 'scroll_lock':
            return Key.scroll_lock
        case 'shift':
            return Key.shift
        case 'shift_l':
            return Key.shift_l
        case 'shift_r':
            return Key.shift_r
        case 'space':
            return Key.space
        case 'tab':
            return Key.tab
        case 'up':
            return Key.up
        case _:
            return None

command_list_tmp = command_string.split('}')
command_list = []

def is_string_command(string:str):
    if string[:5] == 'PRESS' or string[:7] == 'RELEASE' or string[:4] == 'TYPE' or string[:5] == 'SLEEP':
        return True
    else:
        return False

sub_command = ''
for i in range(len(command_list_tmp)-1):
    sub_command += command_list_tmp[i]
    if i == len(command_list_tmp)-2:
        command_list.append(sub_command)
    elif is_string_command(command_list_tmp[i+1]):
        command_list.append(sub_command)
        sub_command = ''
    else:
        sub_command += '}'

keyboard = Controller()

for command in command_list:
    try:
        if command[:5] == 'PRESS':
            key = string_to_key(command[6:])
            if key != None:
                keyboard.press(key)
            else:
                print(f'No {command[6:]} key found')
        elif command[:7] == 'RELEASE':
            key = string_to_key(command[8:])
            if key != None:
                keyboard.release(key)
            else:
                print(f'No {command[8:]} key found')
        elif command[:4] == 'TYPE':
            keyboard.type(command[5:])
        elif command[:5] == 'SLEEP':
            time.sleep(float(command[6:]))
    except Exception as e:
        print(e)