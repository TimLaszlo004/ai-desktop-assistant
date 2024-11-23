import requests

def log(msg:str):
    with open('logfile.txt', 'a') as file:
        file.write(msg)
        file.write('\n')


BASE_URL = 'http://127.0.0.1:5000'

def store_file(filename, file):
    try:
        normalized_filename = filename.replace('\\\\', '\\')
        
        # trigger.log(f'stored: {normalized_filename}')
        # toast('Assistant', f'Stored: {normalized_filename}')
        files = {'file': (normalized_filename, file), 'filename': (None, normalized_filename)}
        response = requests.post(f'{BASE_URL}/storefile', files=files)
        log(f'FILEMANAGER STORED: {normalized_filename}')

    except:
        pass

def get_files():
    try:
        response = requests.get(f'{BASE_URL}/getfiles')
        
        if response.status_code == 200:
            files = response.json().get("files", [])
            if files:
                return files
            else:
                print("No files")
                return []
        else:
            print("Not OK")
            return []
    except Exception as e:
        print(f"Error {e}")
        return []

def get_file(filename):
    try:
        response = requests.get(f'{BASE_URL}/getfile/{filename}')
        
        if response.status_code == 200:
            return response.content
        else:
            log(f'FILEMANAGER: status code is not OK: {response.status_code}')
            return None
    except Exception as e:
        log(f'FILEMANAGER ERROR: {str(e)}')
        return None

def delete_file(filename):
    try:
        response = requests.delete(f'{BASE_URL}/deletefile/{filename}')
        log(f'FILEMANAGER DELETING FILE: {filename}')
    except:
        pass
