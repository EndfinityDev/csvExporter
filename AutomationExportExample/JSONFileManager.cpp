#include "stdafx.h"

#include "JSONFileManager.h"
#include "MessageBoxManager.h"
#include "StringConverter.h"

bool JSONFileManager::TryParseJSONFile(std::wstring filename, std::wstring path, json* outData, 
										const bool silentError, const JSONDataType topLevelType)
{
	std::wstring sanitizedName = FileUtils::SanitizeFileNameWithExtension(filename, L".json");

	std::wstring fullFilePath = path + L'\\' + sanitizedName;
	std::wifstream file(fullFilePath.c_str(), std::ifstream::in | std::ifstream::binary);

	if (!file.is_open())
		return false;

	std::wstringstream tempFile;
	tempFile << file.rdbuf();

	file.close();

	return TryParseJSONData(tempFile.str(), outData, silentError, topLevelType);
}

template <typename T>
bool JSONFileManager::TryParseJSONData(const T& inJSONString, json* outData, const bool silentError, const JSONDataType topLevelType)
{
	try {
		*outData = json::parse(inJSONString, nullptr, true, true);

		if ((topLevelType == JSONDataType::OBJECT && !outData->is_object()) || (topLevelType == JSONDataType::ARRAY && !outData->is_array()))
		{
			if (!silentError)
			{
				MessageBoxManager::ShowMessageBox(
					topLevelType == JSONDataType::OBJECT ? L"The top level of the file does not contain an object" :
					L"The top level of the file does not contain an array",
					L"Failed to parse JSON");
			}
			Logger::GetLogger()->Log(L"[JSONFileManager] Failed to parse JSON");
			Logger::GetLogger()->Log(
				topLevelType == JSONDataType::OBJECT ? L"The top level of the file does not contain an object" :
				L"The top level of the file does not contain an array");
			return false;
		}

		return true;
	}
	catch (const json::parse_error& e) {
		std::stringstream ss;
		ss << "--- JSON Parse Failure ---\n"
			<< "Error ID:  " << e.id << "\n"
			<< "Message:   " << e.what() << "\n"
			<< "At Byte:   " << e.byte << "\n"
			<< "------------------------------------";

		std::wstring errorString = StringConverter::to_wstring(ss.str());
		Logger::GetLogger()->Log(L"[JSONFileManager] Failed to parse JSON");
		Logger::GetLogger()->Log(errorString);
		if (!silentError)
		{
			MessageBoxManager::ShowMessageBox(errorString, L"Failed to parse JSON");
		}
		return false;
	}
}

json JSONFileManager::GetJSONValueBySchema(const json& data, const json& schema)
{
	if (data.type() == schema.type())
	{
		if (data.is_object())
		{
			return TraverseJSONDictBySchema(data, schema);
		}
		if (data.is_array())
		{
			return TraverseJSONListBySchema(data, schema);
		}
	}

	return json();
}

json JSONFileManager::TraverseJSONDictBySchema(const json& dataDict, const json& schemaDict)
{
	for (const auto& schemaItems : schemaDict.items())
	{
		const auto& schemaKey = schemaItems.key();
		const auto& schemaValue = schemaItems.value();
		if (dataDict.contains(schemaKey))
		{
			const json& dataValue = dataDict[schemaKey];
			if (dataValue.is_structured() && schemaValue.is_structured() && (dataValue.type() == schemaValue.type()))
			{
				if (dataValue.is_object())
				{
					json returnValue = TraverseJSONDictBySchema(dataValue, schemaValue);
					if (returnValue.is_null())
					{
						continue;
					}
					return returnValue;
				}
				
				json returnValue = TraverseJSONListBySchema(dataValue, schemaValue);
				if (returnValue.is_null())
				{
					continue;
				}
				return returnValue;
			}

			if (schemaValue.is_string())
			{
				std::string expectedTypeString = schemaValue.get<std::string>();
				JSONSchemaResultType expectedType = JSONSchemaResultType::NONE;
				auto& iterator = JSONSchemaResultTypeMap.find(expectedTypeString);
				if (iterator != JSONSchemaResultTypeMap.end())
				{
					expectedType = iterator->second;
				}

				if (expectedType == JSONSchemaResultType::NONE)
				{
					continue;
				}

				if ((dataValue.is_string() && static_cast<int>(expectedType & JSONSchemaResultType::STRING)) ||
					(dataValue.is_number_float() && static_cast<int>(expectedType & JSONSchemaResultType::FLOAT)) ||
					(dataValue.is_number_integer() && static_cast<int>(expectedType & JSONSchemaResultType::INT)) ||
					(dataValue.is_boolean() && static_cast<int>(expectedType & JSONSchemaResultType::BOOL)))
				{
					return dataValue;
				}
			}
			else
			{
				Logger::GetLogger()->Log(L"[JSONFileManager] Schema target value is not a string. Key: " + StringConverter::to_wstring(schemaKey));
			}
		}
	}
	return json();
}

json JSONFileManager::TraverseJSONListBySchema(const json& dataList, const json& schemaList)
{
	size_t schemaListSize = schemaList.size();

	if (schemaListSize > dataList.size())
	{
		return json();
	}

	for (size_t i = 0; i < schemaListSize ; i++)
	{
		const auto& schemaItem = schemaList[i];
		if (schemaItem.is_null())
		{
			continue;
		}

		const auto& dataItem = dataList[i];

		if (dataItem.is_structured() && schemaItem.is_structured() && (dataItem.type() == schemaItem.type()))
		{
			if (dataItem.is_object())
			{
				json returnValue = TraverseJSONDictBySchema(dataItem, schemaItem);
				if (returnValue.is_null())
				{
					continue;
				}
				return returnValue;
			}

			json returnValue = TraverseJSONListBySchema(dataItem, schemaItem);
			if (returnValue.is_null())
			{
				continue;
			}
			return returnValue;
		}

		if (schemaItem.is_string())
		{
			std::string expectedTypeString = schemaItem.get<std::string>();
			JSONSchemaResultType expectedType = JSONSchemaResultType::NONE;
			auto& iterator = JSONSchemaResultTypeMap.find(expectedTypeString);
			if (iterator != JSONSchemaResultTypeMap.end())
			{
				expectedType = iterator->second;
			}

			if (expectedType == JSONSchemaResultType::NONE)
			{
				continue;
			}

			if ((dataItem.is_string() && static_cast<int>(expectedType & JSONSchemaResultType::STRING)) ||
				(dataItem.is_number_float() && static_cast<int>(expectedType & JSONSchemaResultType::FLOAT)) ||
				(dataItem.is_number_integer() && static_cast<int>(expectedType & JSONSchemaResultType::INT)) ||
				(dataItem.is_boolean() && static_cast<int>(expectedType & JSONSchemaResultType::BOOL)))
			{
				return dataItem;
			}
		}
		else
		{
			Logger::GetLogger()->Log(L"[JSONFileManager] Schema target array value is not a string");
		}
	}
	return json();
}


template bool JSONFileManager::TryParseJSONData<std::string>(const std::string&, json*, bool, const JSONDataType);
template bool JSONFileManager::TryParseJSONData<std::wstring>(const std::wstring&, json*, bool, const JSONDataType);