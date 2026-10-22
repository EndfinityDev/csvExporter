/************************************************* *
** Copyright(c) 2021 -- Camshaft Software PTY LTD
** All Rights Reserved
************************************************** */

#pragma once

#include "csvExporter.h"
#include "LuaData.h"
#include "CSVManager.h"
#include "ExportCustomParamsManager.h"

#include "nlohmann/json.hpp"
using json = nlohmann::ordered_json;

enum ExporterStringData : unsigned int
{
	ExporterStringData_FileName = 0,
	ExporterStringData_CSVDelimiter,

	ExporterStringData_LuaFileName,
	ExporterStringData_DataJSONFileName,
	ExporterStringData_TranslationsJSONFileName,

	ExporterStringData_Locale,

	ExporterStringData_Count
};

enum ExporterBoolData : unsigned int
{


	ExporterBoolData_Count
};


class AuExpManager
{
public:

	static AuExpManager* GetInstance() { return s_Instance; }
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

	inline unsigned int GetLuaFileCount() const { return m_LuaFiles.size(); }
	inline size_t GetLuaFileLength(const unsigned int fileID) const { return m_LuaFiles[fileID].length(); }
	inline std::string GetLuaFile(const unsigned int fileID) const { return m_LuaFiles[fileID]; }

	inline std::wstring GetLocale() const { return m_Locale; }

	static const std::wstring APPDATA_EXPORTER_DIR;
	static const std::wstring APPDATA_EXPORTER_PREFS_DIR;
private:

	AuExpManager();
	~AuExpManager() {}

	void ConfigureCustomParamData();
	void BuildParamsLuaFile();

	void ExportInternal();

	void BuildSingleCSV();

	bool LoadLuaFiles(const std::wstring& fileNames, const std::wstring& path);
	bool LoadLuaFile(const std::wstring& filename, const std::wstring& path);
	bool LoadLuaResource();
	bool LoadDataFile(const std::wstring& filename, const std::wstring& path);
	bool LoadDataResource();
	bool LoadTranslationFile(const std::wstring& filename, const std::wstring& path);
	bool LoadTranslationResource();

	bool LoadDataJSON(const json& jsonData);
	bool LoadDataEntriesFromJSON(const json& arrayObject);

	bool LoadTranslationJSON(const json& jsonData);

	static AuExpManager* s_Instance;
	static const std::wstring s_IllegalCharacters;

	bool m_IsExportInProcess = false;

	const AuCarExpCarData* m_CarData = nullptr;

	ExportCustomParamsGroup m_ExportCustomParamsGroup;

	std::vector<const AuCarExpTexture*> m_AllImages;

	std::wstring m_ExportDirectory;

	std::map<std::wstring, int> m_MeshNameCounts;

	std::map<std::wstring, float> m_LuaFloatData;
	std::map<std::wstring, std::wstring> m_LuaStringData;
	std::map<std::wstring, AuCarLuaDataFile> m_LuaFDataFiles;

	std::vector<LuaDataHeader> m_LuaData;
	std::map<std::wstring, std::wstring> m_Translations;

	std::wstring m_FileName = L"";
	std::wstring m_EngineName = L"";
	std::wstring m_Delimiter = L"";

	std::vector<std::string> m_LuaFiles;
	//std::string m_LuaFile = "";
	//size_t m_LuaFileLength = 0;

	bool m_LuaDataAdded = false;

	std::wstring m_Locale = L"";

	const HMODULE m_Module;
};