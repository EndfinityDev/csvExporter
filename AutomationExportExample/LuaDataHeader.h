#pragma once


#include "DataType.h"

struct LuaDataHeader
{

	LuaDataHeader(std::wstring title, std::wstring name, DataType type, bool doNotTranslate = false) 
	{
		HeaderTitle = title;
		HeaderName = name;
		DataType = type;
		DoNotTranslate = doNotTranslate;
	}

	//void SetData(std::wstring data) { Data = data; }

	std::wstring HeaderTitle;
	std::wstring HeaderName;
	DataType DataType;
	bool DoNotTranslate;

	//std::wstring Data;
};
