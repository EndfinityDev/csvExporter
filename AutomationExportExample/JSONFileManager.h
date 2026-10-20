#pragma once
#include <string>

#include "FileUtils.h"

#include "nlohmann/json.hpp"
using json = nlohmann::ordered_json;

enum class JSONDataType
{
	ANY,
	OBJECT,
	ARRAY
};

class JSONFileManager
{
public:
	static bool TryParseJSONFile(std::wstring filename, std::wstring path, json* outData);

	template <typename T = std::wstring>
	static bool TryParseJSONData(const T& inJSONString, json* outData, const bool silentError = false, const JSONDataType topLevelType = JSONDataType::OBJECT);
private:
};
