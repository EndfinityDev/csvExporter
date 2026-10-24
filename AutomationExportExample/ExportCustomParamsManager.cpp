#include "stdafx.h"

#include "ExportCustomParamsManager.h"
#include "JSONFileManager.h"
#include "DirectoryManager.h"
#include "StringConverter.h"


ExportCustomParamsGroup ExportCustomParamsManager::LoadParameterJSONs(const std::wstring& path, bool allowWebString, bool allowWebBool)
{
	ExportCustomParamsGroup exportParamGroup;

	if (DirectoryManager::CheckPathIsDirectory(path))
	{
		std::vector<std::wstring> jsonFiles = DirectoryManager::GetDirectoryFilesWithExt(path, { L".json", L".jsonc"});

		std::vector<std::future<ExportCustomParamsWebResult>> webRequests;

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

						ParseParameterList(items.value(), exportParamGroup, webRequests, allowWebString, allowWebBool);
					}
				}
				else if (jsonData.is_array())
				{
					ParseParameterList(jsonData, exportParamGroup, webRequests, allowWebString, allowWebBool);
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

		// Web request data
		for (auto& webRequest : webRequests)
		{
			ExportCustomParamsWebResult webResult = webRequest.get();
			json webValue = webResult.WebValue;

			if (webResult.DataType == ExportParamsDataType::STRING)
			{
				std::wstring value = L"";

				if (!webValue.is_null())
				{
					if (webValue.is_string())
					{
						value = StringConverter::to_wstring(webValue.get<std::string>());
					}
					else if (webValue.is_number())
					{
						if (webValue.is_number_float())
						{
							value = std::to_wstring(webValue.get<float>());
						}
						else
						{
							value = std::to_wstring(webValue.get<int>());
						}
					}
					else if (webValue.is_boolean())
					{
						value = webValue.get<bool>() ? L"true" : L"false";
					}
					else
					{
						//TODO
						continue;
					}

					//exportParamGroup.StringParams.emplace_back(ExportCustomStringParam{ label, tooltip, value, luaName, saveToPrefs });
					for (auto& param : exportParamGroup.StringParams)
					{
						if (param.LuaName != webResult.LuaName)
						{
							continue;
						}
						param.Value = value;
					}
				}
			}
			else if (webResult.DataType == ExportParamsDataType::BOOL)
			{
				bool value = false;

				if (!webValue.is_null())
				{
					if (webValue.is_string())
					{
						value = webValue.get<std::string>().size() > 0;
					}
					else if (webValue.is_number())
					{
						if (webValue.is_number_float())
						{
							value = webValue.get<float>() != 0;
						}
						else
						{
							value = webValue.get<int>() != 0;
						}
					}
					else if (webValue.is_boolean())
					{
						value = webValue.get<bool>();
					}
					else
					{
						//TODO
						continue;
					}

					//exportParamGroup.BoolParams.emplace_back(ExportCustomBoolParam{ label, tooltip, value, luaName, saveToPrefs });
					for (auto& param : exportParamGroup.BoolParams)
					{
						if (param.LuaName != webResult.LuaName)
						{
							continue;
						}
						param.Value = value;
					}
				}
			}
			else
			{
				// TODO
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

void ExportCustomParamsManager::ParseParameterList(const json& array, ExportCustomParamsGroup& exportParamGroup,
													std::vector<std::future<ExportCustomParamsWebResult>>& webRequests, bool allowWebString, bool allowWebBool)
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
		if (luaName.empty() /* || (luaName.size() >= CSVE_CUSTOM_PREFS_STR_MAX_LENGTH) */)
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

		//json webValue;

		if (((isStringParameter && allowWebString) || (!isStringParameter && allowWebBool)) &&
				parameter.contains("web") && parameter["web"].is_object())
		{
			auto& web = parameter["web"];
			//if (web.contains("url") && web["url"].is_object() && 
			//	web["url"].contains("host") && web["url"]["host"].is_string() && 
			//	(web.contains("type") && web["type"].is_string() && 
			//	(web["type"].get<std::string>() == "text" && isStringParameter) || (web["type"].get<std::string>() == "json")))

			bool isURLBlockValid = web.contains("url") && web["url"].is_object();
			isURLBlockValid = isURLBlockValid && (web["url"].contains("host") && web["url"]["host"].is_string());
			isURLBlockValid = isURLBlockValid && (web.contains("type") && web["type"].is_string());
			isURLBlockValid = isURLBlockValid && (((web["type"].get<std::string>() == "text") && isStringParameter) || 
														(web["type"].get<std::string>() == "json"));

			if (isURLBlockValid)
			{
				std::string urlHost = web["url"]["host"].get<std::string>();
				std::string urlPath = "/";
				if (web["url"].contains("path") && web["url"]["path"].is_string())
				{
					urlPath = web["url"]["path"].get<std::string>();
				}

				std::string responseTypeString = web["type"].get<std::string>();
				ExportParamsWebResponseType responseType = ExportParamsWebResponseType::UNKNOWN;
				auto& iterator = ExportParamsWebResponseTypeMap.find(responseTypeString);
				if (iterator != ExportParamsWebResponseTypeMap.end())
				{
					responseType = iterator->second;
				}

				if (responseType == ExportParamsWebResponseType::UNKNOWN)
				{
					continue;
				}

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
								if (paramValue.is_number_float())
								{
									params.emplace(paramName, std::to_string(paramValue.get<float>()));
								}
								else
								{
									params.emplace(paramName, std::to_string(paramValue.get<int>()));
								}
							}
							else if (paramValue.is_boolean())
							{
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

				bool textOnFail = false;
				if (web.contains("textOnFail") && web["textOnFail"].is_boolean())
				{
					textOnFail = web["textOnFail"].get<bool>();
				}

				//ExportCustomParamsWebResult webResult = GetParameterValueFromWeb({ luaName,
				//																	urlHost, urlPath, params,
				//																	isStringParameter ? ExportParamsDataType::STRING : ExportParamsDataType::BOOL,
				//																	responseType, web.contains("schema") ? web["schema"] : json(),
				//																	allowedStatuses, textOnFail });
				//webValue = webResult.WebValue;


				WebValueRequestParams webRequestParams = { luaName,
											urlHost, urlPath, params,
											isStringParameter ? ExportParamsDataType::STRING : ExportParamsDataType::BOOL,
											responseType, web.contains("schema") ? web["schema"] : json(),
											allowedStatuses, textOnFail };

				webRequests.push_back(std::async(std::launch::async, [webRequestParams]() {
						return GetParameterValueFromWeb(webRequestParams);
					}));

				/*
				int status = 0;
				std::string response = HTTPManager::Get(urlHost, urlPath, params, false, true, &status);
				if (!response.empty() && std::find(allowedStatuses.begin(), allowedStatuses.end(), status) != allowedStatuses.end())
				{
					if (responseType == ExportParamsWebResponseType::TEXT)
					{
						webValue = response;
					}
					else
					{
						json jsonData; 
						if (JSONFileManager::TryParseJSONData<std::string>(response, &jsonData, true, JSONDataType::ANY))
						{
							if (web.contains("schema") && web["schema"].is_structured())
							{
								webValue = JSONFileManager::GetJSONValueBySchema(jsonData, web["schema"]);
							}
							else
							{
								//TODO
							}
						}
					}
				}
				else 
				{
					if (textOnFail && !response.empty())
					{
						webValue = response;
					}
					//TODO
				}
				*/
			}
			else
			{
				//TODO
			}
		}

		if (isStringParameter)
		{
			std::wstring value = L"";

			/*
			if (!webValue.is_null())
			{
				if (webValue.is_string())
				{
					value = StringConverter::to_wstring(webValue.get<std::string>());
				}
				else if (webValue.is_number())
				{
					if (webValue.is_number_float())
					{
						value = std::to_wstring(webValue.get<float>());
					}
					else
					{
						value = std::to_wstring(webValue.get<int>());
					}
				}
				else if (webValue.is_boolean())
				{
					value = webValue.get<bool>() ? L"true" : L"false";
				}
			}
			else 
			*/	

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

		/*
		if (!webValue.is_null())
		{
			if (webValue.is_string())
			{
				value = webValue.get<std::string>().size() > 0;
			}
			else if (webValue.is_number())
			{
				if (webValue.is_number_float())
				{
					value = webValue.get<float>() != 0;
				}
				else
				{
					value = webValue.get<int>() != 0;
				}
			}
			else if (webValue.is_boolean())
			{
				value = webValue.get<bool>();
			}
		}
		else 
		*/
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

ExportCustomParamsWebResult ExportCustomParamsManager::GetParameterValueFromWeb(const WebValueRequestParams webRequestParams)
{
	json webValue;

	int status = 0;
	std::string response = HTTPManager::Get(webRequestParams.Host, webRequestParams.Path, webRequestParams.Params, false, true, &status);
	if (!response.empty() && 
		std::find(webRequestParams.AllowedStatuses.begin(), webRequestParams.AllowedStatuses.end(), status) != webRequestParams.AllowedStatuses.end())
	{
		if (webRequestParams.ResponseType == ExportParamsWebResponseType::TEXT)
		{
			webValue = response;
		}
		else
		{
			json jsonData;
			if (JSONFileManager::TryParseJSONData<std::string>(response, &jsonData, true, JSONDataType::ANY))
			{
				//if (web.contains("schema") && web["schema"].is_structured())
				if (!webRequestParams.Schema.is_null() && webRequestParams.Schema.is_structured())
				{
					webValue = JSONFileManager::GetJSONValueBySchema(jsonData, webRequestParams.Schema);
				}
				else
				{
					//TODO
				}
			}
		}
	}
	else
	{
		if (webRequestParams.TextOnFail && !response.empty())
		{
			webValue = response;
		}
		//TODO
	}

	return { webRequestParams.DataType, webRequestParams.LuaName, webValue };
}
