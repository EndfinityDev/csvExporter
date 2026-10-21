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

		// 2. Convert the entire message block to a wstring
		std::wstring errorString = StringConverter::to_wstring(ss.str());
		if (!silentError)
		{
			MessageBoxManager::ShowMessageBox(errorString, L"Failed to parse JSON");
		}
		return false;
	}
}


template bool JSONFileManager::TryParseJSONData<std::string>(const std::string&, json*, bool, const JSONDataType);
template bool JSONFileManager::TryParseJSONData<std::wstring>(const std::wstring&, json*, bool, const JSONDataType);