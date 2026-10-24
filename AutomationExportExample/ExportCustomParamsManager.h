#pragma once
#include <future>

#include "nlohmann/json.hpp"
using json = nlohmann::ordered_json;

#include "ExportCustomParams.h"
#include "HTTPManager.h"

struct ExportCustomParamsGroup 
{
	std::vector<ExportCustomStringParam> StringParams;
	std::vector<ExportCustomBoolParam> BoolParams;
};

enum class ExportParamsDataType : char
{
	UNKNOWN,
	STRING,
	BOOL
};

struct ExportCustomParamsWebResult 
{
	ExportParamsDataType DataType = ExportParamsDataType::UNKNOWN;
	std::wstring LuaName;
	json WebValue;
};


enum class ExportParamsWebResponseType : char
{
	UNKNOWN,
	TEXT,
	JSON
};

struct WebValueRequestParams
{
	std::wstring LuaName;
	std::string Host;
	std::string Path;
	httplib::Params Params;
	ExportParamsDataType DataType;
	ExportParamsWebResponseType ResponseType;
	json Schema;
	std::vector<int> AllowedStatuses;
	bool TextOnFail;
};

static const std::map<std::string, ExportParamsWebResponseType> ExportParamsWebResponseTypeMap = {
	{ "text", ExportParamsWebResponseType::TEXT },
	{ "json", ExportParamsWebResponseType::JSON },
};

class ExportCustomParamsManager
{
public:
	static ExportCustomParamsGroup LoadParameterJSONs(const std::wstring& path, bool allowWebString, bool allowWebBool);
	static bool TrySaveParamsGroup(const ExportCustomParamsGroup& exportParamGroup, const std::wstring& directory);
private:
	static void ParseParameterList(const json& array, ExportCustomParamsGroup& exportParamGroup, 
									std::vector<std::future<ExportCustomParamsWebResult>>& webRequests, bool allowWebString, bool allowWebBool);
	static ExportCustomParamsWebResult GetParameterValueFromWeb(const WebValueRequestParams webRequestParams);
};

