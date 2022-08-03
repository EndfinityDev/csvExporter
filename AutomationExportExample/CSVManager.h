#pragma once

#include "stdafx.h"
#include "LuaData.h"
#include "FileManager.h"

class CSVManager
{
public:
	CSVManager(std::wstring filepath, std::wstring delimiter);
	~CSVManager() { }

	void AddData(const LuaData& data);

	void Save() const;

private:
	std::vector<LuaData> m_Data;

	std::wstring m_FilePath;

	std::wstring m_Delimiter;

	bool VerifyHeaders(const LuaData& data);

	bool TryCompareFileHeaders(FileManager fileManager) const;
};

