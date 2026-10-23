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
	CSVManager(std::wstring filepath, std::wstring delimiter, bool requireSameHeader);
	~CSVManager() { }

	//inline void BindData(const std::vector<LuaDataHeader>* luaData) { m_Data = luaData; }

	//void SaveOld() const;
	void Save(std::vector<LuaDataHeader>& luaDataHeaders) const;


private:
	inline void WriteHeader(std::wofstream& outputFile, std::wstring headerText) const { outputFile << headerText; }
	void WriteData(std::wofstream& outputFile, std::vector<LuaDataHeader>& luaDataHeaders) const;

	std::wstring BuildCSVHeaderString(std::vector<LuaDataHeader>& luaDataHeaders, const std::locale& locale) const;
	bool CheckSameHeader(const std::wstring& headerString, const std::locale& locale) const;

	//bool VerifyHeaders(const LuaData& data);
	//bool TryCompareFileHeaders(FileManager fileManager) const;

	//const std::vector<LuaDataHeader>* m_Data;

	std::wstring m_FilePath;
	std::wstring m_Delimiter;
	bool m_RequireSameHeader;

	AuExpManager* m_ExportManager;

};

