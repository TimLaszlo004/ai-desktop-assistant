from flask import Flask, request, jsonify, send_file
from io import BytesIO
import threading
from win11toast import toast

def log(msg:str):
    with open('logfile.txt', 'a') as file:
        file.write(msg)
        file.write('\n')

app = Flask(__name__)

# In-memory file storage (Dictionary of filename -> BytesIO data)
file_storage = {}

# Lock for handling concurrency (thread-safe file access)
storage_lock = threading.Lock()

# Store file: Save binary data in memory with a filename
@app.route('/storefile', methods=['POST'])
def storefile():
    try:
        # Retrieve the file from the request
        filename = request.form['filename']
        file = request.files['file']
        file_data = file.read()  # Read the binary data of the file
        
        with storage_lock:
            # Store the file in memory
            file_storage[filename] = BytesIO(file_data)
        
        return jsonify({"message": "File stored successfully!"}), 200
    except Exception as e:
        return jsonify({"error": str(e)}), 400

# Get list of all stored files
@app.route('/getfiles', methods=['GET'])
def getfiles():
    with storage_lock:
        filenames = list(file_storage.keys())
    
    return jsonify({"files": filenames})

# Retrieve a specific file by filename
@app.route('/getfile/<filename>', methods=['GET'])
def getfile(filename):
    try:
        with storage_lock:
            if filename not in file_storage:
                return jsonify({"error": "File not found!"}), 404

            # Rewind the BytesIO object to the start
            file_data = file_storage[filename]
            file_data.seek(0)

        return send_file(file_data, as_attachment=True, download_name=filename)
    
    except Exception as e:
        log(f'SERVER ERROR: {str(e)}')
        return jsonify({"error": str(e)}), 400

# Delete a specific file by filename
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
    try:
        # toast('Assistant', f'Start server')
        app.run(debug=False, threaded=True)
    except Exception as e:
        toast('Assistant', f'error in server: {str(e)}')