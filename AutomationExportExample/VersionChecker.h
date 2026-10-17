#pragma once
#include <string>

#ifndef CSVE_VERSIONCHECK_TARGET_URL
#define CSVE_VERSIONCHECK_TARGET_URL ""
#endif // !CSVE_VERSIONCHECK_TARGET_URL

#ifndef CSVE_VERSIONCHECK_VERIFY_CERT
#define CSVE_VERSIONCHECK_VERIFY_CERT true
#endif // !CSVE_VERSIONCHECK_VERIFY_CERT

#include "HTTPManager.h"
#include "JSONFileManager.h"


class VersionChecker
{
public:
	static void RunVersionCheck();
private:
	static void RunVersionCheckThread();
};

