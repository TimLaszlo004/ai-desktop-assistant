from flask import Flask, request, jsonify, send_file
from io import BytesIO
import threading

import signal
import sys

def handle_signal(signum, frame):
    print(f"Received signal {signum}, cleaning up...")
    sys.exit(0)

signal.signal(signal.SIGTERM, handle_signal)
signal.signal(signal.SIGINT, handle_signal)

LOGGING = False

def log(msg:str):
    if LOGGING:
        with open('logfile.txt', 'a') as file:
            file.write(msg)
            file.write('\n')

app = Flask(__name__)

file_storage = {}

storage_lock = threading.Lock()

@app.route('/storefile', methods=['POST'])
def storefile():
    try:
        filename = request.form['filename']
        file = request.files['file']
        file_data = file.read()
        
        with storage_lock:
            file_storage[filename] = BytesIO(file_data)
        
        return jsonify({"message": "File stored successfully!"}), 200
    except Exception as e:
        return jsonify({"error": str(e)}), 400

@app.route('/getfiles', methods=['GET'])
def getfiles():
    with storage_lock:
        filenames = list(file_storage.keys())
    
    return jsonify({"files": filenames})

@app.route('/getfile/<filename>', methods=['GET'])
def getfile(filename):
    try:
        with storage_lock:
            if filename not in file_storage:
                return jsonify({"error": "File not found!"}), 404

            file_data = file_storage[filename]
            file_data.seek(0)

        return send_file(file_data, as_attachment=True, download_name=filename)
    
    except Exception as e:
        log(f'SERVER ERROR: {str(e)}')
        return jsonify({"error": str(e)}), 400

@app.route('/deletefile/<filename>', methods=['DELETE'])
def deletefile(filename):
    try:
        with storage_lock:
            if filename in file_storage:
                del file_storage[filename]
                return jsonify({"message": f"File {filename} deleted successfully!"}), 200
            else:
                return jsonify({"error": "File not found!"}), 404
    except Exception as e:
        return jsonify({"error": str(e)}), 400

if __name__ == '__main__':
    app.run(debug=False, threaded=True)
