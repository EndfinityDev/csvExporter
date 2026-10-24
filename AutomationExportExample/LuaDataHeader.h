#pragma once


#include "DataType.h"

struct LuaDataHeader
{

	LuaDataHeader(std::wstring title, std::wstring name, CSVDataType type, bool doNotTranslate = false) 
	{
		HeaderTitle = title;
		HeaderName = name;
		CSVDataType = type;
		DoNotTranslate = doNotTranslate;
	}

	//void SetData(std::wstring data) { Data = data; }

	std::wstring HeaderTitle;
	std::wstring HeaderName;
	CSVDataType CSVDataType;
	bool DoNotTranslate;

	//std::wstring Data;
};
