/************************************************* *
** Copyright(c) 2021 -- Camshaft Software PTY LTD
** All Rights Reserved
************************************************** */

#include "stdafx.h"
#include <Shlobj.h>
#include <fstream>
#include <sstream>

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
			ShowMessageBox(L"Failed to load Lua file", L"Error");
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
			ShowMessageBox(L"Failed to load dataset file", L"Error");
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
			ShowMessageBox(L"Failed to load translation file", L"Error");
			return AuCarExpErrorCode_UnknownError;
		}
	}
	else
	{
		LoadTranslationResource();
	}

	return AuCarExpErrorCode_Success;
}

std::wstring AuExpManager::ValidateFileName(std::wstring filename, std::wstring defaultExtension)
{
	size_t filenameSize = filename.size();

	for (size_t i = filenameSize; i > 0; i--)
	{
		if (filename[i - 1] != L' ')
			break;
		filename[i - 1] = L'\0'; // Remove trailing whitespaces by replacing them with a null terminator
	}

	std::wstring sanitizedName = L"";
	bool startOfString = true;

	for (size_t i = 0; i < filenameSize; i++)
	{
		wchar_t currentChar = filename[i];

		if (currentChar == L' ' && startOfString)
			continue;
		else
			startOfString = false;

		//if (currentChar == L'\\' || currentChar == L'\'' || currentChar == L'"' || currentChar == L'/')
		//	return false;

		sanitizedName += currentChar;
	}

	bool hasExtension = false;
	for (size_t i = 1; i < sanitizedName.size(); i++)
	{
		if (sanitizedName[i] == L'.')
		{
			hasExtension = true;
			break;
		}
	}

	if (!hasExtension)
	{
		sanitizedName += defaultExtension;
	}

	return sanitizedName;
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
	//const char* tempRes = (char*)LockResource(hResourceLoaded);
	m_LuaFile = (char*)LockResource(hResourceLoaded);

	//for (size_t i = 0; i < m_LuaFileLength - 1; i++)
	//{
	//	m_LuaFile += tempRes[i];
	//}
	//m_LuaFile += "\0";

	//std::wstring wdata;
	//wdata.resize(size * 2, 0);
	//int result = MultiByteToWideChar(CP_UTF8, 0, data, strlen(data), &wdata[0], wdata.size());

	//if (result <= 0)
	//{
	//	throw new std::exception("Failed to convert resource to wide char. File: ExportManager.cpp, line: " + __LINE__);
	//}

	/*
	if ((size + 1) <= stringBuffer.GetCount())
	{
		for (unsigned int i = 0; i < size; i++)
		{
			stringBuffer[i] = data[i];
		}

		stringBuffer[size] = '\0';
	}
	*/

	UnlockResource(hResourceLoaded);

	//m_LuaFile = wdata;

	return true;
}

bool AuExpManager::LoadLuaFile(std::wstring filename, std::wstring path)
{
	std::wstring sanitizedName = ValidateFileName(filename, L".lua");

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

	return ParseDataJSON(dataW);
}

bool AuExpManager::LoadDataFile(std::wstring filename, std::wstring path)
{
	std::wstring sanitizedName = ValidateFileName(filename, L".json");

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

	return ParseDataJSON(tempFile.str());
}

bool AuExpManager::ParseDataJSON(std::wstring json)
{
	//efc::CJSONReturnStatus status(efc::CJSONReturnStatus::Code::OK);
	//std::wstring errorMessage = L"";
	efc::CJSONResult jresult = efc::CJSONReader::FromString(json);
	const efc::CJSONDict* dict = jresult.Dictionary;

	if (jresult.Status.GetCode() != efc::CJSONReturnStatus::Code::OK)
	{
		ShowMessageBox(jresult.ErrorMessage, L"Failed to parse JSON");
		return false;
	}

	const std::unordered_map<efc::CJString, efc::CJSONObject*>* dictMap = dict->GetMap();

	efc::CJSONObject* tempObj;
	for (const auto& pair : *dictMap)
	{
		tempObj = pair.second;
		if (tempObj->GetType() != efc::CJSONType::Array)
		{
			return false;
		}

		efc::CJSONList* tempList = (efc::CJSONList*)tempObj;
		std::vector<efc::CJSONObject*>* tempListVector = tempList->GetList();

		for (size_t i = 0; i < tempListVector->size(); i++)
		{
			tempObj = tempListVector->at(i);
			if (tempObj->GetType() != efc::CJSONType::Dictionary)
			{
				return false;
			}

			efc::CJSONDict* tempDict = (efc::CJSONDict*)tempObj;

			auto& innerPair = tempDict->GetMap()->begin();
			std::wstring luaName = innerPair->first;
			efc::CJSONDict* innerDict = (efc::CJSONDict*)innerPair->second;
			std::wstring translation = innerDict->Get<efc::CJSONString>(L"Translation")->GetValue();
			std::wstring typeString = innerDict->Get<efc::CJSONString>(L"DataType")->GetValue();
			DataType dataType = ParseDataType(typeString);
			m_LuaData.push_back(LuaDataHeader(translation, luaName, dataType));
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

	return ParseTranslationJSON(dataW);
}

bool AuExpManager::LoadTranslationFile(std::wstring filename, std::wstring path)
{
	std::wstring sanitizedName = ValidateFileName(filename, L".json");

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
}

bool AuExpManager::ParseTranslationJSON(std::wstring json)
{
	//efc::CJSONReturnStatus status(efc::CJSONReturnStatus::Code::OK);
	//std::wstring errorMessage = L"";
	efc::CJSONResult jresult = efc::CJSONReader::FromString(json);
	const efc::CJSONDict* dict = jresult.Dictionary;

	if (jresult.Status.GetCode() != efc::CJSONReturnStatus::Code::OK)
	{
		ShowMessageBox(jresult.ErrorMessage, L"Failed to parse JSON");
		return false;
	}

	const std::unordered_map<efc::CJString, efc::CJSONObject*>* dictMap = dict->GetMap();

	efc::CJSONObject* tempObj;
	for (const auto& pair : *dictMap)
	{
		tempObj = pair.second;
		if (tempObj->GetType() != efc::CJSONType::Dictionary)
		{
			return false;
		}

		efc::CJSONDict* tempDict = (efc::CJSONDict*)tempObj;
		std::unordered_map<efc::CJString, efc::CJSONObject*>* tempMap = tempDict->GetMap();

		for (const auto& pair : *tempMap)
		{
			tempObj = pair.second;
			if (tempObj->GetType() != efc::CJSONType::String)
			{
				return false;
			}

			efc::CJSONString* tempStr = (efc::CJSONString*)tempObj;

			m_Translations.emplace(pair.first, tempStr->GetValue());
		}
	}
	return true;
}

void AuExpManager::EndExport() 
{
	m_IsExportInProcess = true;

	//PopulateExportDataHeader();

	ExportInternal();

	/*
	TCHAR path[MAX_PATH];
	if (SHGetFolderPathW(0, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_CURRENT, path) == S_OK)
	{
		std::wstring prefsFilePath = path;
		prefsFilePath += L"\\AutomationGame\\ExporterPrefs";

		DWORD att = GetFileAttributes(prefsFilePath.c_str());

		if (att == INVALID_FILE_ATTRIBUTES)
		{
			//create directory, one level at a time:
			size_t slashPos = FindDirDelimiter(prefsFilePath, 0);
			size_t offset = 0;

			while (slashPos != std::wstring::npos)
			{
				CreateDirectory(prefsFilePath.substr(offset, slashPos - offset).c_str(), nullptr);
				slashPos = FindDirDelimiter(prefsFilePath, slashPos + 1);
			}

			//last one:
			CreateDirectory(prefsFilePath.c_str(), nullptr);

			att = GetFileAttributes(prefsFilePath.c_str());
		}

		if (att != INVALID_FILE_ATTRIBUTES && att & FILE_ATTRIBUTE_DIRECTORY)
		{
			//directory exists, all good to go:

			std::wstring boolPrefsFilePath = prefsFilePath + L"\\csvExporter.prefs";

			FILE* prefsFile;
			_wfopen_s(&prefsFile, boolPrefsFilePath.c_str(), L"wb");

			if (prefsFile)
			{
				ExportPrefs prefs;
				prefs.PrefsVersion = EXPPREFSVERSION;
				prefs.PrefsFlags = 0b00000000 | (m_CarData->GetBoolData(0)->Value << 0) | (m_CarData->GetBoolData(1)->Value << 1);

				fwrite(&prefs, sizeof(prefs), 1, prefsFile);

				fclose(prefsFile);
			}
		}
	}
	*/

	m_IsExportInProcess = false;
}

void AuExpManager::ExportInternal()
{
	/*
	m_FileName = m_CarData->GetStringData(0)->Value;

	for (std::wstring::iterator i = m_FileName.begin(); i < m_FileName.end(); i++)
	{
		if (IllegalCharacters.find(*i) != std::wstring::npos)
		{
			*i = L'_';
		}
	}
	
	m_Delimiter = m_CarData->GetStringData(1)->Value;
	*/
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

/*
void AuExpManager::FillTableData(LuaData& data)
{
	std::wstring ret;

	for (uint32_t i = 0; i < data.Size(); i++)
	{
		ret = L"???";

		switch (data[i].DataType)
		{
		case DataType_String:
			TryGetStringData(data[i].HeaderName, ret);
			break;

		case DataType_Float:
			TryGetFloatDataString(data[i].HeaderName, &ret);
			break;

		case DataType_ExporterVersion:
			ret = EXPVERSIONWS;
			break;

		case DataType_CarName:
			ret = m_CarData->GetCarName();
			break;

		default:
			break;
		}
		GetTranslation(&ret);
		data[i].SetData(ret);
	}
}
*/

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


	/*
	FileManager fileManager;

	for (int i = 0; i < data.GetCount(); i++)
	{
		m_LuaFDataFiles[data[i].FileName] = data[i];

		AuCarLuaDataFile& File = data[i];

		std::wstring FileName = File.FileName;
		//FileName = TextUtils::SanitiseFileName(FileName);
		//FILE* OutFile; //= m_FileManager->OpenFile(m_Directory + L"/" + FileName, L"wb");
		FILE* OutFile = fileManager.OpenFile((FileName).c_str(), L"wb");
		if (!OutFile)
		{
			return;
		}

		fwrite(File.Buffer, 1, File.BufferSize, OutFile);

	}
	*/
}

void AuExpManager::GetTranslation(std::wstring* ret)
{
	if (m_Translations.find(*ret) == m_Translations.end())
		return;

	*ret = m_Translations[*ret];
}

void AuExpManager::ShowMessageBox(std::wstring text, std::wstring title)
{
	MessageBox(NULL, text.c_str(), title.c_str(), 0);
}
