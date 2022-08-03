#include "LuaData.h"
#include "stdafx.h"

LuaData::LuaData() :
	m_Data(std::vector<LuaDataHeader>())
{
}

void LuaData::Add(const LuaDataHeader& data) 
{
	m_Data.push_back(data);
}

void LuaData::Add(const LuaData& data)
{
	for (const LuaDataHeader& item : data.m_Data) 
	{
		m_Data.push_back(item);
	}
}

void LuaData::Add(std::wstring title, std::wstring name, DataType type)
{
	Add(LuaDataHeader(title, name, type));
}

LuaData LuaData::Add(LuaData data, const LuaDataHeader& header) const
{
	data.Add(header);
	return data;
}

LuaData LuaData::Add(LuaData a, const LuaData& b) const
{
	a.Add(b);
	return a;
}

LuaData LuaData::operator+(const LuaDataHeader& data) const
{
	LuaData ret = LuaData();
	ret.Add(*this);
	ret.Add(data);
	return ret;
}

LuaData LuaData::operator+(const LuaData& data) const
{
	LuaData ret = LuaData();
	ret.Add(*this);
	ret.Add(data);
	return ret;
}

LuaData& LuaData::operator+=(const LuaDataHeader& data)
{
	Add(data);
	return *this;
}

LuaData& LuaData::operator+=(const LuaData& data)
{
	Add(data);
	return *this;
}

const LuaDataHeader LuaData::operator[](uint32_t index) const
{
	return m_Data[index];
}

LuaDataHeader& LuaData::operator[](uint32_t index)
{
	return m_Data[index];
}

int32_t LuaData::Size() const
{
	return m_Data.size();
}