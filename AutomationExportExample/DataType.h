#pragma once
#include <string>

enum DataType : uint8_t
{
	DataType_String					= 0,
	DataType_Float					= 1,
	DataType_ExporterVersion		= 2,
	DataType_CarName				= 3,

	DataType_Unknown				= 4
};

DataType ParseDataType(std::wstring inString);