#include "stdafx.h"

#include "ExportCustomParamsManager.h"
#include "JSONFileManager.h"
#include "DirectoryManager.h"
#include "StringConverter.h"
#include "HTTPManager.h"


ExportCustomParamsGroup ExportCustomParamsManager::LoadParameterJSONs(const std::wstring& path, bool allowWebString, bool allowWebBool)
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

						ParseParameterList(items.value(), exportParamGroup, allowWebString, allowWebBool);
					}
				}
				else if (jsonData.is_array())
				{
					ParseParameterList(jsonData, exportParamGroup, allowWebString, allowWebBool);
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

bool ExportCustomParamsManager::TrySaveParamsGroup(const ExportCustomParamsGroup& exportParamGroup, const std::wstring& directory)
{
	bool success = ExportCustomBoolParamPrefs::TryWriteToFile(ExportCustomBoolParamPrefs::FromExportParams(exportParamGroup.BoolParams), directory);
	success = success && ExportCustomStringParamPrefs::TryWriteToFile(ExportCustomStringParamPrefs::FromExportParams(exportParamGroup.StringParams), directory);
	return success;
}

void ExportCustomParamsManager::ParseParameterList(const json& array, ExportCustomParamsGroup& exportParamGroup, bool allowWebString, bool allowWebBool)
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
		if (parameterType == "string" || parameterType == "str")
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
		
		if (label.empty())
		{
			label = luaName;
		}

		json webValue;

		if (((isStringParameter && allowWebString) || (!isStringParameter && allowWebBool)) &&
				parameter.contains("web") && parameter["web"].is_object())
		{
			auto& web = parameter["web"];
			//if (web.contains("url") && web["url"].is_object() && 
			//	web["url"].contains("host") && web["url"]["host"].is_string() && 
			//	(web.contains("type") && web["type"].is_string() && 
			//	(web["type"].get<std::string>() == "text" && isStringParameter) || (web["type"].get<std::string>() == "json")))
			if (web.contains("url") && web["url"].is_object() &&
				web["url"].contains("host") && web["url"]["host"].is_string() &&
				web.contains("type") && web["type"].is_string() &&
				((web["type"].get<std::string>() == "text" && isStringParameter) || (web["type"].get<std::string>() == "json")))
			{
				std::string urlHost = web["url"]["host"].get<std::string>();
				std::string urlPath = "";
				if (web["url"].contains("path") && web["url"]["path"].is_string())
				{
					urlPath = web["url"]["path"].get<std::string>();
				}

				std::string responseType = web["type"].get<std::string>();

				httplib::Params params;
				if (web["url"].contains("params") && web["url"]["params"].is_array())
				{
					for (const auto& urlParam : web["url"]["params"])
					{
						std::string paramName;
						json paramValue;
						if (urlParam.is_array() && urlParam.size() == 2)
						{
							paramName = urlParam[0].get<std::string>();
							paramValue = urlParam[1];
						}
						else if (urlParam.is_object() && urlParam.contains("name") && urlParam.contains("value"))
						{
							paramName = urlParam["name"].get<std::string>();
							paramValue = urlParam["value"];
						}
						if (!paramValue.is_null())
						{
							if (paramValue.is_string())
							{
								params.emplace(paramName, paramValue.get<std::string>());
							}
							else if (paramValue.is_number())
							{
								params.emplace(paramName, std::to_string(paramValue.get<float>()));
								//params.emplace(paramName, paramValue.get<float>());
							}
							else if (paramValue.is_boolean())
							{
								//params.emplace(paramName, paramValue.get<bool>());
								params.emplace(paramName, paramValue.get<bool>() ? "true" : "false");
							}
						}
					}
				}

				std::vector<int> allowedStatuses = { 200 };

				if (web.contains("allowedStatuses") && web["allowedStatuses"].is_array())
				{
					try {
						std::vector<int> allowedStatuses = web["allowedStatuses"].get<std::vector<int>>();
					}
					catch (const nlohmann::json::type_error& e) {
						//TODO
					}
				}

				int status = 0;
				std::string response = HTTPManager::Get(urlHost, urlPath, params, true, &status);
				if (!response.empty() && std::find(allowedStatuses.begin(), allowedStatuses.end(), status) != allowedStatuses.end())
				{
					if (responseType == "text")
					{
						webValue = response;
					}
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

		if (isStringParameter)
		{
			std::wstring value = L"";

			if (!webValue.is_null() && webValue.is_string())
			{
				value = StringConverter::to_wstring(webValue.get<std::string>());
			}
			else if (parameter.contains("defaultValue"))
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

		exportParamGroup.BoolParams.emplace_back(ExportCustomBoolParam{ label, tooltip, value, luaName, saveToPrefs });
		continue;
	}
}