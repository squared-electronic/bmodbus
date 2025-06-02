import requests
import base64
import time
import zipfile
import io
import os
import argparse
import sys

"""
This script runs a Python program on a remote firmware testing service, sending an Arduino binary file and a Python script to be executed on the specified board type.
It then waits for the results of the execution and prints them to the console.
"""


def generate_zip_from_folder(folder_path):
    # this generates a zip file from the contents of a folder
    zip_buffer = io.BytesIO()
    with zipfile.ZipFile(zip_buffer, 'w', zipfile.ZIP_DEFLATED) as zip_file:
        for root, dirs, files in os.walk(folder_path):
            for file in files:
                file_path = os.path.join(root, file)
                arcname = os.path.relpath(file_path, folder_path)
                zip_file.write(file_path, arcname)
    zip_buffer.seek(0)  # Reset the buffer's file pointer to the beginning
    return zip_buffer.getvalue()  # Return the binary content of the zip file


def run_program(server, python_program, binary_program, board_type):
    # run a program via http requests to a remote firmware service, and wait for the results
    response = requests.post(server+"/start", json={"python": python_program, "binary": binary_program, "board": board_type, "auth": "1234"}, headers={"Content-Type": "application/json"})
    if response.status_code == 200:
        data = response.json()
        # print(data.keys())
        test_id = data.get("test_id")
    else:
        print(f"Error: {response.status_code} - {response.text}")
        return -400
    busy = True
    while busy:
        response = requests.post(server + "/status", headers={"Content-Type": "application/json"}, json={"test_id": test_id, "auth": "1234"})
        if response.status_code == 200:
            data = response.json()
            busy = data.get("status", "running") == "running"
            if not busy:
                print("Python completed.")
                results = requests.post(server + "/results", headers={"Content-Type": "application/json"}, json={"test_id": test_id, "auth": "1234"})
                if results.status_code == 200:
                    results_data = results.json()
                    print(results_data["results"]["stdout"])
                    exit_code = results_data["results"]["exit_code"]
                    return exit_code
                else:
                    print(f"Error fetching results: {results.status_code} - {results.text}")
                break
            else:
                print(data["message"])
        else:
            print(f"Error checking status: {response.status_code} - {response.text}")
        time.sleep(1)
    return -401


if __name__ == "__main__":
    #bin = open("esp32.arduino_unit_tests.ino.bin", "rb").read()
    #bin = open("arduino_unit_tests.ino.merged.zip", "rb").read()
    parser = argparse.ArgumentParser(description="Run a program on a remote firmware service.")
    parser.add_argument("board_type", type=str, help="The full arduino board type (fqbn) to use, e.g. esp32:esp32:esp32 or adafruit:samd:adafruit_feather_m4.")
    parser.add_argument("path_to_python", type=str, default=None, help="Path to the Python program to run.")
    parser.add_argument("path_to_folder_with_ino", type=str, default=None, help="Path to the folder containing the Arduino .ino files.")
    parser.add_argument("server", type=str, help="The server URL of the remote firmware service.")
    args = parser.parse_args()
    if args.path_to_python is None or args.path_to_folder_with_ino is None or args.server is None or args.board_type is None:
        print("You must provide a path to the Python program and a folder with Arduino .ino files.")
    else:
        python_program = open(args.path_to_python, "r").read()
        folder = args.path_to_folder_with_ino
        board_type = args.board_type
        base_folder = os.path.join(folder, "build", board_type.replace(":", "."))
        zip_in_ram = generate_zip_from_folder(base_folder)
        binary_program = base64.b64encode(zip_in_ram).decode('utf-8')
        results = run_program(args.server, python_program, binary_program, board_type)
        print("Results:", results)
        sys.exit(results)
    # for example:
    # python run_my_program.py esp32:esp32:esp32 C:\customers\internal\bmodbus\tests\arduino\arduino_unit_tests\interactive_tests.py C:\customers\internal\bmodbus\tests\arduino\arduino_unit_tests http://192.168.150.2:5000