#pragma once
#include <string>

#include "FileUtils.h"

#include "nlohmann/json.hpp"
using json = nlohmann::ordered_json;


class JSONFileManager
{
public:
	static bool TryParseJSONFile(std::wstring filename, std::wstring path, json* outData);

	template <typename T = std::wstring>
	static bool TryParseJSONData(T& inJSONString, json* outData, bool silentError = false);
private:
};
