#include "stdafx.h"

#include "JSONFileManager.h"
#include "MessageBoxManager.h"
#include "StringConverter.h"

bool JSONFileManager::TryParseJSONFile(std::wstring filename, std::wstring path, json* outData)
{
	std::wstring sanitizedName = FileUtils::ValidateFileName(filename, L".json");

	std::wstring fullFilePath = path + L'\\' + sanitizedName;
	std::wifstream file(fullFilePath.c_str(), std::ifstream::in | std::ifstream::binary);

	if (!file.is_open())
		return false;

	std::wstringstream tempFile;
	tempFile << file.rdbuf();

	file.close();

	return TryParseJSONData(tempFile.str(), outData);
}

template <typename T>
bool JSONFileManager::TryParseJSONData(T& inJSONString, json* outData, bool silentError)
{
	try {
		*outData = json::parse(inJSONString, nullptr, true, true);

		if (!outData->is_object())
		{
			if (!silentError)
			{
				MessageBoxManager::ShowMessageBox(L"The top level of the file does not contain an object", L"Failed to parse JSON");
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


template bool JSONFileManager::TryParseJSONData<std::string>(std::string&, json*, bool);
template bool JSONFileManager::TryParseJSONData<std::wstring>(std::wstring&, json*, bool);