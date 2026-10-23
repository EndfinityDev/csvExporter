#pragma once

#include <cwchar>
#include <string>
#include <array>
#include <vector>
#include <unordered_map>
#include <ShlObj.h>

#include "DirectoryManager.h"

#define CSVE_CUSTOM_PREFS_STR_MAX_LENGTH 25

using ExportParamString = std::array<wchar_t, CSVE_CUSTOM_PREFS_STR_MAX_LENGTH>;

class ExportParamStringConverter
{
public:
    static ExportParamString FromWString(const wchar_t* inString) {
        ExportParamString paramString = {}; // Zero-initializes the entire buffer (\0)

        if (inString != nullptr) {
            // arr.size() extracts the capacity dynamically at compile time
            wcsncpy_s(paramString.data(), paramString.size(), inString, _TRUNCATE);
        }

        return paramString;
    }

    static ExportParamString FromWString(const std::wstring& inString) {
        return FromWString(inString.c_str());
    }

    static std::wstring ToWString(const ExportParamString& paramString)
    {
        return std::wstring(paramString.data());
    }
};

struct ExportCustomStringParam
{
    std::wstring Label = L"";
    std::wstring Tooltip = L"";
    std::wstring Value = L"";
    std::wstring LuaName = L"";

    bool AllowSavingToPrefs = false;
};

#define CSVE_CUSTOM_PREFS_STRING_VERSION 0

struct ExportCustomStringParamPrefs
{
    uint8_t Version = CSVE_CUSTOM_PREFS_STRING_VERSION;
    std::vector<std::pair<ExportParamString, ExportParamString>> ParamTable;

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
                ExportParamStringConverter::FromWString(param.LuaName),
                ExportParamStringConverter::FromWString(param.Value)
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
            map.emplace(ExportParamStringConverter::ToWString(items.first), ExportParamStringConverter::ToWString(items.second));
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
                fread_s(&prefsFileVersion, sizeof(prefsFileVersion), sizeof(prefsFileVersion), 1, file);

                if (prefsFileVersion == CSVE_CUSTOM_PREFS_STRING_VERSION)
                {
                    size_t customParamsCount;
                    fread_s(&customParamsCount, sizeof(customParamsCount), sizeof(customParamsCount), 1, file);
                    exportParamsPrefs.ParamTable.resize(customParamsCount);
                    for (size_t i = 0; i < customParamsCount; i++)
                    {
                        fread_s(&exportParamsPrefs.ParamTable[i].first, sizeof(ExportParamString), sizeof(ExportParamString), 1, file);
                        fread_s(&exportParamsPrefs.ParamTable[i].second, sizeof(ExportParamString), sizeof(ExportParamString), 1, file);
                    }
                }
                else
                {
                    //TODO
                }

                fclose(file);
            }
            else
            {
                // TODO
            }
        }
        else
        {
            //TODO
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
                    //TODO
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
                // TODO
                return false;
            }

            fwrite(&exportParamsPrefs.Version, sizeof(exportParamsPrefs.Version), 1, file);
            size_t customParamsCount = exportParamsPrefs.ParamTable.size();
            fwrite(&customParamsCount, sizeof(customParamsCount), 1, file);
            //fwrite(&exportParamsPrefs.ParamTable, sizeof(std::pair<ExportParamString, bool>), customParamsCount, file);
            for (size_t i = 0; i < customParamsCount; i++)
            {
                fwrite(&exportParamsPrefs.ParamTable[i].first, sizeof(ExportParamString), 1, file);
                fwrite(&exportParamsPrefs.ParamTable[i].second, sizeof(ExportParamString), 1, file);
            }

            fclose(file);

            return true;
        }

        //TODO
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
                    //TODO
                    //return exportParamsPrefs;
                }                
                else if (prefsFileVersion == CSVE_CUSTOM_PREFS_BOOL_VERSION)
                {
                    //size_t customParamsCount;
                    //size_t readStatus = fread_s(&customParamsCount, sizeof(customParamsCount), sizeof(customParamsCount), 1, file);
                    //if (!readStatus)
                    //{
                    //    //TODO
                    //    return exportParamsPrefs;
                    //}
                    //
                    //exportParamsPrefs.ParamTable.resize(customParamsCount);
                    //for (size_t i = 0; i < customParamsCount; i++)
                    //{
                    //    fread_s(&exportParamsPrefs.ParamTable[i].first, sizeof(ExportParamString), sizeof(ExportParamString), 1, file);
                    //    fread_s(&exportParamsPrefs.ParamTable[i].second, sizeof(bool), sizeof(bool), 1, file);
                    //}

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
                            //TODO
                            key = L"";
                        }
                        else
                        {
                            key.resize(keySize);
                            readStatus = fread_s(&key[0], key.size() * sizeof(wchar_t), sizeof(wchar_t), keySize, file);
                        }

                        if (readStatus != keySize)
                        {
                            //TODO
                            break;
                        }

                        bool value;
                        readStatus = fread_s(&value, sizeof(value), sizeof(value), 1, file);
                        if (!readStatus)
                        {
                            //TODO
                            break;
                        }
                        exportParamsPrefs.ParamTable.emplace_back(key, value);
                    }
                }
                else
                {
                    //TODO
                }

                fclose(file);
            }
            else
            {
                // TODO
            }
        }
        else
        {
            //TODO
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
                    //TODO
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
                // TODO
                return false;
            }

            fwrite(&exportParamsPrefs.Version, sizeof(exportParamsPrefs.Version), 1, file);

            size_t customParamsCount = exportParamsPrefs.ParamTable.size();
            //fwrite(&customParamsCount, sizeof(customParamsCount), 1, file);
            //fwrite(&exportParamsPrefs.ParamTable, sizeof(std::pair<ExportParamString, bool>), customParamsCount, file);
            //for (size_t i = 0; i < customParamsCount; i++)
            //{
            //    fwrite(&exportParamsPrefs.ParamTable[i].first, sizeof(ExportParamString), 1, file);
            //    fwrite(&exportParamsPrefs.ParamTable[i].second, sizeof(bool), 1, file);
            //}

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

        //TODO
        return false;
    }
};