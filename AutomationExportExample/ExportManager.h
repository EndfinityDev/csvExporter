/************************************************* *
** Copyright(c) 2021 -- Camshaft Software PTY LTD
** All Rights Reserved
************************************************** */

#pragma once

#include "csvExporter.h"
#include "LuaData.h"
#include "CSVManager.h"
#include "EXPVERSION.h"

class AuExpManager
{
public:

	static AuExpManager* Instance() { return s_Instance; }
	static void CreateInstance() { if (!s_Instance) s_Instance = new AuExpManager(); }
	static void DestroyInstance() { delete s_Instance; s_Instance = nullptr; }


	AuCarExpErrorCode Init(const AuCarExpCarData* carData);

	bool IsExportInProcess() { return m_IsExportInProcess; }

	//Getting pixel data is a threaded operation within Automation, which means that we should not assume pixel data within these textures is complete until EndExport() is called
	//So what we do is save the reference and save to file at the end of the export
	void AddImage(const AuCarExpTexture* texture);
	void SaveImages();
	void EndExport();

	void SaveMesh(const AuCarExpMesh* mesh, const wchar_t* name);

	const wchar_t* GetExportDirectory() const { return m_ExportDirectory.c_str(); }

	void GetTranslation(std::wstring* ret);

	void AddLuaFiles(const AuCarExpArray<AuCarLuaDataFile>& Data);
	void AddLuaStringData(const AuCarExpArray<AuCarExpLuaStringData>& Data);
	void AddLuaFloatData(const AuCarExpArray<AuCarExpLuaFloatData>& Data);
	bool TryGetFloatData(const std::wstring& key, float* retValue);
	bool TryGetFloatDataString(const std::wstring& key, std::wstring* retValue);
	bool TryGetStringData(const std::wstring& key, std::wstring& value);

	const wchar_t* GetCarName() const { return m_CarData->GetCarName(); }

	const std::wstring IllegalCharacters = L"\\/:?\"<>|";

private:

	AuExpManager();
	void PopulateExportDataHeader();
	~AuExpManager() {}

	void ExportInternal();

	void BuildCarAndEngineChoicesAndResultsCSVs();

	void FillTableData(LuaData& data);

	void BuildCarAndEngineCSVs();

	void BuildResultAndChoiceCSVs();

	void BuildSingleCSV();

	void BuildTranslations();

	static AuExpManager* s_Instance;

	bool m_IsExportInProcess = false;

	const AuCarExpCarData* m_CarData;

	std::vector<const AuCarExpTexture*> m_AllImages;

	std::wstring m_ExportDirectory;

	std::map<std::wstring, int> m_MeshNameCounts;

	std::map<std::wstring, float> m_LuaFloatData;
	std::map<std::wstring, std::wstring> m_LuaStringData;
	std::map<std::wstring, AuCarLuaDataFile> m_LuaFDataFiles;

	std::map<std::wstring, std::wstring> m_Translations;

	std::wstring m_FileName = L"";
	std::wstring m_EngineName = L"";
	std::wstring m_Delimiter = L"";

	LuaData m_GeneralData;
	LuaData m_CarChoicesData;
	LuaData m_CarResultsData;
	LuaData m_EngineChoicesData;
	LuaData m_EngineResultsData;
};



