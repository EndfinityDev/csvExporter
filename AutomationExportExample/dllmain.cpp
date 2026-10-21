/************************************************* *
** Copyright(c) 2019 -- Camshaft Software PTY LTD
** All Rights Reserved
************************************************** */


#include "stdafx.h"
#include <ShlObj.h>

#include "..\Include\AuCarExportDLL.h"

#if defined(_M_X64) || defined(_M_AMD64)
	#define ARCHITECHTURE_x64
#endif

#include "BuildDate.h"
#include "ExportManager.h"
#include "VersionChecker.h"
#include "ExportPrefs.h"
#include "ExportCustomParamsManager.h"

//Gets the plugin name, to be displayed in the drop-down list of available plugins
AuCarExpErrorCode AuCarExportDLL::GetExporterName(AuCarExpArray<wchar_t>& name, wchar_t const* locale)
{
	//Locale handling example (Automation default locale is "en-GB")
	//go go Google Translate (don't do this for actual plugin text :) )

	//your prefered strings handling method should go in here

	//Languages supported by Automation at the time of writing this:
	//(looking only at the language and not the region is enough for this example)
	
	//default to English
	wcscpy_s(name.GetData(), name.GetCount(), L"CSVExporter 2");
	

	return AuCarExpErrorCode_Success;
}

//Gets the plugin version number
//This is for display in the UI only (to help when users report bugs, etc.). Automation does nothing else with this information.
AuCarExpErrorCode AuCarExportDLL::GetExporterVersion(unsigned int* VersionNumber)
{
	//*VersionNumber = EXPVERSION;
	*VersionNumber = BUILD_DATE_INT;

#ifdef ARCHITECHTURE_x64
	VersionChecker::RunVersionCheck();
#endif

	return AuCarExpErrorCode_Success;
}

//Gets the number of user-supplied strings that the plugin will be requesting
AuCarExpErrorCode AuCarExportDLL::GetRequiredStringDataCount(unsigned int* retCount)
{
	//we will want to get 3 lots of string information from the user:
	*retCount = ExporterStringData_Count;

	TCHAR path[MAX_PATH];
	if (SHGetFolderPathW(0, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_CURRENT, path) == S_OK)
	{
		std::wstring exportParamDir = path + AuExpManager::APPDATA_EXPORTER_DIR + L"\\UserFiles\\.ExportParams";
		ExportCustomParamsGroup exportParams = ExportCustomParamsManager::LoadParameterJSONs(exportParamDir);
		*retCount += exportParams.StringParams.size();
	}

	return AuCarExpErrorCode_Success;
}

//Gets the user-supplied string information
AuCarExpErrorCode AuCarExportDLL::GetRequiredStringData(AuCarExpArray<AuCarExpUIStringData>& stringData, wchar_t const* locale)
{
	unsigned int stringDataCount = ExporterStringData_Count;

	ExportCustomParamsGroup exportParams;

	TCHAR path[MAX_PATH];
	if (SHGetFolderPathW(0, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_CURRENT, path) == S_OK)
	{
		std::wstring exportParamDir = path + AuExpManager::APPDATA_EXPORTER_DIR + L"\\UserFiles\\.ExportParams";
		exportParams = ExportCustomParamsManager::LoadParameterJSONs(exportParamDir);
		stringDataCount += exportParams.StringParams.size();
	}

	if (stringData.GetCount() != stringDataCount)
	{
		//Automation has not given us the number of items we asked for
		//(this should never happen)
		return AuCarExpErrorCode_UnknownError;
	}

	//set the values:
	wcscpy_s(stringData[ExporterStringData_FileName].Label, L"File Name");//label
	wcscpy_s(stringData[ExporterStringData_FileName].ToolTip, L"The .csv file the data will be put into");
	wcscpy_s(stringData[ExporterStringData_FileName].Value, L"[PlayerName] [PlatformName] [TrimName]");//default value

	//wcscpy_s(stringData[1].Label, L"Engine Name");//label
	//wcscpy_s(stringData[1].Value, L"");//default value, containing wildcards to be filled with information from Automation

	ExportPrefs prefs = ExportPrefs::LoadFromFile();

	//std::wstring delimiter = L"";
	//delimiter += prefs.Delimiter;
	wcscpy_s(stringData[ExporterStringData_CSVDelimiter].Label, L"CSV Delimiter"); //label
	//wcscpy_s(stringData[ExporterStringData_CSVDelimiter].Value, delimiter.c_str()); //default value, containing wildcards to be filled with information from Automation
	wcscpy_s(stringData[ExporterStringData_CSVDelimiter].ToolTip, L"The character to use as the delimiter for columns of data");
	stringData[ExporterStringData_CSVDelimiter].Value[0] = prefs.Delimiter;
	stringData[ExporterStringData_CSVDelimiter].Value[1] = L'\0';

	//std::wstring luaFile = prefs.LuaFile;
	wcscpy_s(stringData[ExporterStringData_LuaFileName].Label, L"Lua File");
	wcscpy_s(stringData[ExporterStringData_LuaFileName].ToolTip, L"The name of the Lua code file in the CSVExporter 2 user data folder. A .lua extension is added automatically if one is not provided. The default file is used if left blank. Multiple files names can be provided separated by commas, they will be loaded in the provided order");
	wcscpy_s(stringData[ExporterStringData_LuaFileName].Value, prefs.LuaFile);

	//std::wstring datasetFile = prefs.DataFile;
	wcscpy_s(stringData[ExporterStringData_DataJSONFileName].Label, L"Dataset File");
	wcscpy_s(stringData[ExporterStringData_DataJSONFileName].ToolTip, L"The name of the dataset JSON file in the CSVExporter 2 user data folder. A .json extension is added automatically if one is not provided. The default file is used if left blank");
	wcscpy_s(stringData[ExporterStringData_DataJSONFileName].Value, prefs.DataFile);

	//std::wstring translationsFile = prefs.TranslationsFile;
	wcscpy_s(stringData[ExporterStringData_TranslationsJSONFileName].Label, L"Translations File");
	wcscpy_s(stringData[ExporterStringData_TranslationsJSONFileName].ToolTip, L"The name of the translations JSON file in the CSVExporter 2 user data folder. A .json extension is added automatically if one is not provided. The default file is used if left blank");
	wcscpy_s(stringData[ExporterStringData_TranslationsJSONFileName].Value, prefs.TranslationsFile);

	//std::wstring localeName = prefs.Locale;
	wcscpy_s(stringData[ExporterStringData_Locale].Label, L"Locale");
	wcscpy_s(stringData[ExporterStringData_Locale].ToolTip, L"The system locale to use for data and text formatting. Can be used along with a custom delimiter to export decimal point numbers with commas as delimiters. Examples: en-US, en-GB, ru-RU");
	wcscpy_s(stringData[ExporterStringData_Locale].Value, prefs.Locale);

	std::unordered_map<std::wstring, std::wstring> exportParamSaveTable =
		ExportCustomStringParamPrefs::ToWStringMap(ExportCustomStringParamPrefs::LoadFromFile(AuExpManager::APPDATA_EXPORTER_PREFS_DIR));

	for (int i = 0; i < exportParams.StringParams.size(); i++)
	{
		wcscpy_s(stringData[ExporterStringData_Count + i].Label, exportParams.StringParams[i].Label.c_str());
		wcscpy_s(stringData[ExporterStringData_Count + i].ToolTip, exportParams.StringParams[i].Tooltip.c_str());
		wcscpy_s(stringData[ExporterStringData_Count + i].Value, exportParams.StringParams[i].Value.c_str());
		auto iterator = exportParamSaveTable.find(exportParams.StringParams[i].LuaName);
		if (iterator != exportParamSaveTable.end())
		{
			wcscpy_s(stringData[ExporterStringData_Count + i].Value, iterator->second.c_str());
		}
	}

	return AuCarExpErrorCode_Success;
}

//Gets the number of user-supplied booleans that the plugin will be requesting
AuCarExpErrorCode AuCarExportDLL::GetRequiredBoolDataCount(unsigned int* retCount)
{
	*retCount = ExporterBoolData_Count;

	TCHAR path[MAX_PATH];
	if (SHGetFolderPathW(0, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_CURRENT, path) == S_OK)
	{
		std::wstring exportParamDir = path + AuExpManager::APPDATA_EXPORTER_DIR + L"\\UserFiles\\.ExportParams";
		ExportCustomParamsGroup exportParams = ExportCustomParamsManager::LoadParameterJSONs(exportParamDir);
		*retCount += exportParams.BoolParams.size();
	}

	return AuCarExpErrorCode_Success;
}

//Gets the user-supplied boolean information
AuCarExpErrorCode AuCarExportDLL::GetRequiredBoolData(AuCarExpArray<AuCarExpUIBoolData>& boolData, wchar_t const* locale)
{
	unsigned int boolDataCount = ExporterBoolData_Count;

	ExportCustomParamsGroup exportParams;

	TCHAR path[MAX_PATH];
	if (SHGetFolderPathW(0, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_CURRENT, path) == S_OK)
	{
		std::wstring exportParamDir = path + AuExpManager::APPDATA_EXPORTER_DIR + L"\\UserFiles\\.ExportParams";
		exportParams = ExportCustomParamsManager::LoadParameterJSONs(exportParamDir);
		boolDataCount += exportParams.BoolParams.size();
	}

	if (boolData.GetCount() != boolDataCount)
	{
		//Automation has not given us the number of items we asked for
		//(this should never happen)
		return AuCarExpErrorCode_UnknownError;
	}

	std::unordered_map<std::wstring, bool> exportParamSaveTable =
		ExportCustomBoolParamPrefs::ToWStringMap(ExportCustomBoolParamPrefs::LoadFromFile(AuExpManager::APPDATA_EXPORTER_PREFS_DIR));

	for (int i = 0; i < exportParams.BoolParams.size(); i++)
	{
		wcscpy_s(boolData[ExporterBoolData_Count + i].Label, exportParams.BoolParams[i].Label.c_str());
		wcscpy_s(boolData[ExporterBoolData_Count + i].ToolTip, exportParams.BoolParams[i].Tooltip.c_str());
		boolData[ExporterBoolData_Count + i].Value = exportParams.BoolParams[i].Value;
		auto iterator = exportParamSaveTable.find(exportParams.BoolParams[i].LuaName);
		if (iterator != exportParamSaveTable.end())
		{
			boolData[ExporterBoolData_Count + i].Value = iterator->second;
		}
	}

	return AuCarExpErrorCode_Success;
}


AuCarExpErrorCode AuCarExportDLL::BeginExport(const AuCarExpCarData* carData, AuCarExpArray<wchar_t>& retDir, unsigned int* retFlags)
{
	AuExpManager::CreateInstance();
	AuCarExpErrorCode error = AuExpManager::GetInstance()->Init(carData);

	//tell Automation which directory the plugin will be exporting files to (so this can be displayed in a message to the user):
	wcscpy_s(retDir.GetData(), retDir.GetCount(), AuExpManager::GetInstance()->GetExportDirectory());

	//set the flags to none:
	*retFlags = AuCarExpExporterFlags_None;

	return error;
}

//Called after all data has been supplied to the export plugin, and all texture information has been finalised
AuCarExpErrorCode AuCarExportDLL::EndExport()
{
	//texture information is all good now, we can save the image files:
	AuExpManager::GetInstance()->SaveImages();
	AuExpManager::GetInstance()->EndExport();

	return AuCarExpErrorCode_Success;
}

//Called after EndExport(), to give the plugin the chance to finish any threaded operations that may still be running
AuCarExpErrorCode AuCarExportDLL::IsExportInProgress(bool* retInProgress)
{
	//*retInProgress = false;//set to true if there are still threads running
	*retInProgress = AuExpManager::GetInstance()->IsExportInProcess();

	return AuCarExpErrorCode_Success;
}

//Called after the export has finished, and IsExportInProgress() has reported nothing in progress.
//There will be no further function calls for this export.
//All outstanding plugin-allocated memory should be cleaned up here
AuCarExpErrorCode AuCarExportDLL::FreeAllData()
{
	//de-allocate everything:
	AuExpManager::DestroyInstance();

	return AuCarExpErrorCode_Success;
}


//Export the car body mesh
AuCarExpErrorCode AuCarExportDLL::AddBodyMesh(AuCarExpMesh* mesh)
{
	//AuExpManager::Instance()->SaveMesh(mesh, L"car_body");

	return AuCarExpErrorCode_Success;
}

//Export a (single) fixture
AuCarExpErrorCode AuCarExportDLL::AddFixtureMeshes(const AuCarExpArray<AuCarExpMesh*>& meshes, bool isBreakable, const wchar_t* name, const bool isTowBar, const AuCarExpVector& towPosition)
{
	//for (unsigned int i = 0; i < meshes.GetCount(); i++)
	//{
	//	AuExpManager::Instance()->SaveMesh(meshes[i], L"fixture");
	//}

	return AuCarExpErrorCode_Success;
}

//Export an engine mesh
AuCarExpErrorCode  AuCarExportDLL::AddEngineMesh(const AuCarExpMesh* mesh, const wchar_t* name)
{
	//AuExpManager::Instance()->SaveMesh(mesh, L"engine");

	return AuCarExpErrorCode_Success;
}

//Export an engine mesh whose mesh data is shared with another mesh
AuCarExpErrorCode  AuCarExportDLL::AddEngineMeshInstance(const AuCarExpMeshInstance* meshInstance)
{
	return AuCarExpErrorCode_Success;
}

//Export an exhaust line
//meshes will include piping, catalytic converter, mufflers and exhaust tip fixture (if present)
AuCarExpErrorCode  AuCarExportDLL::AddExhaust(const AuCarExpArray<AuCarExpMesh*>& meshes, const AuCarExpArray<AuCarExpVector>& points)
{
	//for (unsigned int i = 0; i < meshes.GetCount(); i++)
	//{
	//	AuExpManager::Instance()->SaveMesh(meshes[i], L"exhaust");
	//}

	return AuCarExpErrorCode_Success;
}

//Set the driver and bonnet camera positions
AuCarExpErrorCode  AuCarExportDLL::AddCameraPositions(const AuCarExpCameraData& driverCam, const AuCarExpCameraData& bonnetCam)
{
	return AuCarExpErrorCode_Success;
}

//Export a pair of wheels (either front or back), including the suspension meshes
AuCarExpErrorCode  AuCarExportDLL::AddWheelPair(const AuCarExpWheelData& wheelData, const bool isFront)
{
	//AuExpManager::Instance()->SaveMesh(wheelData.SuspensionMesh, isFront ? L"suspension_front" : L"suspension_");
	//AuExpManager::Instance()->SaveMesh(wheelData.RimMesh, isFront ? L"rim_front" : L"rim_rear");
	//AuExpManager::Instance()->SaveMesh(wheelData.TyreMesh, isFront ? L"tyre_front" : L"tyre_rear");
	//
	//for (int i = 0; i < 3; i++)
	//{
	//	AuExpManager::Instance()->SaveMesh(wheelData.BrakeMeshes[i], isFront ? L"brake_front" : L"brake_rear");
	//}

	return AuCarExpErrorCode_Success;
}

//Export the chassis meshes
AuCarExpErrorCode  AuCarExportDLL::AddChassis(const AuCarExpArray<AuCarExpMesh*>& meshes)
{
	//for (unsigned int i = 0; i < meshes.GetCount(); i++)
	//{
	//	AuExpManager::Instance()->SaveMesh(meshes[i], L"chassis");
	//}

	return AuCarExpErrorCode_Success;
}

//Export the stamp map
//See Readme.txt for more information
AuCarExpErrorCode AuCarExportDLL::SetStampTexture(AuCarExpTexture* stampTexture)
{
	//AuExpManager::Instance()->AddImage(stampTexture);

	return AuCarExpErrorCode_Success;
}




//Get render information required to generate a preview image
AuCarExpErrorCode AuCarExportDLL::GetPreviewImageData(unsigned int* retTextureWidth, unsigned int* retTextureHeight, float* retFOV)
{
	*retTextureWidth = 500;//required image width in pixels
	*retTextureHeight = 281;//required image height in pixels
	*retFOV = 17.0f;//required field of view

	return AuCarExpErrorCode_Success;
}

//Get camera transform information required to generate a preview image
AuCarExpErrorCode AuCarExportDLL::GetPreviewTransformData(const AuCarExpVector* carMin, const AuCarExpVector* carMax, AuCarExpVector* retPosition, AuCarExpVector* retLookAt)
{
	float carLength = carMax->z - carMin->z;
	float carHeight = carMax->y - carMin->y;

	/*

	retLookAt->x = (carMin->x + carMax->x) * 0.5f;
	retLookAt->y = (carMin->y + carMax->y) * 0.5f;
	retLookAt->z = (carMin->z + carMax->z) * 0.5f;

	retLookAt->z += 0.05f * carLength;
	retLookAt->y -= 0.02f * carHeight;

	*retPosition = *retLookAt;

	retPosition->x -= 1.65f * carLength;
	retPosition->z += 0.5f * carLength * 2.25f;

	retPosition->y += 0.3f * carHeight * 1.12f;

	*/

	retLookAt->x = (carMin->x + carMax->x) * 0.0f;
	retLookAt->y = (carMin->y + carMax->y) * 0.5f;
	retLookAt->z = (carMin->z + carMax->z) * 0.5f;

	//retLookAt->z += 0.05f * carLength;
	retLookAt->y -= 0.02f * carHeight;

	*retPosition = *retLookAt;

	retPosition->x -= 3.5f * carLength;
	//retPosition->z += 0.2f * carLength;
	//retPosition->z = (carMin->z + carMax->z) * 0.8f;

	//retPosition->y += 1.0f * carHeight;

	return AuCarExpErrorCode_Success;
}

//Export the generated preview image for the car
AuCarExpErrorCode AuCarExportDLL::AddPreviewImage(AuCarExpTexture* image)
{
	AuExpManager::GetInstance()->AddImage(image);
	return AuCarExpErrorCode_Success;
}


AuCarExpErrorCode AuCarExportDLL::AddLuaFiles(const AuCarExpArray<AuCarLuaDataFile>& Data)
{
	AuExpManager::GetInstance()->AddLuaFiles(Data);
	return AuCarExpErrorCode_Success;
}

AuCarExpErrorCode AuCarExportDLL::AddLuaFloatData(const AuCarExpArray<AuCarExpLuaFloatData>& Data)
{
	AuExpManager::GetInstance()->AddLuaFloatData(Data);
	return AuCarExpErrorCode_Success;
}

AuCarExpErrorCode AuCarExportDLL::AddLuaStringData(const AuCarExpArray<AuCarExpLuaStringData>& Data)
{
	AuExpManager::GetInstance()->AddLuaStringData(Data);
	return AuCarExpErrorCode_Success;
}

AuCarExpErrorCode AuCarExportDLL::GetLuaFileCount(unsigned int* fileCount)
{
	*fileCount = AuExpManager::GetInstance()->GetLuaFileCount();

	return AuCarExpErrorCode_Success;
}

AuCarExpErrorCode AuCarExportDLL::GetLuaFileLength(unsigned int* retLength, unsigned int FileNum)
{
	*retLength = AuExpManager::GetInstance()->GetLuaFileLength(FileNum);

	return AuCarExpErrorCode_Success;
}

AuCarExpErrorCode AuCarExportDLL::GetLuaFile(AuCarExpArray<wchar_t>& stringBuffer, unsigned int FileNum)
{
	if (!stringBuffer.GetData())
	{
		return AuCarExpErrorCode_UnknownError;
	}

	size_t size = AuExpManager::GetInstance()->GetLuaFileLength(FileNum);
	std::string data = AuExpManager::GetInstance()->GetLuaFile(FileNum);

	if ((size + 1) <= stringBuffer.GetCount())
	{
		for (unsigned int i = 0; i < size; i++)
		{
			stringBuffer[i] = data[i];
		}

		stringBuffer[size] = '\0';
	}

	//UnlockResource(hResourceLoaded);

	return AuCarExpErrorCode_Success;
}

//_declspec(dllimport) void CheckExporterVersion();
