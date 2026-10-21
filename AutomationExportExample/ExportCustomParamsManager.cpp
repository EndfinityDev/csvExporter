#include "stdafx.h"

#include "ExportCustomParamsManager.h"
#include "JSONFileManager.h"
#include "DirectoryManager.h"
#include "StringConverter.h"


ExportCustomParamsGroup ExportCustomParamsManager::LoadParameterJSONs(const std::wstring& path)
{
	ExportCustomParamsGroup exportParamGroup;

	if (DirectoryManager::CheckPathIsDirectory(path))
	{
		std::vector<std::wstring> jsonFiles = DirectoryManager::GetDirectoryFilesWithExt(path, { L".json", L".jsonc"});

		for (std::wstring& fileName : jsonFiles)
		{
			json jsonData;
			if (JSONFileManager::TryParseJSONFile(fileName, path, &jsonData, true, JSONDataType::ANY))
			{
				if (jsonData.is_object())
				{
					for (const auto& items : jsonData.items())
					{
						if (!items.value().is_array())
						{
							//TODO
							continue;
						}

						ParseParameterList(items.value(), exportParamGroup);
					}
				}
				else if (jsonData.is_array())
				{
					ParseParameterList(jsonData, exportParamGroup);
				}
				else
				{
					//TODO
				}
			}
			else
			{
				//TODO
			}
		}
	}

	return exportParamGroup;
}

void ExportCustomParamsManager::ParseParameterList(const json& array, ExportCustomParamsGroup& exportParamGroup)
{
	for (const auto& parameter : array)
	{
		if (!parameter.is_object())
		{
			//TODO
			continue;
		}

		if (!parameter.contains("type"))
		{
			//TODO
			continue;
		}

		bool isStringParameter = false;
		std::string parameterType = parameter["type"].get<std::string>();
		if (parameterType == "string")
		{
			isStringParameter = true;
		}
		else if (parameterType != "bool" && parameterType != "boolean")
		{
			//TODO
			continue;
		}

		std::wstring label = L"";
		std::wstring tooltip = L"";
		std::wstring luaName = L"";
		bool saveToPrefs = false;

		if (parameter.contains("label"))
		{
			if (!parameter["label"].is_string())
			{
				//TODO
				continue;
			}
			label = StringConverter::to_wstring(parameter["label"].get<std::string>());
		}
		if (parameter.contains("tooltip"))
		{
			if (!parameter["tooltip"].is_string())
			{
				//TODO
				continue;
			}
			tooltip = StringConverter::to_wstring(parameter["tooltip"].get<std::string>());
		}
		if (parameter.contains("save"))
		{
			if (!parameter["save"].is_boolean())
			{
				//TODO
				continue;
			}
			saveToPrefs = parameter["save"].get<bool>();
		}
		if (!parameter.contains("luaName"))
		{
			//TODO
			continue;
		}
		if (!parameter["luaName"].is_string())
		{
			//TODO
			continue;
		}
		luaName = StringConverter::to_wstring(parameter["luaName"].get<std::string>());
		if (luaName.empty())
		{
			//TODO
			continue;
		}
		if (!StringUtils::ValidateLuaVariable(luaName))
		{
			//TODO
			continue;
		}
		

		if (label.empty())
		{
			label = luaName;
		}

		if (isStringParameter)
		{
			std::wstring value = L"";
			if (parameter.contains("defaultValue"))
			{
				if (!parameter["defaultValue"].is_string())
				{
					//TODO
					continue;
				}
				value = StringConverter::to_wstring(parameter["defaultValue"].get<std::string>());
			}

			exportParamGroup.StringParams.emplace_back(ExportCustomStringParam{ label, tooltip, value, luaName, saveToPrefs });
			continue;
		}

		bool value = false;
		if (parameter.contains("defaultValue"))
		{
			if (!parameter["defaultValue"].is_boolean())
			{
				//TODO
				continue;
			}
			value = parameter["defaultValue"].get<bool>();
		}

		//exportParamGroup.BoolParams.emplace_back(ExportCustomBoolParam{ label, tooltip, value, saveToPrefs });
		continue;
	}
}