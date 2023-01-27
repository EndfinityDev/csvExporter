#pragma once


#include "DataType.h"

struct LuaDataHeader
{

	LuaDataHeader(std::wstring title, std::wstring name, DataType type) 
	{
		HeaderTitle = title;
		HeaderName = name;
		DataType = type;
	}

	void SetData(std::wstring data) { Data = data; }

	std::wstring HeaderTitle;
	std::wstring HeaderName;
	DataType DataType;

	std::wstring Data;
};
