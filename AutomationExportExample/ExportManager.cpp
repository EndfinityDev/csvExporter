/************************************************* *
** Copyright(c) 2021 -- Camshaft Software PTY LTD
** All Rights Reserved
************************************************** */

#include "stdafx.h"
#include <fstream>
#include <sstream>

#include "StringConverter.h"
#include "FileUtils.h"
#include "MessageBoxManager.h"
#include "JSONFileManager.h"


size_t FindDirDelimiter(std::wstring dir, size_t start)
{
	size_t slashPos = dir.find(L"\\", start);

	if (slashPos == std::wstring::npos)
	{
		return dir.find(L"/", start);
	}

	return slashPos;
}

const std::wstring AuExpManager::s_IllegalCharacters = L"\\/:?\"<>|";


AuExpManager* AuExpManager::s_Instance = nullptr;

AuExpManager::AuExpManager() :
	m_ExportDirectory(L""),
	m_Module(GetModuleHandle(PROJECT_FILENAME))
{
	s_Instance = this;

	//PopulateExportDataHeader();

	//BuildTranslations();
}


AuCarExpErrorCode AuExpManager::Init(const AuCarExpCarData* carData)
{
	TCHAR path[MAX_PATH];

	m_CarData = carData;

	m_Delimiter = carData->GetStringData(ExporterStringData_CSVDelimiter)->Value;
	m_Locale = carData->GetStringData(ExporterStringData_Locale)->Value;

	std::wstring mainDirectory = L"\\AutomationGame\\CSVExporter 2";

	//get the user's documents directory:
	if (SHGetFolderPathW(0, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_CURRENT, path) == S_OK)
	{
		m_ExportDirectory = path;
		m_ExportDirectory += mainDirectory + L"\\Exports";
	}
	else
	{
		return AuCarExpErrorCode_CouldNotObtainOutputPathFatal;
	}


	m_ExportDirectory += L"\\";
	//m_ExportDirectory += carData->GetCarName();//TODO: sanitise filename

	// Sanitizing Filename
	std::wstring exportFileName = m_CarData->GetStringData(ExporterStringData_FileName)->Value;

	for (std::wstring::iterator i = exportFileName.begin(); i < exportFileName.end(); i++)
	{
		if (s_IllegalCharacters.find(*i) != std::wstring::npos)
		{
			*i = L'_';
		}
	}

	if (exportFileName.size() > 1)
		while (exportFileName[exportFileName.size() - 1] == L' ')
		{
			exportFileName.resize(exportFileName.size() - 1);

			if (exportFileName.size() < 2) 
			{
				if (exportFileName[0] == L' ')
					exportFileName = L"_";

				break;
			}
		}
	else
		if (exportFileName[0] == L' ')
			exportFileName = L"_";

	//m_ExportDirectory += m_CarData->GetStringData(0)->Value;
	m_FileName = exportFileName;
	m_ExportDirectory += exportFileName;

	//ensure target directory exists:
	DWORD att = GetFileAttributes(m_ExportDirectory.c_str());

	if (att == INVALID_FILE_ATTRIBUTES)
	{
		//create directory, one level at a time:
		size_t slashPos = FindDirDelimiter(m_ExportDirectory, 0);
		size_t offset = 0;

		while (slashPos != std::wstring::npos)
		{
			CreateDirectory(m_ExportDirectory.substr(offset, slashPos - offset).c_str(), nullptr);
			slashPos = FindDirDelimiter(m_ExportDirectory, slashPos + 1);
		}

		//last one:
		CreateDirectory(m_ExportDirectory.c_str(), nullptr);

		att = GetFileAttributes(m_ExportDirectory.c_str());
	}

	if (att == INVALID_FILE_ATTRIBUTES || !(att & FILE_ATTRIBUTE_DIRECTORY))
	{
		std::wstring error = L"Could not create directory: ";
		error += m_ExportDirectory;

		MessageBox(nullptr, error.c_str(), TEXT("Error creating directory"), MB_OK);

		return AuCarExpErrorCode_CouldNotObtainOutputPathFatal;
	}



	std::wstring customFilePath = path + mainDirectory + L"\\UserFiles";

	att = GetFileAttributes(customFilePath.c_str());
	if (att == INVALID_FILE_ATTRIBUTES || !(att & FILE_ATTRIBUTE_DIRECTORY)) // Go straight to loading resources if UserFiles doesn't exist
	{
		LoadLuaResource();
		LoadDataResource();
		LoadTranslationResource();

		return AuCarExpErrorCode_Success;
	}

	// Try to load user files (if given)

	std::wstring luaFileName = carData->GetStringData(ExporterStringData_LuaFileName)->Value; // TODO: support multiple lua files
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
		if (!LoadLuaFile(luaFileName, customFilePath))
		{
			MessageBoxManager::ShowMessageBox(L"Failed to load Lua file", L"Error");
			return AuCarExpErrorCode_UnknownError;
		}
	}
	else
	{
		// TODO: load resource lua file
		LoadLuaResource();
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
		if(!LoadDataFile(dataFileName, customFilePath))
		{
			MessageBoxManager::ShowMessageBox(L"Failed to load dataset file", L"Error");
			return AuCarExpErrorCode_UnknownError;
		}
	}
	else
	{
		LoadDataResource();
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
		if(!LoadTranslationFile(translationsFileName, customFilePath))
		{
			MessageBoxManager::ShowMessageBox(L"Failed to load translation file", L"Error");
			return AuCarExpErrorCode_UnknownError;
		}
	}
	else
	{
		LoadTranslationResource();
	}

	return AuCarExpErrorCode_Success;
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
	size++; // Null terminator
	m_LuaFileLength = size;
	//m_LuaFile.reserve(m_LuaFileLength);

	hResourceLoaded = LoadResource(m_Module, hRes);
	m_LuaFile = (char*)LockResource(hResourceLoaded);

	UnlockResource(hResourceLoaded);
	return true;
}

bool AuExpManager::LoadLuaFile(std::wstring filename, std::wstring path)
{
	std::wstring sanitizedName = FileUtils::ValidateFileName(filename, L".lua");

	std::wstring fullFilePath = path + L'\\' + sanitizedName;
	std::ifstream file(fullFilePath.c_str(), std::ifstream::in | std::ifstream::binary);

	if (!file.is_open())
		return false;

	// get the length of the file
	//file.seekg(0, std::ifstream::end);
	//m_LuaFileLength = file.tellg();
	//file.seekg(0, std::ifstream::beg);

	//file >> m_LuaFile;
	std::stringstream tempLuaFile;
	tempLuaFile << file.rdbuf();
	m_LuaFile = tempLuaFile.str();

	file.close();

	m_LuaFileLength = m_LuaFile.size();

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
	dataW.resize(size * 2, 0);
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

bool AuExpManager::LoadDataFile(std::wstring filename, std::wstring path)
{
	json jsonData;
	if (!JSONFileManager::TryParseJSONFile(filename, path, &jsonData))
	{
		return false;
	}

	return LoadDataJSON(jsonData);
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
		DataType dataType = ParseDataType(dataTypeString);

		bool doNotTranslate = false;
		if (entryValue.contains("DoNotTranslate") && entryValue["DataType"].is_boolean())
		{
			doNotTranslate = entryValue["DoNotTranslate"].get<bool>();
		}

		m_LuaData.emplace_back(translation, luaName, dataType);
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

		if (!(value.is_array() || isObject)) // Data, etc. as array or object
		{
			return false;
		}

		if (isObject)
		{
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
	dataW.resize(size * 2, 0);
	int result = MultiByteToWideChar(CP_UTF8, 0, dataA, strlen(dataA), &dataW[0], dataW.size());

	if (result <= 0)
	{
		throw new std::exception("Failed to convert resource to wide char. File: ExportManager.cpp, line: " + (__LINE__ - 4));
	}

	//return ParseTranslationJSON(dataW);

	json jsonData;
	if (!JSONFileManager::TryParseJSONData(dataW, &jsonData))
	{
		return false;
	}
	return LoadTranslationJSON(jsonData);
}

bool AuExpManager::LoadTranslationFile(std::wstring filename, std::wstring path)
{
	json jsonData;
	if (!JSONFileManager::TryParseJSONFile(filename, path, &jsonData))
	{
		return false;
	}

	return LoadTranslationJSON(jsonData);

	/*
	std::wstring sanitizedName = FileUtils::ValidateFileName(filename, L".json");

	std::wstring fullFilePath = path + L'\\' + sanitizedName;
	std::wifstream file(fullFilePath.c_str(), std::ifstream::in | std::ifstream::binary);

	if (!file.is_open())
		return false;

	// get the length of the file
	//file.seekg(0, std::ifstream::end);
	//m_LuaFileLength = file.tellg();
	//file.seekg(0, std::ifstream::beg);

	//file >> m_LuaFile;
	std::wstringstream tempFile;
	tempFile << file.rdbuf();

	file.close();

	return ParseTranslationJSON(tempFile.str());
	*/
}

bool AuExpManager::LoadTranslationJSON(const json& jsonData)
{
	for (const auto& items : jsonData.items()) // Translations, etc.
	{
		const auto& value = items.value();
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
	m_LuaFile = ""; // Free memory of the Lua file since we don't need it anymore
	BuildSingleCSV();

	TCHAR path[MAX_PATH];

	std::wstring prefsDir;

	//get the user's documents directory:
	if (SHGetFolderPathW(0, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_CURRENT, path) == S_OK)
	{
		prefsDir = path;
		prefsDir += L"\\AutomationGame\\ExporterPrefs\\";

		DWORD att = GetFileAttributes(prefsDir.c_str());

		if (att == INVALID_FILE_ATTRIBUTES)
		{
			//create directory, one level at a time:
			size_t slashPos = FindDirDelimiter(prefsDir, 0);
			size_t offset = 0;

			while (slashPos != std::wstring::npos)
			{
				CreateDirectory(prefsDir.substr(offset, slashPos - offset).c_str(), nullptr);
				slashPos = FindDirDelimiter(prefsDir, slashPos + 1);
			}

			//last one:
			CreateDirectory(prefsDir.c_str(), nullptr);

			att = GetFileAttributes(prefsDir.c_str());
		}

		if (att == INVALID_FILE_ATTRIBUTES || !(att & FILE_ATTRIBUTE_DIRECTORY))
		{
			return;
		}

		std::wstring prefsFile = prefsDir + L"CSVExporter2.prefs";

		FILE* file;
		errno_t err = _wfopen_s(&file, prefsFile.c_str(), L"wb");

		if (err || !file)
			return;

		ExportPrefs prefs;

		std::wstring delimiter = m_CarData->GetStringData(ExporterStringData_CSVDelimiter)->Value;
		if (delimiter.size() == 1)
		{
			prefs.Delimiter = delimiter[0];
		}

		std::wstring luaFile = m_CarData->GetStringData(ExporterStringData_LuaFileName)->Value;
		if (luaFile.size() < PREFS_FILE_MAX_LENGTH)
		{
			for (size_t i = 0; i < luaFile.size(); i++)
			{
				prefs.LuaFile[i] = luaFile[i];
			}
		}

		std::wstring dataFile = m_CarData->GetStringData(ExporterStringData_DataJSONFileName)->Value;
		if (dataFile.size() < PREFS_FILE_MAX_LENGTH)
		{
			for (size_t i = 0; i < dataFile.size(); i++)
			{
				prefs.DataFile[i] = dataFile[i];
			}
		}

		std::wstring translationsFile = m_CarData->GetStringData(ExporterStringData_TranslationsJSONFileName)->Value;
		if (translationsFile.size() < PREFS_FILE_MAX_LENGTH)
		{
			for (size_t i = 0; i < translationsFile.size(); i++)
			{
				prefs.TranslationsFile[i] = translationsFile[i];
			}
		}

		std::wstring localeName = m_Locale;
		if (localeName.size() < PREFS_LOCALE_MAX_LENGTH)
		{
			for (size_t i = 0; i < localeName.size(); i++)
			{
				prefs.Locale[i] = localeName[i];
			}
		}

		fwrite(&prefs, sizeof(prefs), 1, file);
		fclose(file);
	}
}


void AuExpManager::BuildSingleCSV()
{
	//LuaData exportData;
	//exportData.Add(m_GeneralData);

	//FillTableData(exportData);

	CSVManager csvManager(m_ExportDirectory + L"\\" + m_FileName + L".csv", m_Delimiter);

	csvManager.BindData(&m_LuaData);
	csvManager.Save();
}


void AuExpManager::AddImage(const AuCarExpTexture* texture)
{
	if (!texture)
	{
		return;
	}

	//only add unique textures
	for (unsigned int i = 0; i < m_AllImages.size(); i++)
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
	for (unsigned int i = 0; i < m_AllImages.size(); i++)
	{
		AuExpImage::SaveImageFile(m_AllImages[i], m_ExportDirectory.c_str());
	}
}

void AuExpManager::SaveMesh(const AuCarExpMesh* mesh, const wchar_t* name)
{
	return;
}

void AuExpManager::AddLuaFloatData(const AuCarExpArray<AuCarExpLuaFloatData>& Data)
{
	if (Data.GetCount() > 0)
		m_LuaDataAdded = true;

	for (int i = 0; i < Data.GetCount(); i++)
	{
		m_LuaFloatData[Data[i].ValueName] = Data[i].Value;
	}
}

void AuExpManager::AddLuaStringData(const AuCarExpArray<AuCarExpLuaStringData>& Data)
{
	if (Data.GetCount() > 0)
		m_LuaDataAdded = true;

	for (int i = 0; i < Data.GetCount(); i++)
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


	for (size_t i = 0; i < fileCount; i++)
	{
		std::ofstream file(m_ExportDirectory + L"\\" + data[i].FileName);
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