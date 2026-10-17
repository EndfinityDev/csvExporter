#pragma once

#include <string>

#ifdef ARCHITECHTURE_x64
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif

#include "httplib.h"


class HTTPManager
{
public:
	static std::string Get(std::string url);
};

