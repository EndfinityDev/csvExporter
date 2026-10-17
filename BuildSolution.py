import os

def main():
    os.system("msbuild csvExporter.sln /p:Configuration=Release /p:Platform=x64")
    os.system("msbuild csvExporter.sln /p:Configuration=Release /p:Platform=x86")

if __name__ == "__main__":
    main()