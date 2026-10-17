#include "stdafx.h"

#include <thread>

#include "VersionChecker.h"
#include "EXPVERSION.h"
#include "MessageBoxManager.h"

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

		// TODO: check last shown message time

		if (exporterVersion > EXPVERSION)
		{
			MessageBoxManager::ShowMessageBox(L"A new exporter version is available", L"Update available");
		}
		return;
	}
	else
	{
		return;
	}
}
