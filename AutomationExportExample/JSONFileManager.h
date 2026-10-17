#pragma once
#include <string>

#include "FileUtils.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;


class JSONFileManager
{
public:
	static bool TryParseJSONFile(std::wstring filename, std::wstring path, json* outData);
	static bool TryParseJSONData(std::wstring& inJSONString, json* outData);
private:
};

