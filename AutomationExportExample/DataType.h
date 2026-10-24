#pragma once
#include <string>
#include <map>

enum class CSVDataType : uint8_t
{
	UNKNOWN,

	STRING,
	FLOAT,
	EXPORTER_VERSION,
	CAR_NAME
};

static const std::map<std::wstring, CSVDataType> CSVDataTypeMap = {
	{ L"DataType_String", CSVDataType::STRING },
	{ L"DataType_Float", CSVDataType::FLOAT },
	{ L"DataType_ExporterVersion", CSVDataType::EXPORTER_VERSION },
	{ L"DataType_CarName", CSVDataType::CAR_NAME },
	{ L"string", CSVDataType::STRING },
	{ L"float", CSVDataType::FLOAT },
	{ L"exporter_version", CSVDataType::EXPORTER_VERSION },
	{ L"car_name", CSVDataType::CAR_NAME },
};

CSVDataType ParseDataType(std::wstring inString);