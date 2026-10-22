#pragma once

#include <string>

#ifdef ARCHITECHTURE_x64
//#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif

#include "httplib.h"

#define CSVE_HTTP_USER_AGENT "CSVExporter2"


class HTTPManager
{
public:
	static std::string Get(const std::string& urlHost, const std::string urlPath, const httplib::Params& params, 
							bool includeAPIKey = false, bool allowAnyStatus = false, int* outStatus = nullptr);
};

