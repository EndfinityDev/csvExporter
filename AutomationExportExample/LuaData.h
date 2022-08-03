#pragma once
#include "LuaDataHeader.h"
#include "csvExporter.h"
#include "stdafx.h"

class LuaData
{

public:
	LuaData();
	~LuaData() { }

	void Add(const LuaDataHeader& data);
	void Add(const LuaData& data);
	void Add(std::wstring title, std::wstring name, DataType type);

	LuaData Add(LuaData data, const LuaDataHeader& header) const;
	LuaData Add(LuaData a, const LuaData& b) const;

	LuaData operator+(const LuaDataHeader& data) const;
	LuaData operator+(const LuaData& data) const;

	LuaData& operator+=(const LuaDataHeader& data);
	LuaData& operator+=(const LuaData& data);

	const LuaDataHeader operator[](uint32_t index) const;
	LuaDataHeader& operator[](uint32_t index);

	int32_t Size() const;

private:
	std::vector<LuaDataHeader> m_Data;

};

