#pragma once
#include <cstdint>
#include <ShlObj.h>

#include "ExportManager.h"
#include "DirectoryManager.h"

#define CSVE_EXP_PREFS_VERSION 1

// One character can take up multiple "character slots"
#define CSVE_PREFS_FILE_MAX_LENGTH 255 // 254 character slots + null terminator
#define CSVE_PREFS_LOCALE_MAX_LENGTH 15 // 14 character slots + null terminator
#define CSVE_PREFS_DELIMITER_MAX_LENGTH 12 // 11 character slots + null terminator

//Legacy V0
#define CSVE_EXP_PREFS_VERSION_V0 0
#define CSVE_PREFS_FILE_MAX_LENGTH_V0 25 // 24 character slots + null terminator
#define CSVE_PREFS_LOCALE_MAX_LENGTH_V0 15 // 14 character slots + null terminator

struct ExportPrefs_V0
{
	uint8_t Version = CSVE_EXP_PREFS_VERSION_V0;
	wchar_t Delimiter = L',';
	wchar_t LuaFile[CSVE_PREFS_FILE_MAX_LENGTH_V0] = L"";
	wchar_t DataFile[CSVE_PREFS_FILE_MAX_LENGTH_V0] = L"";
	wchar_t TranslationsFile[CSVE_PREFS_FILE_MAX_LENGTH_V0] = L"";
	wchar_t Locale[CSVE_PREFS_LOCALE_MAX_LENGTH_V0] = L"";
};

struct ExportPrefs
{
	uint8_t Version = CSVE_EXP_PREFS_VERSION;
	bool RequireSameHeader = false;
	wchar_t Delimiter[CSVE_PREFS_DELIMITER_MAX_LENGTH] = L",";
	wchar_t LuaFile[CSVE_PREFS_FILE_MAX_LENGTH] = L"";
	wchar_t DataFile[CSVE_PREFS_FILE_MAX_LENGTH] = L"";
	wchar_t TranslationsFile[CSVE_PREFS_FILE_MAX_LENGTH] = L"";
	wchar_t Locale[CSVE_PREFS_LOCALE_MAX_LENGTH] = L"";

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

				if (prefsFileSize > 0)
				{
					uint8_t prefsFileVersion;
					fread_s(&prefsFileVersion, sizeof(prefsFileVersion), sizeof(prefsFileVersion), 1, file);
					fseek(file, 0, SEEK_SET);

					if (prefsFileVersion == CSVE_EXP_PREFS_VERSION && prefsFileSize == sizeof(prefs))
					{
						fread_s(&prefs, sizeof(prefs), prefsFileSize, 1, file);
					}
					else if (prefsFileVersion == CSVE_EXP_PREFS_VERSION_V0 && prefsFileSize == sizeof(ExportPrefs_V0))
					{
						ExportPrefs_V0 legacyPrefs;
						fread_s(&legacyPrefs, sizeof(legacyPrefs), prefsFileSize, 1, file);
						prefs = FromLegacy_V0(legacyPrefs);
					}
					else
					{
						Logger::GetLogger()->Log(L"[ExportPrefs] Encountered unsupported prefs file version");
					}
				}
				else
				{
					Logger::GetLogger()->Log(L"[ExportPrefs] Encountered empty prefs file");
				}
				fclose(file);
			}
			else
			{
				Logger::GetLogger()->Log(L"[ExportPrefs] Failed to open prefs file for reading");
			}
		}
		else
		{
			Logger::GetLogger()->Log(L"[ExportPrefs] Failed to get application data directory");
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
					Logger::GetLogger()->Log(L"[ExportPrefs] Failed to create prefs file directory");
					return false;
				}
			}

			std::wstring prefsFile = prefsDir + L"CSVExporter2.prefs";

			FILE* file;
			errno_t err = _wfopen_s(&file, prefsFile.c_str(), L"wb");

			if (err || !file)
			{
				Logger::GetLogger()->Log(L"[ExportPrefs] Failed to open prefs file for writing");
				return false;
			}

			ExportPrefs prefs;

			prefs.RequireSameHeader = carData->GetBoolData(ExporterBoolData_RequireSameHeader)->Value;

			const std::wstring& delimiter = carData->GetStringData(ExporterStringData_CSVDelimiter)->Value;
			if (delimiter.size() < CSVE_PREFS_DELIMITER_MAX_LENGTH)
			{
				wcscpy_s(prefs.Delimiter, delimiter.c_str());
			}

			const std::wstring& luaFile = carData->GetStringData(ExporterStringData_LuaFileName)->Value;
			if (luaFile.size() < CSVE_PREFS_FILE_MAX_LENGTH)
			{
				wcscpy_s(prefs.LuaFile, luaFile.c_str());
			}

			const std::wstring& dataFile = carData->GetStringData(ExporterStringData_DataJSONFileName)->Value;
			if (dataFile.size() < CSVE_PREFS_FILE_MAX_LENGTH)
			{
				wcscpy_s(prefs.DataFile, dataFile.c_str());
			}

			const std::wstring& translationsFile = carData->GetStringData(ExporterStringData_TranslationsJSONFileName)->Value;
			if (translationsFile.size() < CSVE_PREFS_FILE_MAX_LENGTH)
			{
				wcscpy_s(prefs.TranslationsFile, translationsFile.c_str());
			}

			if (locale.size() < CSVE_PREFS_LOCALE_MAX_LENGTH)
			{
				wcscpy_s(prefs.Locale, locale.c_str());
			}

			fwrite(&prefs, sizeof(prefs), 1, file);
			fclose(file);

			return true;
		}

		Logger::GetLogger()->Log(L"[ExportPrefs] Failed to get application data directory");
		return false;
	}

	static ExportPrefs FromLegacy_V0(const ExportPrefs_V0& legacyPrefs)
	{
		ExportPrefs prefs;
		prefs.Delimiter[0] = legacyPrefs.Delimiter;
		prefs.Delimiter[1] = L'\0';
		//wcscpy_s(stringData[ExporterStringData_Count + i].Value, iterator->second.c_str());
		wcscpy_s(prefs.LuaFile, legacyPrefs.LuaFile);
		wcscpy_s(prefs.DataFile, legacyPrefs.DataFile);
		wcscpy_s(prefs.TranslationsFile, legacyPrefs.TranslationsFile);
		wcscpy_s(prefs.Locale, legacyPrefs.Locale);

		return prefs;
	}
};
