import os
import sys
import time
import shutil
import zipfile
import traceback

def main():
    try:
        cwd = ""
        for i in range(len(sys.argv)):
            if sys.argv[i] == "--cwd" and len(sys.argv) > (i+1):
                cwd = sys.argv[i+1]
                if cwd[-1] == "\\":
                    cwd = cwd[:-1]
                break

        if cwd == "":
            cwd = os.getcwd()

        if not os.path.exists(cwd + "\\Builds\\.Distribution\\Installer"):
            os.makedirs(cwd + "\\Builds\\.Distribution\\Installer")

        # Binaries
        binariesArchive = zipfile.ZipFile(cwd + "\\Builds\\.Distribution\\Installer\\binaries.zip", mode="w", compression=zipfile.ZIP_DEFLATED)

        binariesArchive.write(cwd + "\\Builds\\Release\\csvExporter2_x64.dll", "csvExporter2_x64.dll")
        binariesArchive.write(cwd + "\\Builds\\Release\\csvExporter2_x86.dll", "csvExporter2_x86.dll")

        binariesArchive.close()
        
        # UserFiles
        userFilesArchive = zipfile.ZipFile(cwd + "\\Builds\\.Distribution\\Installer\\userfiles.zip", mode="w", compression=zipfile.ZIP_DEFLATED)

        userFilesArchive.write(cwd + "\\AutomationExportExample\\Resources\\ExportExample.lua", "ExportExample.lua")
        userFilesArchive.write(cwd + "\\AutomationExportExample\\Resources\\csvExporter_dataset.json", "DatasetExample.json")
        userFilesArchive.write(cwd + "\\AutomationExportExample\\Resources\\csvExporter_translations.json", "TranslationsExample.json")

        userFilesArchive.write(cwd + "\\Example_UserFiles\\example_override.lua", "example_override.lua")
        userFilesArchive.write(cwd + "\\Example_UserFiles\\example_params.lua", "example_params.lua")
        userFilesArchive.write(cwd + "\\Example_UserFiles\\dump_data.lua", "dump_data.lua")
        userFilesArchive.write(cwd + "\\Example_UserFiles\\prepended_dataset.json", "prepended_dataset.json")
        userFilesArchive.write(cwd + "\\Example_UserFiles\\prepended_translations.json", "prepended_translations.json")

        userFilesArchive.write(cwd + "\\Example_UserFiles\\.ExportParams\\example.json", ".ExportParams\\example.json.example")
        userFilesArchive.write(cwd + "\\Example_UserFiles\\.ExportParams\\example.jsonc", ".ExportParams\\example.jsonc.example")
        userFilesArchive.write(cwd + "\\Example_UserFiles\\.ExportParams\\web_example.json", ".ExportParams\\web_example.json.example")
        userFilesArchive.write(cwd + "\\Example_UserFiles\\.ExportParams\\web_schema_example.json", ".ExportParams\\web_schema_example.json.example")
        userFilesArchive.write(cwd + "\\Example_UserFiles\\.ExportParams\\imperial.json", ".ExportParams\\imperial.json")

        userFilesArchive.close()

        # ReadMe
        shutil.copy2(cwd + "\\Builds\\Release\\ReadMe.txt", cwd + "\\Builds\\.Distribution\\Installer\\ReadMe.txt")
        
        print("Packaging installer content complete")
    except:
        traceback.print_exc()

    if "--nowait" not in sys.argv:
        time.sleep(3)

if __name__ == "__main__":
    main()
