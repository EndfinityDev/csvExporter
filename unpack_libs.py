import os
import zipfile
import sys

def main():
    #cwd = os.getcwd()
    #print(cwd)
    #print(sys.argv[1])
    cwd = sys.argv[1]
    zipFile = zipfile.ZipFile(cwd + "libs\\cJSON\\lib.zip", 'r')
    zipFile.extractall(cwd + "libs\\cJSON\\")
    zipFile.close()
    print("CJSON Library updated")

if __name__ == "__main__":
    main()