#pragma once

#include "nlohmann/json.hpp"
using json = nlohmann::ordered_json;

#include "ExportCustomParams.h"

struct ExportCustomParamsGroup {
	std::vector<ExportCustomStringParam> StringParams;
	std::vector<ExportCustomBoolParam> BoolParams;
};

class ExportCustomParamsManager
{
public:
	static ExportCustomParamsGroup LoadParameterJSONs(const std::wstring& path);
private:
	static void ParseParameterList(const json& array, ExportCustomParamsGroup& exportParamGroup);
};

