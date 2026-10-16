/************************************************* *
** Copyright(c) 2021 -- Camshaft Software PTY LTD
** All Rights Reserved
************************************************** */

#pragma once

#include "csvExporter.h"
#include "LuaData.h"
#include "CSVManager.h"
#include "EXPVERSION.h"

#include "CJSONReader.h"

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

#define PREFS_FILE_MAX_LENGTH 25
#define PREFS_LOCALE_MAX_LENGTH 15

struct ExportPrefs
{
	uint8_t Version = EXPPREFSVERSION;
	wchar_t Delimiter = L',';
	wchar_t LuaFile[PREFS_FILE_MAX_LENGTH] = L"";
	wchar_t DataFile[PREFS_FILE_MAX_LENGTH] = L"";
	wchar_t TranslationsFile[PREFS_FILE_MAX_LENGTH] = L"";
	wchar_t Locale[PREFS_LOCALE_MAX_LENGTH] = L"";
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

	inline size_t GetLuaFileLength() const { return m_LuaFileLength; }
	inline std::string GetLuaFile() const { return m_LuaFile; }

	inline std::wstring GetLocale() const { return m_Locale; }

	static inline void ShowMessageBox(std::wstring text, std::wstring title);

	static const std::wstring s_IllegalCharacters;

private:

	AuExpManager();
	~AuExpManager() {}

	void ExportInternal();

	void FillTableData(LuaData& data);

	void BuildSingleCSV();

	std::wstring ValidateFileName(std::wstring filename, std::wstring defaultExtension);

	bool LoadLuaFile(std::wstring filename, std::wstring path);
	bool LoadLuaResource();
	bool LoadDataFile(std::wstring filename, std::wstring path);
	bool LoadDataResource();
	bool LoadTranslationFile(std::wstring filename, std::wstring path);
	bool LoadTranslationResource();

	bool ParseDataJSON(std::wstring json);
	bool ParseTranslationJSON(std::wstring json);

	static AuExpManager* s_Instance;

	bool m_IsExportInProcess = false;

	const AuCarExpCarData* m_CarData = nullptr;

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

	std::string m_LuaFile = "";
	size_t m_LuaFileLength = 0;

	bool m_LuaDataAdded = false;

	std::wstring m_Locale = L"";

	//efc::CJSONDict* m_LuaData = nullptr;
	//efc::CJSONDict* m_TranslationJSON

	//LuaData m_GeneralData;

	const HMODULE m_Module;
};