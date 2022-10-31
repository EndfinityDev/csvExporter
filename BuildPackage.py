#import shutil
import os
import time
import zipfile
import traceback

def main():
    try:
        cwd = os.getcwd()
        if not os.path.exists(cwd + "\\Builds\\.Distribution"):
            os.makedirs(cwd + "\\Builds\\.Distribution")
        archive = zipfile.ZipFile(cwd + "\\Builds\\.Distribution\\csvExporter.zip", mode="w")

        archive.write(cwd + "\\Builds\\Release\\csvExporter_x64.dll", "csvExporter_x64.dll")
        archive.write(cwd + "\\Builds\\Release\\csvExporter_x86.dll", "csvExporter_x86.dll")
        archive.write(cwd + "\\Builds\\Release\\ReadMe.txt", "ReadMe.txt")

        archive.close()
        
        print("Packaging complete")
    except:
        traceback.print_exc()

    time.sleep(3)

if __name__ == "__main__":
    main()
