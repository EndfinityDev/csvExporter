#include "DataType.h"
#include "stdafx.h"

CSVDataType ParseDataType(std::wstring inString)
{
	auto& iterator = CSVDataTypeMap.find(inString);
	if (iterator != CSVDataTypeMap.end())
	{
		return iterator->second;
	}
	return CSVDataType::UNKNOWN;

	/*
	if (inString == L"DataType_String")
		return DataType_String;
	else if (inString == L"DataType_Float")
		return DataType_Float;
	else if (inString == L"DataType_ExporterVersion")
		return DataType_ExporterVersion;
	else if (inString == L"DataType_CarName")
		return DataType_CarName;
	else
		return DataType_Unknown;
	*/
}