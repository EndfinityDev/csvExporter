#pragma once

#include <cwchar>
#include <string>
#include <array>
#include <vector>
#include <unordered_map>

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

    bool allowSavingToPrefs = false;
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
            if (!param.allowSavingToPrefs)
            {
                continue;
            }
            paramPrefs.ParamTable.emplace_back(
                ExportParamStringConverter::FromWString(param.Label),
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
};
