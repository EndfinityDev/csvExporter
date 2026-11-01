#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

#include "RegistryManager.h"
#include "DirectoryManager.h"

class GameLocationManager
{
public:
    static std::wstring FindSteamGameFolder(const std::wstring& gameAppId, const std::wstring& expectedDirName) {
        std::wstring steamPath = RegistryManager::GetSteamInstallPath();
        if (steamPath.empty()) 
        {
            return L"";
        }

        // Build the collection of active library root directories
        std::vector<std::wstring> libraryPaths;
        libraryPaths.push_back(steamPath + L"\\steamapps");

        // Parse libraryfolders.vdf manually using standard streams
        std::wstring libraryFile = steamPath + L"\\steamapps\\libraryfolders.vdf";

        // Convert path to string for ifstream (Win32 specific conversion)
        std::ifstream file(std::string(libraryFile.begin(), libraryFile.end()));
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                size_t pathPos = line.find("\"path\"");
                if (pathPos != std::string::npos) {
                    size_t firstQuote = line.find("\"", pathPos + 6);
                    size_t secondQuote = line.find("\"", firstQuote + 1);
                    if (firstQuote != std::string::npos && secondQuote != std::string::npos) {
                        std::string rawPath = line.substr(firstQuote + 1, secondQuote - firstQuote - 1);

                        // Remove escaped JSON backslashes ("D:\\Steam" -> "D:\Steam")
                        size_t pos;
                        while ((pos = rawPath.find("\\\\")) != std::string::npos) {
                            rawPath.replace(pos, 2, "\\");
                        }

                        // Convert back to wstring for Unicode directory checking
                        std::wstring wideLibraryPath(rawPath.begin(), rawPath.end());
                        std::wstring targetAppsFolder = wideLibraryPath + L"\\steamapps";

                        if (DirectoryManager::CheckPathIsDirectory(targetAppsFolder)) {
                            libraryPaths.push_back(targetAppsFolder);
                        }
                    }
                }
            }
            file.close();
        }

        // Look for the appmanifest file in the discovered library locations
        std::wstring manifestName = L"\\appmanifest_" + gameAppId + L".acf";
        for (size_t i = 0; i < libraryPaths.size(); ++i) {
            std::wstring manifestFullPath = libraryPaths[i] + manifestName;
            if (DirectoryManager::CheckPathIsNotDirectory(manifestFullPath)) {
                // Once the manifest is matched, build out the final game binary directory layout path
                std::wstring finalGamePath = libraryPaths[i] + L"\\common\\" + expectedDirName;
                if (DirectoryManager::CheckPathIsDirectory(finalGamePath)) {
                    return finalGamePath;
                }
            }
        }

        return L""; // Not found
    }
};