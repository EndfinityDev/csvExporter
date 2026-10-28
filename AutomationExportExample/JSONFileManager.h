#pragma once
#include <string>
#include <unordered_map>

#include "FileUtils.h"

#include "nlohmann/json.hpp"
using json = nlohmann::ordered_json;

enum class JSONDataType
{
	ANY,
	OBJECT,
	ARRAY
};

enum class JSONSchemaResultType
{
	NONE = 0,

	STRING = 1 << 0,
	INT = 1 << 1,
	FLOAT = 1 << 2,
	BOOL = 1 << 3,

	NUMBER = INT | FLOAT,

	ANY = STRING | INT | FLOAT | BOOL
};

inline JSONSchemaResultType operator&(JSONSchemaResultType lhs, JSONSchemaResultType rhs) {
	return static_cast<JSONSchemaResultType>(
		static_cast<int>(lhs) & static_cast<int>(rhs)
		);
}

inline JSONSchemaResultType operator|(JSONSchemaResultType lhs, JSONSchemaResultType rhs) {
	return static_cast<JSONSchemaResultType>(
		static_cast<int>(lhs) | static_cast<int>(rhs)
		);
}

static const std::unordered_map<std::string, JSONSchemaResultType> JSONSchemaResultTypeMap = {
	{ "string", JSONSchemaResultType::STRING },
	{ "str", JSONSchemaResultType::STRING },
	{ "int", JSONSchemaResultType::INT },
	{ "integer", JSONSchemaResultType::INT },
	{ "float", JSONSchemaResultType::FLOAT },
	{ "bool", JSONSchemaResultType::BOOL },
	{ "boolean", JSONSchemaResultType::BOOL },
	{ "number", JSONSchemaResultType::NUMBER },
	{ "num", JSONSchemaResultType::NUMBER },
	{ "any", JSONSchemaResultType::ANY }
};

class JSONFileManager
{
public:
	static bool TryParseJSONFile(std::wstring filename, std::wstring path, json* outData, 
									const bool silentError = false, const JSONDataType topLevelType = JSONDataType::OBJECT);

	template <typename T = std::wstring>
	static bool TryParseJSONData(const T& inJSONString, json* outData, const bool silentError = false, const JSONDataType topLevelType = JSONDataType::OBJECT);
	static json GetJSONValueBySchema(const json& data, const json& schema);
private:
	static json TraverseJSONDictBySchema(const json& dataDict, const json& schemaDict);
	static json TraverseJSONListBySchema(const json& dataList, const json& schemaList);
};
