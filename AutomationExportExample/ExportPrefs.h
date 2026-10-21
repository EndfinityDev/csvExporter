#pragma once
#include <cstdint>
#include <ShlObj.h>

#include "ExportPrefsVersion.h"
#include "ExportManager.h"
#include "DirectoryManager.h"

#define PREFS_FILE_MAX_LENGTH 25
#define PREFS_LOCALE_MAX_LENGTH 15

struct ExportPrefs
{
	uint8_t Version = EXPPREFSVERSION;
	wchar_t Delimiter = L',';
	wchar_t LuaFile[PREFS_FILE_MAX_LENGTH] = L"";
	wchar_t DataFile[PREFS_FILE_MAX_LENGTH] = L"";
	wchar_t TranslationsFile[PREFS_FILE_MAX_LENGTH] = L"";
	wchar_t Locale[PREFS_LOCALE_MAX_LENGTH] = L"";

	static ExportPrefs LoadFromFile()
	{
		ExportPrefs prefs;

		TCHAR path[MAX_PATH];

		//get the user's documents directory:
		if (SHGetFolderPathW(0, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_CURRENT, path) == S_OK)
		{
			std::wstring prefsDir = path;
			prefsDir += AuExpManager::APPDATA_EXPORTER_PREFS_DIR + L"CSVExporter2.prefs";

			//std::wstring prefsFile = prefsDir + L"CSVExporter2.prefs";

			FILE* file;
			errno_t err = _wfopen_s(&file, prefsDir.c_str(), L"rb");

			if (!err && file)
			{
				fseek(file, 0, SEEK_END);
				size_t prefsFileSize = ftell(file);
				fseek(file, 0, SEEK_SET);

				if (prefsFileSize == sizeof(prefs))
				{
					uint8_t prefsFileVersion;
					fread_s(&prefsFileVersion, sizeof(prefsFileVersion), sizeof(prefsFileVersion), 1, file);
					fseek(file, 0, SEEK_SET);

					if (prefsFileVersion == EXPPREFSVERSION)
					{
						fread_s(&prefs, sizeof(prefs), prefsFileSize, 1, file);
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
				fclose(file);
			}
			else
			{
				// TODO
			}
		}
		else
		{
			//TODO
		}

		return prefs;
	}

	static bool TryWriteToFile(const AuCarExpCarData* carData, const std::wstring& locale)
	{
		TCHAR path[MAX_PATH];

		//get the user's documents directory:
		if (SHGetFolderPathW(0, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_CURRENT, path) == S_OK)
		{
			std::wstring prefsDir = path;
			prefsDir += AuExpManager::APPDATA_EXPORTER_PREFS_DIR;

			if (!DirectoryManager::CheckPathIsDirectory(prefsDir))
			{
				if (!DirectoryManager::TryCreateDirectoryRecursive(prefsDir))
				{
					//TODO
					return false;
				}
			}

			std::wstring prefsFile = prefsDir + L"CSVExporter2.prefs";

			FILE* file;
			errno_t err = _wfopen_s(&file, prefsFile.c_str(), L"wb");

			if (err || !file)
			{
				// TODO
				return false;
			}

			ExportPrefs prefs;

			std::wstring delimiter = carData->GetStringData(ExporterStringData_CSVDelimiter)->Value;
			if (delimiter.size() == 1)
			{
				prefs.Delimiter = delimiter[0];
			}

			std::wstring luaFile = carData->GetStringData(ExporterStringData_LuaFileName)->Value;
			if (luaFile.size() < PREFS_FILE_MAX_LENGTH)
			{
				for (size_t i = 0; i < luaFile.size(); i++)
				{
					prefs.LuaFile[i] = luaFile[i];
				}
			}

			std::wstring dataFile = carData->GetStringData(ExporterStringData_DataJSONFileName)->Value;
			if (dataFile.size() < PREFS_FILE_MAX_LENGTH)
			{
				for (size_t i = 0; i < dataFile.size(); i++)
				{
					prefs.DataFile[i] = dataFile[i];
				}
			}

			std::wstring translationsFile = carData->GetStringData(ExporterStringData_TranslationsJSONFileName)->Value;
			if (translationsFile.size() < PREFS_FILE_MAX_LENGTH)
			{
				for (size_t i = 0; i < translationsFile.size(); i++)
				{
					prefs.TranslationsFile[i] = translationsFile[i];
				}
			}

			std::wstring localeName = locale;
			if (localeName.size() < PREFS_LOCALE_MAX_LENGTH)
			{
				for (size_t i = 0; i < localeName.size(); i++)
				{
					prefs.Locale[i] = localeName[i];
				}
			}

			fwrite(&prefs, sizeof(prefs), 1, file);
			fclose(file);

			return true;
		}

		//TODO
		return false;
	}
};
