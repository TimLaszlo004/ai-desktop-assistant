import argparse
import os
import signal
from win11toast import toast
import subprocess

LOGGING = False

with open('killerlog.txt', 'w') as file:
    if LOGGING:
        file.write('')

def log(msg:str):
    if LOGGING:
        with open('killerlog.txt', 'a') as file:
            file.write(msg)
            file.write('\n')

def kill_process(pid):
    try:
        subprocess.Popen(f"taskkill /f /t /pid {pid}".split(),
            stdout=subprocess.DEVNULL,
            stderr=subprocess.STDOUT)
        log(f"Process with PID {pid} has been terminated.")
    except ProcessLookupError:
        log(f"No process found with PID {pid}.")
    except PermissionError:
        log(f"Permission denied: Unable to terminate PID {pid}. Run as administrator.")
    except Exception as e:
        log(f"An error occurred while trying to terminate PID {pid}: {e}")

def main():
    parser = argparse.ArgumentParser(description="Kill a process on Windows by its PID using os.kill.")
    parser.add_argument(
        "pid", type=int, help="The PID of the process to kill."
    )
    args = parser.parse_args()
    kill_process(args.pid)

if __name__ == "__main__":
    main()