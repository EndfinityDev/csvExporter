#pragma once
#include <string>

#ifndef CSVE_VERSIONCHECK_TARGET_URL
#define CSVE_VERSIONCHECK_TARGET_URL ""
#endif // !CSVE_VERSIONCHECK_TARGET_URL

#ifndef CSVE_VERSIONCHECK_VERIFY_CERT
#define CSVE_VERSIONCHECK_VERIFY_CERT true
#endif // !CSVE_VERSIONCHECK_VERIFY_CERT

#define CSVE_VERSIONCHECK_TIMESTAMP_FILE_VERSION 1

#ifndef CSVE_VERSIONCHECK_HOURS_LIMIT
#define CSVE_VERSIONCHECK_HOURS_LIMIT 20
#endif // !CSVE_VERSIONCHECK_HOURS_LIMIT

#include "HTTPManager.h"
#include "JSONFileManager.h"


class VersionChecker
{
public:
	static void RunVersionCheck();
private:
	static void RunVersionCheckThread();
	static bool CheckFrequencyThreshold();
};

