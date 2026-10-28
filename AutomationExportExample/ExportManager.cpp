/************************************************* *
** Copyright(c) 2021 -- Camshaft Software PTY LTD
** All Rights Reserved
************************************************** */

#include "stdafx.h"
#include <fstream>
#include <sstream>

#include "StringConverter.h"
#include "FileUtils.h"
#include "StringUtils.h"
#include "MessageBoxManager.h"
#include "JSONFileManager.h"
#include "DirectoryManager.h"
#include "ExportPrefs.h"
#include "ExportCustomParamsManager.h"

const std::wstring AuExpManager::APPDATA_EXPORTER_DIR = L"\\AutomationGame\\CSVExporter 2";
const std::wstring AuExpManager::APPDATA_EXPORTER_PREFS_DIR = L"\\AutomationGame\\ExporterPrefs\\";
const std::wstring AuExpManager::s_IllegalCharacters = L"\\/:?\"<>|";

AuExpManager* AuExpManager::s_Instance = nullptr;

AuExpManager::AuExpManager() :
	m_ExportDirectory(L""),
	m_Module(GetModuleHandle(PROJECT_FILENAME))
{
	s_Instance = this;
}


AuCarExpErrorCode AuExpManager::Init(const AuCarExpCarData* carData)
{
	TCHAR path[MAX_PATH];

	m_LuaFiles.clear();

	m_CarData = carData;

	m_Delimiter = carData->GetStringData(ExporterStringData_CSVDelimiter)->Value;
	m_Locale = carData->GetStringData(ExporterStringData_Locale)->Value;
	m_RequireSameCSVHeader = carData->GetBoolData(ExporterBoolData_RequireSameHeader)->Value;

	//std::wstring mainDirectory = L"\\AutomationGame\\CSVExporter 2";

	//get the user's documents directory:
	if (SHGetFolderPathW(0, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_CURRENT, path) == S_OK)
	{
		m_ExportDirectory = path;
		m_ExportDirectory += APPDATA_EXPORTER_DIR + L"\\Exports";
	}
	else
	{
		return AuCarExpErrorCode_CouldNotObtainOutputPathFatal;
	}


	m_ExportDirectory += L"\\";

	// Sanitizing Filename
	std::wstring exportFileName = m_CarData->GetStringData(ExporterStringData_FileName)->Value;

	FileUtils::SanitizeFileNameIllegalCharacters(&exportFileName, s_IllegalCharacters);

	//m_ExportDirectory += m_CarData->GetStringData(0)->Value;
	m_FileName = exportFileName;
	m_ExportDirectory += exportFileName;

	if (!DirectoryManager::CheckPathIsDirectory(m_ExportDirectory))
	{
		if (!DirectoryManager::TryCreateDirectoryRecursive(m_ExportDirectory))
		{
			std::wstring error = L"Could not create directory: ";
			error += m_ExportDirectory;

			MessageBox(nullptr, error.c_str(), TEXT("Error creating directory"), MB_OK);

			return AuCarExpErrorCode_CouldNotObtainOutputPathFatal;
		}
	}

	ConfigureCustomParamData();
	BuildParamsLuaFile();

	std::wstring customFilePath = path + APPDATA_EXPORTER_DIR + L"\\UserFiles";
	std::wstring exportParamsPath = customFilePath + L"\\.ExportParams";

	if(!DirectoryManager::CheckPathIsDirectory(customFilePath)) // Loading from resources if UserFiles doesn't exist
	{
		if (DirectoryManager::TryCreateDirectoryIfNotExists(customFilePath))
		{
			if (!DirectoryManager::TryCreateDirectoryIfNotExists(exportParamsPath))
			{
				Logger::GetLogger()->Log(L"Failed to create ExportParams directory: " + exportParamsPath);
			}
		}
		else
		{
			Logger::GetLogger()->Log(L"Failed to create UserFiles directory: " + customFilePath);
		}

		if (!LoadLuaResource())
		{
			return AuCarExpErrorCode_UnknownError;
		}
		if (!LoadDataResource())
		{
			return AuCarExpErrorCode_UnknownError;
		}
		if (!LoadTranslationResource())
		{
			return AuCarExpErrorCode_UnknownError;
		}

		return AuCarExpErrorCode_Success;
	}

	if (!DirectoryManager::TryCreateDirectoryIfNotExists(exportParamsPath))
	{
		Logger::GetLogger()->Log(L"Failed to create ExportParams directory: " + exportParamsPath);
	}

	// Try to load user files (if given)

	std::wstring luaFileName = carData->GetStringData(ExporterStringData_LuaFileName)->Value;
	std::wstring dataFileName =	carData->GetStringData(ExporterStringData_DataJSONFileName)->Value;
	std::wstring translationsFileName = carData->GetStringData(ExporterStringData_TranslationsJSONFileName)->Value;

	bool hasValue = false;
	for (wchar_t ch : luaFileName)
	{
		if (ch != L' ')
		{
			hasValue = true;
			break;
		}
	}
	if (hasValue)
	{
		if (!LoadLuaFiles(luaFileName, customFilePath))
		{
			MessageBoxManager::ShowMessageBox(L"Failed to load Lua file", L"Error");
			return AuCarExpErrorCode_UnknownError;
		}
	}
	else
	{
		if (!LoadLuaResource())
		{
			return AuCarExpErrorCode_UnknownError;
		}
	}

	hasValue = false;
	for (wchar_t ch : dataFileName)
	{
		if (ch != L' ')
		{
			hasValue = true;
			break;
		}
	}
	if (hasValue)
	{
		if(!LoadDataFiles(dataFileName, customFilePath))
		{
			MessageBoxManager::ShowMessageBox(L"Failed to load dataset file", L"Error");
			return AuCarExpErrorCode_UnknownError;
		}
	}
	else
	{
		if (!LoadDataResource())
		{
			return AuCarExpErrorCode_UnknownError;
		}
	}

	hasValue = false;
	for (wchar_t ch : translationsFileName)
	{
		if (ch != L' ')
		{
			hasValue = true;
			break;
		}
	}
	if (hasValue)
	{
		if(!LoadTranslationFiles(translationsFileName, customFilePath))
		{
			MessageBoxManager::ShowMessageBox(L"Failed to load translation file", L"Error");
			return AuCarExpErrorCode_UnknownError;
		}
	}
	else
	{
		if (!LoadTranslationResource())
		{
			return AuCarExpErrorCode_UnknownError;
		}
	}

	return AuCarExpErrorCode_Success;
}

void AuExpManager::ConfigureCustomParamData()
{
	unsigned int stringParameterCount = m_CarData->GetStringDataCount() - ExporterStringData_Count;
	unsigned int boolParameterCount = m_CarData->GetBoolDataCount() - ExporterBoolData_Count;

	TCHAR path[MAX_PATH];
	if (SHGetFolderPathW(0, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_CURRENT, path) == S_OK)
	{
		std::wstring exportParamDir = path + APPDATA_EXPORTER_DIR + L"\\UserFiles\\.ExportParams";
		m_ExportCustomParamsGroup = ExportCustomParamsManager::LoadParameterJSONs(exportParamDir, false, false);
	}

	if (stringParameterCount != m_ExportCustomParamsGroup.StringParams.size())
	{
		Logger::GetLogger()->Log(L"[ExportManager] Encountered mismatch between received and stored export string parameter counts");
		return;
	}

	if (boolParameterCount != m_ExportCustomParamsGroup.BoolParams.size())
	{
		Logger::GetLogger()->Log(L"[ExportManager] Encountered mismatch between received and stored export boolean parameter counts");
		return;
	}

	for (unsigned int i = 0; i < stringParameterCount; i++)
	{
		//std::wstring value = StringUtils::SanitizeLuaString(m_CarData->GetStringData(ExporterStringData_Count + i)->Value);
		std::wstring newValue = m_CarData->GetStringData(ExporterStringData_Count + i)->Value;
		//if (newValue.size() >= CSVE_CUSTOM_PREFS_STR_MAX_LENGTH)
		//{
		//	continue;
		//}
		m_ExportCustomParamsGroup.StringParams[i].Value = newValue;
	}

	for (unsigned int i = 0; i < boolParameterCount; i++)
	{
		m_ExportCustomParamsGroup.BoolParams[i].Value = m_CarData->GetBoolData(ExporterBoolData_Count + i)->Value;
	}
}

void AuExpManager::BuildParamsLuaFile()
{
	unsigned int stringParameterCount = m_CarData->GetStringDataCount() - ExporterStringData_Count;
	unsigned int boolParameterCount = m_CarData->GetBoolDataCount() - ExporterBoolData_Count;

	std::stringstream paramsLuaStream;
	paramsLuaStream << "ExportParameters = {\n";
	paramsLuaStream << "\tStringParameters = {\n\t\t";

	for (unsigned int i = 0; i < stringParameterCount; i++)
	{
		std::wstring value = StringUtils::SanitizeLuaString(m_CarData->GetStringData(ExporterStringData_Count + i)->Value);

		if (i != 0)
		{
			paramsLuaStream << ",\n\t\t";
		}

		std::string escaped_lua_string = "";
		for (size_t j = 0; j < value.length(); ++j)
		{
			wchar_t c = value[j];
			if (c >= 0xD800 && c <= 0xDBFF && j + 1 < value.length()) { // High Surrogate
				wchar_t l = value[++j]; // Low Surrogate
				unsigned int cp = 0x10000 + ((c - 0xD800) << 10) + (l - 0xDC00);
				char buf[32];
				sprintf_s(buf, "\\u{%X}", cp);
				escaped_lua_string += buf;
			}
			else if (c > 127) { // Standard non-ASCII (Unicode BMP)
				char buf[32];
				sprintf_s(buf, "\\u{%X}", c);
				escaped_lua_string += buf;
			}
			else { // Standard safe ASCII character
				escaped_lua_string += (char)c;
			}
		}

		paramsLuaStream << StringConverter::to_utf8string(m_ExportCustomParamsGroup.StringParams[i].LuaName) << " = \"" << escaped_lua_string << "\"";
		//paramsLuaStream << StringConverter::to_utf8string(m_ExportCustomParamsGroup.StringParams[i].LuaName) << " = \"" << StringConverter::to_utf8string(value) << "\"";
	}
	paramsLuaStream << "\n\t},";

	paramsLuaStream << "\n\n\tBoolParameters = {\n\t\t";
	for (unsigned int i = 0; i < boolParameterCount; i++)
	{
		bool value = m_CarData->GetBoolData(ExporterBoolData_Count + i)->Value;

		if (i != 0)
		{
			paramsLuaStream << ",\n\t\t";
		}
		paramsLuaStream << StringConverter::to_utf8string(m_ExportCustomParamsGroup.BoolParams[i].LuaName) << " = " << (value ? "true" : "false");
	}
	paramsLuaStream << "\n\t}";
	paramsLuaStream << "\n}\n\0";

	m_LuaFiles.push_back(paramsLuaStream.str());
}

bool AuExpManager::LoadLuaResource()
{
	HRSRC   hRes;              // handle/ptr to res. info.
	HGLOBAL hResourceLoaded;  // handle to loaded resource
	hRes = FindResource(m_Module, MAKEINTRESOURCE(IDR_LUA_FILE), TEXT("BINARY"));

	if (!hRes)
	{
		return false;
	}

	unsigned int size = SizeofResource(m_Module, hRes);
	//size++; // Null terminator
	//m_LuaFileLength = size;
	//m_LuaFile.reserve(m_LuaFileLength);

	hResourceLoaded = LoadResource(m_Module, hRes);
	//m_LuaFile = (char*)LockResource(hResourceLoaded);
	m_LuaFiles.emplace_back((char*)LockResource(hResourceLoaded), size);

	UnlockResource(hResourceLoaded);
	return true;
}

bool AuExpManager::LoadLuaFiles(const std::wstring& fileNames, const std::wstring& path)
{
	std::vector<std::wstring> splitFileNames = StringUtils::SplitByCharW(fileNames, L',');
	if (splitFileNames.size() == 0)
	{
		return false;
	}

	for (const std::wstring& fileName : splitFileNames)
	{
		std::wstring trimmedFileName = StringUtils::Trim(fileName);
		if (trimmedFileName.empty())
		{
			continue;
		}
		if (trimmedFileName == L"*")
		{
			if (!LoadLuaResource())
			{
				return false;
			}
			continue;
		}
		if (!LoadLuaFile(trimmedFileName, path))
		{
			return false;
		}
	}

	return true;
}

bool AuExpManager::LoadLuaFile(const std::wstring& filename, const std::wstring& path)
{
	std::wstring sanitizedName = FileUtils::SanitizeFileNameWithExtension(filename, L".lua");

	std::wstring fullFilePath = path + L'\\' + sanitizedName;
	std::ifstream file(fullFilePath.c_str(), std::ifstream::in | std::ifstream::binary);

	if (!file.is_open())
		return false;

	std::stringstream tempLuaFile;
	tempLuaFile << file.rdbuf();
	//m_LuaFile = tempLuaFile.str();
	m_LuaFiles.push_back(tempLuaFile.str());

	file.close();

	return true;
}

bool AuExpManager::LoadDataResource()
{
	HRSRC   hRes;              // handle/ptr to res. info.
	HGLOBAL hResourceLoaded;  // handle to loaded resource
	hRes = FindResource(m_Module, MAKEINTRESOURCE(IDR_DATASET_FILE), TEXT("BINARY"));

	if (!hRes)
	{
		return false;
	}

	size_t size = SizeofResource(m_Module, hRes);
	size++; // Null terminator

	hResourceLoaded = LoadResource(m_Module, hRes);
	char* dataA = (char*)LockResource(hResourceLoaded);

	std::wstring dataW;
	dataW.resize(size * sizeof(wchar_t), 0);
	int result = MultiByteToWideChar(CP_UTF8, 0, dataA, strlen(dataA), &dataW[0], dataW.size());

	if (result <= 0)
	{
		throw new std::exception("Failed to convert resource to wide char. File: ExportManager.cpp, line: " + (__LINE__ - 4));
	}

	//return ParseDataJSON(dataW);

	json jsonData;
	if (!JSONFileManager::TryParseJSONData(dataW, &jsonData))
	{
		return false;
	}
	return LoadDataJSON(jsonData);
}

bool AuExpManager::LoadDataFiles(const std::wstring& fileNames, const std::wstring& path)
{
	std::vector<std::wstring> splitFileNames = StringUtils::SplitByCharW(fileNames, L',');
	if (splitFileNames.size() == 0)
	{
		return false;
	}

	for (const std::wstring& fileName : splitFileNames)
	{
		std::wstring trimmedFileName = StringUtils::Trim(fileName);
		if (trimmedFileName.empty())
		{
			continue;
		}
		if (trimmedFileName == L"*")
		{
			if (!LoadDataResource())
			{
				return false;
			}
			continue;
		}
		if (!LoadDataFile(trimmedFileName, path))
		{
			return false;
		}
	}

	return true;
}

bool AuExpManager::LoadDataFile(const std::wstring& filename, const std::wstring& path)
{
	json jsonData;
	if (!JSONFileManager::TryParseJSONFile(filename, path, &jsonData))
	{
		return false;
	}

	return LoadDataJSON(jsonData);
}

bool AuExpManager::LoadDataEntryFromJSON(const std::string& entryKey, const json& entryValue)
{
	if (!entryValue.is_object()) // Verify inner object
	{
		return false;
	}

	std::wstring luaName = StringConverter::to_wstring(entryKey);

	if (!(entryValue.contains("Translation") && entryValue["Translation"].is_string()))
	{
		return false;
	}
	auto& translationValue = entryValue["Translation"].get<std::string>(); // Inner object search by key
	std::wstring translation = StringConverter::to_wstring(translationValue);

	if (!(entryValue.contains("DataType") && entryValue["DataType"].is_string()))
	{
		return false;
	}
	auto& dataTypeValue = entryValue["DataType"].get<std::string>(); // Inner object search by key
	std::wstring dataTypeString = StringConverter::to_wstring(dataTypeValue);
	CSVDataType dataType = ParseDataType(dataTypeString);
	if (dataType == CSVDataType::UNKNOWN)
	{
		Logger::GetLogger()->Log(L"[ExportManager] Encountered unknown CSV data type: " + dataTypeString);
		return false;
	}

	bool doNotTranslate = false;
	if (entryValue.contains("DoNotTranslate") && entryValue["DoNotTranslate"].is_boolean())
	{
		doNotTranslate = entryValue["DoNotTranslate"].get<bool>();
	}

	m_LuaData.emplace_back(translation, luaName, dataType, doNotTranslate);
	return true;
}

bool AuExpManager::LoadDataEntriesFromJSON(const json& arrayObject)
{
	if (!arrayObject.is_object()) // Verify object
	{
		return false;
	}

	for (const auto& entryPair : arrayObject.items()) // Iterate over every item in the object, old implementation only took the first one
	{
		auto& entryKey = entryPair.key(); // Outer object key
		auto& entryValue = entryPair.value(); // Inner object
		if(!LoadDataEntryFromJSON(entryKey, entryValue))
		{
			return false;
		}
	}
	return true;
}

bool AuExpManager::LoadDataJSON(const json& jsonData)
{
	for (const auto& items : jsonData.items()) // Data, etc.
	{
		//auto& key = items.key();
		auto& value = items.value();

		bool isObject = value.is_object();

		if (!value.is_structured()) // Data, etc. as array or object
		{
			return false;
		}

		if (isObject)
		{
			if (value.contains("Translation") && value.contains("DataType"))
			{
				if (!LoadDataEntryFromJSON(items.key(), value))
				{
					return false;
				}
				continue;
			}
			if (!LoadDataEntriesFromJSON(value))
			{
				return false;
			}
		}
		else
		{
			for (const auto& arrayObject : value) // Object in array
			{
				if (!LoadDataEntriesFromJSON(arrayObject))
				{
					return false;
				}
			}
		}
	}
	return true;
}

bool AuExpManager::LoadTranslationResource()
{
	HRSRC   hRes;              // handle/ptr to res. info.
	HGLOBAL hResourceLoaded;  // handle to loaded resource
	hRes = FindResource(m_Module, MAKEINTRESOURCE(IDR_TRANSLATIONS_FILE), TEXT("BINARY"));

	if (!hRes)
	{
		return false;
	}

	size_t size = SizeofResource(m_Module, hRes);
	size++; // Null terminator

	hResourceLoaded = LoadResource(m_Module, hRes);
	char* dataA = (char*)LockResource(hResourceLoaded);

	std::wstring dataW;
	dataW.resize(size * sizeof(wchar_t), 0);
	int result = MultiByteToWideChar(CP_UTF8, 0, dataA, strlen(dataA), &dataW[0], dataW.size());

	if (result <= 0)
	{
		throw new std::exception("Failed to convert resource to wide char. File: ExportManager.cpp, line: " + (__LINE__ - 4));
	}

	json jsonData;
	if (!JSONFileManager::TryParseJSONData(dataW, &jsonData))
	{
		return false;
	}
	return LoadTranslationJSON(jsonData);
}

bool AuExpManager::LoadTranslationFiles(const std::wstring& fileNames, const std::wstring& path)
{
	std::vector<std::wstring> splitFileNames = StringUtils::SplitByCharW(fileNames, L',');
	if (splitFileNames.size() == 0)
	{
		return false;
	}

	for (const std::wstring& fileName : splitFileNames)
	{
		std::wstring trimmedFileName = StringUtils::Trim(fileName);
		if (trimmedFileName.empty())
		{
			continue;
		}
		if (trimmedFileName == L"*")
		{
			if (!LoadTranslationResource())
			{
				return false;
			}
			continue;
		}
		if (!LoadTranslationFile(trimmedFileName, path))
		{
			return false;
		}
	}

	return true;
}

bool AuExpManager::LoadTranslationFile(const std::wstring& filename, const std::wstring& path)
{
	json jsonData;
	if (!JSONFileManager::TryParseJSONFile(filename, path, &jsonData))
	{
		return false;
	}

	return LoadTranslationJSON(jsonData);
}

bool AuExpManager::LoadTranslationJSON(const json& jsonData)
{
	for (const auto& items : jsonData.items()) // Translations, etc.
	{
		const auto& value = items.value();

		if (value.is_string())
		{
			auto& key = items.key();
			std::wstring keyWString = StringConverter::to_wstring(key);
			auto& valueString = value.get<std::string>();
			std::wstring valueWString = StringConverter::to_wstring(valueString);

			m_Translations.emplace(keyWString, valueWString);
			continue;
		}

		if (!value.is_object())
		{
			return false;
		}

		for (const auto& translationPair : value.items())
		{
			auto& key = translationPair.key();
			auto& value = translationPair.value();
			if (!value.is_string())
			{
				return false;
			}
			std::wstring keyWString = StringConverter::to_wstring(key);
			auto& valueString = value.get<std::string>();
			std::wstring valueWString = StringConverter::to_wstring(valueString);

			m_Translations.emplace(keyWString, valueWString);
		}
	}
	return true;
}

void AuExpManager::EndExport() 
{
	m_IsExportInProcess = true;

	//PopulateExportDataHeader();

	ExportInternal();

	m_IsExportInProcess = false;
}

void AuExpManager::ExportInternal()
{
	m_LuaFiles.clear();

	BuildSingleCSV();

	ExportPrefs::TryWriteToFile(m_CarData, m_Locale);
	//ExportCustomBoolParamPrefs::TryWriteToFile(ExportCustomBoolParamPrefs::FromExportParams(m_ExportCustomParamsGroup.BoolParams), APPDATA_EXPORTER_PREFS_DIR);
	ExportCustomParamsManager::TrySaveParamsGroup(m_ExportCustomParamsGroup, APPDATA_EXPORTER_PREFS_DIR);
}


void AuExpManager::BuildSingleCSV()
{
	CSVManager csvManager(m_ExportDirectory + L"\\" + m_FileName + L".csv", m_Delimiter, m_RequireSameCSVHeader);

	//csvManager.BindData(&m_LuaData);
	csvManager.Save(m_LuaData);
}


void AuExpManager::AddImage(const AuCarExpTexture* texture)
{
	if (!texture)
	{
		return;
	}

	//only add unique textures
	for (size_t i = 0; i < m_AllImages.size(); i++)
	{
		if (m_AllImages[i] == texture)
		{
			return;
		}
	}

	m_AllImages.push_back(texture);
}

void AuExpManager::SaveImages()
{
	for (size_t i = 0; i < m_AllImages.size(); i++)
	{
		AuExpImage::SaveImageFile(m_AllImages[i], m_ExportDirectory.c_str());
	}
}

void AuExpManager::SaveMesh(const AuCarExpMesh* mesh, const wchar_t* name)
{
	return;
}

const wchar_t* AuExpManager::GetExportDirectory()
{
	TCHAR path[MAX_PATH];
	if (SHGetFolderPathW(0, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_CURRENT, path) == S_OK)
	{
		std::wstring exportDirectory = path + APPDATA_EXPORTER_DIR + L"\\Exports";
		return exportDirectory.c_str();
	}
	else
	{
		return L"";
	}
}

void AuExpManager::AddLuaFloatData(const AuCarExpArray<AuCarExpLuaFloatData>& Data)
{
	if (Data.GetCount() > 0)
		m_LuaDataAdded = true;

	for (size_t i = 0; i < Data.GetCount(); i++)
	{
		m_LuaFloatData[Data[i].ValueName] = Data[i].Value;
	}
}

void AuExpManager::AddLuaStringData(const AuCarExpArray<AuCarExpLuaStringData>& Data)
{
	if (Data.GetCount() > 0)
		m_LuaDataAdded = true;

	for (size_t i = 0; i < Data.GetCount(); i++)
	{
		m_LuaStringData[Data[i].ValueName] = Data[i].Buffer;
	}
}

bool AuExpManager::TryGetFloatData(const std::wstring& key, float* retValue)
{
	if (m_LuaFloatData.find(key) == m_LuaFloatData.end())
	{
		return false;
	}

	*retValue = m_LuaFloatData[key];
	return true;
}

bool AuExpManager::TryGetFloatDataString(const std::wstring& key, std::wstring* retValue)
{
	float val = 0.0f;
	if (AuExpManager::GetInstance()->TryGetFloatData(key, &val))
	{
		wchar_t valStr[64];
		valStr[63] = '\0';

		swprintf_s(valStr, 63, L"%f", val);
		*retValue = valStr;

		return true;
	}

	return false;
}

bool AuExpManager::TryGetStringData(const std::wstring& key, std::wstring& value)
{
	if (m_LuaStringData.find(key) == m_LuaStringData.end())
	{
		return false;
	}

	value = m_LuaStringData[key];
	return true;
}

void AuExpManager::AddLuaFiles(const AuCarExpArray<AuCarLuaDataFile>& data)
{
	size_t fileCount = data.GetCount();

	std::locale locale(std::locale(std::locale::classic(), new std::codecvt_utf8_utf16<wchar_t>));
	if (!m_Locale.empty())
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
		std::string localeNameA = converter.to_bytes(m_Locale);
		try
		{
			std::locale baseLocale(localeNameA);
			//std::locale localeWithUTF16(baseLocale, new std::codecvt_utf8_utf16<wchar_t>);
			locale = std::locale(baseLocale, new std::codecvt_utf8_utf16<wchar_t>);
			//outputFile.imbue(localeWithUTF16);
		}
		catch (const std::runtime_error&)
		{
			//outputFile.imbue(std::locale(std::locale::classic(), new std::codecvt_utf8_utf16<wchar_t>));
			//locale = std::locale(std::locale::classic(), new std::codecvt_utf8_utf16<wchar_t>);
			Logger::GetLogger()->Log(L"[ExportManager] Failed to set Lua file locale, using default");
		}
	}

	for (size_t i = 0; i < fileCount; i++)
	{
		std::ofstream file(m_ExportDirectory + L"\\" + data[i].FileName, std::ios::trunc | std::ios::binary);
		file.imbue(locale);
		file.write(data[i].Buffer, data[i].BufferSize);
		file.close();
	}

}

void AuExpManager::GetTranslation(std::wstring* ret)
{
	if (m_Translations.find(*ret) == m_Translations.end())
		return;

	*ret = m_Translations[*ret];
}