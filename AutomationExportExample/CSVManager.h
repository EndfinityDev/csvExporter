#pragma once

#include "stdafx.h"
//#include "LuaData.h"
//#include "FileManager.h"
#include <sstream>
#include <fstream>
//#include "ExportManager.h"
#include "LuaDataHeader.h"

class AuExpManager;

class CSVManager
{
public:
	CSVManager(std::wstring filepath, std::wstring delimiter);
	~CSVManager() { }

	inline void BindData(const std::vector<LuaDataHeader>* luaData) { m_Data = luaData; }

	//void SaveOld() const;
	void Save() const;

	std::wstring BuildCSVHeaderString() const;

private:
	inline void WriteHeader(std::wofstream& outputFile, std::wstring headerText) const { outputFile << headerText; }
	void WriteData(std::wofstream& outputFile) const;

	//bool VerifyHeaders(const LuaData& data);
	//bool TryCompareFileHeaders(FileManager fileManager) const;

	const std::vector<LuaDataHeader>* m_Data;

	std::wstring m_FilePath;

	std::wstring m_Delimiter;

	AuExpManager* m_ExportManager;

};

