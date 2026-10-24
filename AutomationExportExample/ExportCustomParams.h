#pragma once

#include <cwchar>
#include <string>
#include <array>
#include <vector>
#include <unordered_map>
#include <ShlObj.h>

#include "DirectoryManager.h"
#include "Logger.h"

struct ExportCustomStringParam
{
    std::wstring Label = L"";
    std::wstring Tooltip = L"";
    std::wstring Value = L"";
    std::wstring LuaName = L"";

    bool AllowSavingToPrefs = false;
};

#define CSVE_CUSTOM_PREFS_STRING_VERSION 1

struct ExportCustomStringParamPrefs
{
    uint8_t Version = CSVE_CUSTOM_PREFS_STRING_VERSION;
    std::vector<std::pair<std::wstring, std::wstring>> ParamTable;

    static ExportCustomStringParamPrefs FromExportParams(const std::vector<ExportCustomStringParam>& exportParams)
    {
        ExportCustomStringParamPrefs paramPrefs;
        for(const ExportCustomStringParam& param : exportParams)
        {
            if (!param.AllowSavingToPrefs)
            {
                continue;
            }
            paramPrefs.ParamTable.emplace_back(
                param.LuaName,
                param.Value
            );
        }

        return paramPrefs;
    }

    static std::unordered_map<std::wstring, std::wstring> ToWStringMap(const ExportCustomStringParamPrefs& prefs)
    {
        std::unordered_map<std::wstring, std::wstring> map;
        map.reserve(prefs.ParamTable.size());
        for (const auto& items : prefs.ParamTable)
        {
            map.emplace(items.first, items.second);
        }
        return map;
    }

    static ExportCustomStringParamPrefs LoadFromFile(const std::wstring& directory)
    {
        ExportCustomStringParamPrefs exportParamsPrefs;

        TCHAR path[MAX_PATH];

        //get the user's documents directory:
        if (SHGetFolderPathW(0, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_CURRENT, path) == S_OK)
        {
            std::wstring prefsDir = path;
            prefsDir += directory + L"CSVExporter2_CustomStrings.prefs";


            FILE* file;
            errno_t err = _wfopen_s(&file, prefsDir.c_str(), L"rb");

            if (!err && file)
            {
                uint8_t prefsFileVersion;
                size_t readStatus = fread_s(&prefsFileVersion, sizeof(prefsFileVersion), sizeof(prefsFileVersion), 1, file);
                if (!readStatus)
                {
                    Logger::GetLogger()->Log(L"[ExportCustomParams] Failed to read custom export string parameter prefs file");
                    //return exportParamsPrefs;
                }
                else if (prefsFileVersion == CSVE_CUSTOM_PREFS_STRING_VERSION)
                {
                    while (true)
                    {
                        std::wstring key;
                        size_t keySize;
                        readStatus = fread_s(&keySize, sizeof(keySize), sizeof(keySize), 1, file);
                        if (!readStatus)
                        {
                            break;
                        }
                        if (!keySize)
                        {
                            Logger::GetLogger()->Log(L"[ExportCustomParams] Encountered a zero-length key in string custom prefs file");
                            key = L"";
                        }
                        else
                        {
                            key.resize(keySize);
                            readStatus = fread_s(&key[0], key.size() * sizeof(wchar_t), sizeof(wchar_t), keySize, file);
                            if (readStatus != keySize)
                            {
                                Logger::GetLogger()->Log(L"[ExportCustomParams] Failed to read custom export string parameter key from prefs file");
                                break;
                            }
                        }

                        std::wstring value;
                        size_t valueSize;
                        readStatus = fread_s(&valueSize, sizeof(valueSize), sizeof(valueSize), 1, file);
                        if (!readStatus)
                        {
                            Logger::GetLogger()->Log(L"[ExportCustomParams] Failed to read custom export string parameter value size from prefs file");
                            break;
                        }
                        if (!valueSize)
                        {
                            value = L"";
                        }
                        else
                        {
                            value.resize(valueSize);
                            readStatus = fread_s(&value[0], value.size() * sizeof(wchar_t), sizeof(wchar_t), valueSize, file);
                            if (readStatus != valueSize)
                            {
                                Logger::GetLogger()->Log(L"[ExportCustomParams] Failed to read custom export string parameter value from prefs file");
                                break;
                            }
                        }

                        exportParamsPrefs.ParamTable.emplace_back(key, value);
                    }
                }
                else
                {
                    Logger::GetLogger()->Log(L"[ExportCustomParams] Encountered a mismatch in custom export string parameter prefs file version");
                }

                fclose(file);
            }
            else
            {
                Logger::GetLogger()->Log(L"[ExportCustomParams] Failed to open custom export string parameter prefs file");
            }
        }
        else
        {
            Logger::GetLogger()->Log(L"[ExportCustomParams] Failed to locate application data directory while reading custom export string parameter prefs file");
        }

        return exportParamsPrefs;
    }

    static bool TryWriteToFile(ExportCustomStringParamPrefs& exportParamsPrefs, const std::wstring& directory)
    {
        TCHAR path[MAX_PATH];

        //get the user's documents directory:
        if (SHGetFolderPathW(0, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_CURRENT, path) == S_OK)
        {
            std::wstring prefsDir = path;
            prefsDir += directory;

            if (!DirectoryManager::CheckPathIsDirectory(prefsDir))
            {
                if (!DirectoryManager::TryCreateDirectoryRecursive(prefsDir))
                {
                    Logger::GetLogger()->Log(L"[ExportCustomParams] Failed to create export prefs directory");
                    return false;
                }
            }
            else
            {
                
                ExportCustomStringParamPrefs oldParams = LoadFromFile(directory);
                for (auto& oldItems : oldParams.ParamTable)
                {
                    bool exists = false;
                    for (auto& newItems : exportParamsPrefs.ParamTable)
                    {
                        if (oldItems.first == newItems.first)
                        {
                            exists = true;
                            break;
                        }
                    }
                    if (!exists)
                    {
                        exportParamsPrefs.ParamTable.push_back(oldItems);
                    }
                }
                
            }

            std::wstring prefsFile = prefsDir + L"CSVExporter2_CustomStrings.prefs";

            FILE* file;
            errno_t err = _wfopen_s(&file, prefsFile.c_str(), L"wb");

            if (err || !file)
            {
                Logger::GetLogger()->Log(L"[ExportCustomParams] Failed to open custom export string parameter prefs file for writing");
                return false;
            }

            fwrite(&exportParamsPrefs.Version, sizeof(exportParamsPrefs.Version), 1, file);
            size_t customParamsCount = exportParamsPrefs.ParamTable.size();

            for (size_t i = 0; i < customParamsCount; i++)
            {
                size_t keySize = exportParamsPrefs.ParamTable[i].first.size();
                fwrite(&keySize, sizeof(keySize), 1, file);
                fwrite(exportParamsPrefs.ParamTable[i].first.c_str(), sizeof(wchar_t), keySize, file);

                size_t valueSize = exportParamsPrefs.ParamTable[i].second.size();
                fwrite(&valueSize, sizeof(valueSize), 1, file);
                fwrite(exportParamsPrefs.ParamTable[i].second.c_str(), sizeof(wchar_t), valueSize, file);
            }

            fclose(file);

            return true;
        }

        Logger::GetLogger()->Log(L"[ExportCustomParams] Failed to find application data directory");
        return false;
    }
};

struct ExportCustomBoolParam
{
    std::wstring Label = L"";
    std::wstring Tooltip = L"";
    bool Value = false;
    std::wstring LuaName = L"";

    bool AllowSavingToPrefs = false;
};

#define CSVE_CUSTOM_PREFS_BOOL_VERSION 1

struct ExportCustomBoolParamPrefs
{
    uint8_t Version = CSVE_CUSTOM_PREFS_BOOL_VERSION;
    std::vector<std::pair<std::wstring, bool>> ParamTable;

    static ExportCustomBoolParamPrefs FromExportParams(const std::vector<ExportCustomBoolParam>& exportParams)
    {
        ExportCustomBoolParamPrefs paramPrefs;
        for (const ExportCustomBoolParam& param : exportParams)
        {
            if (!param.AllowSavingToPrefs)
            {
                continue;
            }
            paramPrefs.ParamTable.emplace_back(
                //ExportParamStringConverter::FromWString(param.LuaName),
                param.LuaName,
                param.Value
            );
        }

        return paramPrefs;
    }

    static std::unordered_map<std::wstring, bool> ToWStringMap(const ExportCustomBoolParamPrefs& prefs)
    {
        std::unordered_map<std::wstring, bool> map;
        map.reserve(prefs.ParamTable.size());
        for (const auto& items : prefs.ParamTable)
        {
            //map.emplace(ExportParamStringConverter::ToWString(items.first), items.second);
            map.emplace(items.first, items.second);
        }
        return map;
    }

    static ExportCustomBoolParamPrefs LoadFromFile(const std::wstring& directory)
    {
        ExportCustomBoolParamPrefs exportParamsPrefs;

        TCHAR path[MAX_PATH];

        //get the user's documents directory:
        if (SHGetFolderPathW(0, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_CURRENT, path) == S_OK)
        {
            std::wstring prefsDir = path;
            prefsDir += directory + L"CSVExporter2_CustomBools.prefs";


            FILE* file;
            errno_t err = _wfopen_s(&file, prefsDir.c_str(), L"rb");

            if (!err && file)
            {
                uint8_t prefsFileVersion;
                size_t readStatus = fread_s(&prefsFileVersion, sizeof(prefsFileVersion), sizeof(prefsFileVersion), 1, file);
                if (!readStatus)
                {
                    Logger::GetLogger()->Log(L"[ExportCustomParams] Failed to read custom export boolean parameter prefs file");
                    //return exportParamsPrefs;
                }                
                else if (prefsFileVersion == CSVE_CUSTOM_PREFS_BOOL_VERSION)
                {

                    while (true)
                    {
                        std::wstring key;
                        size_t keySize;
                        readStatus = fread_s(&keySize, sizeof(keySize), sizeof(keySize), 1, file);
                        if (!readStatus)
                        {
                            break;
                        }
                        if (!keySize)
                        {
                            Logger::GetLogger()->Log(L"[ExportCustomParams] Encountered a zero-length key in boolean custom prefs file");
                            key = L"";
                        }
                        else
                        {
                            key.resize(keySize);
                            readStatus = fread_s(&key[0], key.size() * sizeof(wchar_t), sizeof(wchar_t), keySize, file);
                            if (readStatus != keySize)
                            {
                                Logger::GetLogger()->Log(L"[ExportCustomParams] Failed to read custom export boolean parameter key from prefs file");
                                break;
                            }
                        }

                        bool value;
                        readStatus = fread_s(&value, sizeof(value), sizeof(value), 1, file);
                        if (!readStatus)
                        {
                            Logger::GetLogger()->Log(L"[ExportCustomParams] Failed to read custom export boolean parameter value from prefs file");
                            break;
                        }
                        exportParamsPrefs.ParamTable.emplace_back(key, value);
                    }
                }
                else
                {
                    Logger::GetLogger()->Log(L"[ExportCustomParams] Encountered a mismatch in custom export boolean parameter prefs file version");
                }

                fclose(file);
            }
            else
            {
                Logger::GetLogger()->Log(L"[ExportCustomParams] Failed to open custom export boolean parameter prefs file");
            }
        }
        else
        {
            Logger::GetLogger()->Log(L"[ExportCustomParams] Failed to locate application data directory while reading custom export boolean parameter prefs file");
        }

        return exportParamsPrefs;
    }

    static bool TryWriteToFile(ExportCustomBoolParamPrefs& exportParamsPrefs, const std::wstring& directory)
    {
        TCHAR path[MAX_PATH];

        //get the user's documents directory:
        if (SHGetFolderPathW(0, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_CURRENT, path) == S_OK)
        {
            std::wstring prefsDir = path;
            prefsDir += directory;

            if (!DirectoryManager::CheckPathIsDirectory(prefsDir))
            {
                if (!DirectoryManager::TryCreateDirectoryRecursive(prefsDir))
                {
                    Logger::GetLogger()->Log(L"[ExportCustomParams] Failed to create export prefs directory");
                    return false;
                }
            }
            else
            {
                ExportCustomBoolParamPrefs oldParams = LoadFromFile(directory);
                for (auto& oldItems : oldParams.ParamTable)
                {
                    bool exists = false;
                    for (auto& newItems : exportParamsPrefs.ParamTable)
                    {
                        if (oldItems.first == newItems.first)
                        {
                            exists = true;
                            break;
                        }
                    }
                    if (!exists)
                    {
                        exportParamsPrefs.ParamTable.push_back(oldItems);
                    }
                }
            }

            std::wstring prefsFile = prefsDir + L"CSVExporter2_CustomBools.prefs";

            FILE* file;
            errno_t err = _wfopen_s(&file, prefsFile.c_str(), L"wb");

            if (err || !file)
            {
                Logger::GetLogger()->Log(L"[ExportCustomParams] Failed to open custom export boolean parameter prefs file for writing");
                return false;
            }

            fwrite(&exportParamsPrefs.Version, sizeof(exportParamsPrefs.Version), 1, file);

            size_t customParamsCount = exportParamsPrefs.ParamTable.size();

            for (size_t i = 0; i < customParamsCount; i++)
            {
                size_t keySize = exportParamsPrefs.ParamTable[i].first.size();
                fwrite(&keySize, sizeof(keySize), 1, file);
                fwrite(exportParamsPrefs.ParamTable[i].first.c_str(), sizeof(wchar_t), keySize, file);
                fwrite(&exportParamsPrefs.ParamTable[i].second, sizeof(bool), 1, file);
            }

            fclose(file);

            return true;
        }

        Logger::GetLogger()->Log(L"[ExportCustomParams] Failed to find application data directory");
        return false;
    }
};