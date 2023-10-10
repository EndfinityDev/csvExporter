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
        archive = zipfile.ZipFile(cwd + "\\Builds\\.Distribution\\csvExporter2.zip", mode="w")

        archive.write(cwd + "\\Builds\\Release\\csvExporter2_x64.dll", "csvExporter2_x64.dll")
        archive.write(cwd + "\\Builds\\Release\\csvExporter2_x86.dll", "csvExporter2_x86.dll")
        archive.write(cwd + "\\Builds\\Release\\ReadMe.txt", "ReadMe.txt")

        archive.write(cwd + "\\AutomationExportExample\\Resources\\ExportExample.lua", "AutomationGame\\CSVExporter 2\\UserFiles\\ExportExample.lua")
        archive.write(cwd + "\\AutomationExportExample\\Resources\\csvExporter_dataset.json", "AutomationGame\\CSVExporter 2\\UserFiles\\DatasetExample.json")
        archive.write(cwd + "\\AutomationExportExample\\Resources\\csvExporter_translations.json", "AutomationGame\\CSVExporter 2\\UserFiles\\TranslationsExample.json")

        archive.close()
        
        print("Packaging complete")
    except:
        traceback.print_exc()

    time.sleep(3)

if __name__ == "__main__":
    main()
