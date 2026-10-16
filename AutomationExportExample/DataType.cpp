#include "DataType.h"
#include "stdafx.h"

DataType ParseDataType(std::wstring inString)
{
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
}