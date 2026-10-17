#include "stdafx.h"

#include <thread>
#include <chrono>

#include "VersionChecker.h"
#include "EXPVERSION.h"
#include "MessageBoxManager.h"
#include "StringConverter.h"

void VersionChecker::RunVersionCheck()
{
	if (CSVE_VERSIONCHECK_TARGET_URL[0] != '\0') // TODO: make this a compile time check
	{
		std::thread([]() {
			RunVersionCheckThread();
			}).detach();
	}
}

void VersionChecker::RunVersionCheckThread()
{
	std::string returnData = HTTPManager::Get(CSVE_VERSIONCHECK_TARGET_URL);
	if (returnData.empty())
	{
		return;
	}

	json jsonData;
	if (JSONFileManager::TryParseJSONData<std::string>(returnData, &jsonData, true))
	{
		if (!jsonData.contains("exporterVersion"))
		{
			return;
		}

		int exporterVersion = jsonData["exporterVersion"].get<int>();

		std::wstring exporterDisplayVersion = std::to_wstring(exporterVersion);

		if (jsonData.contains("displayVersion") && jsonData["displayVersion"].is_string())
		{
			auto& exporterDisplayVersionUTF8 = jsonData["displayVersion"].get<std::string>();
			exporterDisplayVersion = StringConverter::to_wstring(exporterDisplayVersionUTF8);
		}

		if (exporterVersion > EXPVERSION && !CheckFrequencyThreshold())
		{
			std::wstringstream ss;
			ss << "A new exporter version " << exporterDisplayVersion << " is available.";
#ifdef CSVE_VERSIONCHECK_UPDATE_URL
			const std::wstring updateURL = CSVE_VERSIONCHECK_UPDATE_URL;
			ss << "\nWould you like to open the download page now?";
			MessageBoxManager::ShowUpdateMessageBox(ss.str(), L"CSVExporter 2 Update available", updateURL);
#else
			MessageBoxManager::ShowMessageBox(ss.str(), L"CSVExporter 2 Update available");
#endif

		}
		return;
	}
	else
	{
		return;
	}
}

bool VersionChecker::CheckFrequencyThreshold()
{
	TCHAR path[MAX_PATH];

	std::wstring prefsDir;

	bool result = true; // true = within pause threshold, false if the time limit has elapsed or timestamp hasn't been cached

	//get the user's documents directory:
	if (SHGetFolderPathW(0, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_CURRENT, path) == S_OK)
	{
		prefsDir = path;
		prefsDir += L"\\AutomationGame\\ExporterPrefs\\";

		bool writeNewFile = false;

		std::wstring timestampFile = prefsDir + L"CSVExporter2.tsc";

		FILE* file;
		errno_t err = _wfopen_s(&file, timestampFile.c_str(), L"rb");

		if (!err && file)
		{
			fseek(file, 0, SEEK_END);
			size_t timestampFileSize = ftell(file);
			fseek(file, 0, SEEK_SET);

			if (timestampFileSize == sizeof(uint8_t) + sizeof(int64_t))
			{
				uint8_t timestampFileVersion = 0;
				fread_s(&timestampFileVersion, sizeof(timestampFileVersion), sizeof(timestampFileVersion), 1, file);

				if (timestampFileVersion == CSVE_VERSIONCHECK_TIMESTAMP_FILE_VERSION)
				{
					int64_t savedSeconds = 0;
					fread_s(&savedSeconds, sizeof(savedSeconds), sizeof(int64_t), 1, file);
					fclose(file);

					auto now = std::chrono::system_clock::now();
					std::chrono::seconds durationSinceEpoch(savedSeconds);
					std::chrono::system_clock::time_point pastTime(durationSinceEpoch);
					auto timeDiff = now - pastTime;
					auto hoursDiff = std::chrono::duration_cast<std::chrono::hours>(timeDiff).count();

					if (hoursDiff > CSVE_VERSIONCHECK_HOURS_LIMIT)
					{
						writeNewFile = true;
						result = false;
					}
				}
				else
				{
					fclose(file);
					writeNewFile = true;
					result = false;
				}
			}
			else
			{
				fclose(file);
				writeNewFile = true;
				result = false;
			}
		}
		else
		{
			writeNewFile = true;
			result = false;
		}

		if (writeNewFile)
		{
			errno_t err = _wfopen_s(&file, timestampFile.c_str(), L"wb");
			if (!err && file)
			{
				auto now = std::chrono::system_clock::now();
				uint8_t timestampFileVersion = CSVE_VERSIONCHECK_TIMESTAMP_FILE_VERSION;
				fwrite(&timestampFileVersion, sizeof(timestampFileVersion), 1, file);

				int64_t currentSeconds = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
				fwrite(&currentSeconds, sizeof(int64_t), 1, file);

				fclose(file);
			}
		}
	}
	return result;
}
