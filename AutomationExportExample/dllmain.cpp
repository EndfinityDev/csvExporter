﻿/************************************************* *
** Copyright(c) 2019 -- Camshaft Software PTY LTD
** All Rights Reserved
************************************************** */


#include "stdafx.h"

#include "..\Include\AuCarExportDLL.h"

#include "EXPVERSION.h"
#include <ShlObj.h>
#include "ExportManager.h"
#include "CJSONReader.h"


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
	*VersionNumber = EXPVERSION;

	//CheckExporterVersion();

	return AuCarExpErrorCode_Success;
}

//Gets the number of user-supplied strings that the plugin will be requesting
AuCarExpErrorCode AuCarExportDLL::GetRequiredStringDataCount(unsigned int* retCount)
{
	//we will want to get 3 lots of string information from the user:
	*retCount = ExporterStringData_Count;

	return AuCarExpErrorCode_Success;
}

//Gets the user-supplied string information
AuCarExpErrorCode AuCarExportDLL::GetRequiredStringData(AuCarExpArray<AuCarExpUIStringData>& stringData, wchar_t const* locale)
{
	if (stringData.GetCount() != ExporterStringData_Count)
	{
		//Automation has not given us the number of items we asked for
		//(this should never happen)
		return AuCarExpErrorCode_UnknownError;
	}

	//set the values:
	wcscpy_s(stringData[ExporterStringData_FileName].Label, L"File Name");//label
	wcscpy_s(stringData[ExporterStringData_FileName].Value, L"[PlayerName] [PlatformName] [TrimName]");//default value

	//wcscpy_s(stringData[1].Label, L"Engine Name");//label
	//wcscpy_s(stringData[1].Value, L"");//default value, containing wildcards to be filled with information from Automation

	ExportPrefs prefs;

	TCHAR path[MAX_PATH];

	std::wstring prefsDir;

	//get the user's documents directory:
	if (SHGetFolderPathW(0, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_CURRENT, path) == S_OK)
	{
		prefsDir = path;
		prefsDir += L"\\AutomationGame\\ExporterPrefs\\";

		std::wstring prefsFile = prefsDir + L"CSVExporter2.prefs";

		FILE* file;
		errno_t err = _wfopen_s(&file, prefsFile.c_str(), L"rb");

		if (!err && file)
		{
			fseek(file, 0, SEEK_END);
			size_t prefsFileSize = ftell(file);
			fseek(file, 0, SEEK_SET);

			if (prefsFileSize == sizeof(prefs))
			{
				uint8_t prefsFileVersion;
				fread_s(&prefsFileVersion, sizeof(prefsFileVersion), sizeof(prefsFileVersion), 1, file);
				fseek(file, 0, SEEK_SET);

				if (prefsFileVersion == EXPPREFSVERSION)
				{
					fread_s(&prefs, sizeof(prefs), prefsFileSize, 1, file);
				}
			}

			fclose(file);
		}
	}

	std::wstring delimiter = L"";
	delimiter += prefs.Delimiter;

	wcscpy_s(stringData[ExporterStringData_CSVDelimiter].Label, L"CSV Delimiter"); //label
	wcscpy_s(stringData[ExporterStringData_CSVDelimiter].Value, delimiter.c_str()); //default value, containing wildcards to be filled with information from Automation

	std::wstring luaFile = prefs.LuaFile;

	wcscpy_s(stringData[ExporterStringData_LuaFileName].Label, L"Lua File");
	wcscpy_s(stringData[ExporterStringData_LuaFileName].Value, luaFile.c_str());

	std::wstring datasetFile = prefs.DataFile;

	wcscpy_s(stringData[ExporterStringData_DataJSONFileName].Label, L"Dataset File");
	wcscpy_s(stringData[ExporterStringData_DataJSONFileName].Value, datasetFile.c_str());

	std::wstring translationsFile = prefs.TranslationsFile;

	wcscpy_s(stringData[ExporterStringData_TranslationsJSONFileName].Label, L"Translations File");
	wcscpy_s(stringData[ExporterStringData_TranslationsJSONFileName].Value, translationsFile.c_str());

	std::wstring localeName = prefs.Locale;

	wcscpy_s(stringData[ExporterStringData_Locale].Label, L"Locale");
	wcscpy_s(stringData[ExporterStringData_Locale].Value, localeName.c_str());

	/*
	wcscpy_s(stringData[2].Label, L"Cost preset (0-13)");//label
	//wcscpy_s(stringData[2].Value, L"0");

	bool readFromFile = true;
	TCHAR path[MAX_PATH];
	if (SHGetFolderPathW(0, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_CURRENT, path) == S_OK)
	{
		std::wstring prefsFilePath = path;
		prefsFilePath += L"\\AutomationGame\\ExporterPrefs\\csvExporterPreset.prefs";
		FILE* prefsFile;
		_wfopen_s(&prefsFile, prefsFilePath.c_str(), L"rb");

		if (prefsFile)
		{
			fseek(prefsFile, 0, SEEK_END);
			size_t fileSize = ftell(prefsFile);
			fseek(prefsFile, 0, SEEK_SET);

			if (fileSize == sizeof(ExportPrefs))
			{
				ExportPrefs prefs;
				fread_s(&prefs, sizeof(prefs), fileSize, 1, prefsFile);
				//if (prefs.PrefsFlags > 13)
				//	prefs.PrefsFlags = 0;
				wcscpy_s(stringData[2].Value, std::to_wstring(prefs.PrefsFlags).c_str());
			}
			else
				readFromFile = false;

			fclose(prefsFile);
		}
		else
			readFromFile = false;
	}
	else
		readFromFile = false;

	if (!readFromFile)
	{
		wcscpy_s(stringData[2].Value, L"0");
	}
	*/

	return AuCarExpErrorCode_Success;
}

//Gets the number of user-supplied booleans that the plugin will be requesting
AuCarExpErrorCode AuCarExportDLL::GetRequiredBoolDataCount(unsigned int* retCount)
{
	//we will want to get 2 booleans from the user:
	*retCount = ExporterBoolData_Count;

	return AuCarExpErrorCode_Success;
}

//Gets the user-supplied boolean information
AuCarExpErrorCode AuCarExportDLL::GetRequiredBoolData(AuCarExpArray<AuCarExpUIBoolData>& boolData, wchar_t const* locale)
{
	/*
	if (boolData.GetCount() != 2)
	{
		//Automation has not given us the number of items we asked for
		//(this should never happen)
		return AuCarExpErrorCode_UnknownError;
	}

	//set the value:
	wcscpy_s(boolData[0].Label, L"Export Engine Separately");//label
	//boolData[0].Value = false;//default value

	wcscpy_s(boolData[1].Label, L"Export Results Separately");//label
	//boolData[1].Value = false;//default value

	bool readFromFile = true;
	TCHAR path[MAX_PATH];
	if (SHGetFolderPathW(0, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_CURRENT, path) == S_OK)
	{
		std::wstring prefsFilePath = path;
		prefsFilePath += L"\\AutomationGame\\ExporterPrefs\\csvExporter.prefs";
		FILE* prefsFile;
		_wfopen_s(&prefsFile, prefsFilePath.c_str(), L"rb");

		if (prefsFile)
		{
			fseek(prefsFile, 0, SEEK_END);
			size_t fileSize = ftell(prefsFile);
			fseek(prefsFile, 0, SEEK_SET);

			if (fileSize == sizeof(ExportPrefs))
			{
				ExportPrefs prefs;
				fread_s(&prefs, sizeof(prefs), fileSize, 1, prefsFile);
				boolData[0].Value = prefs.PrefsFlags & 0b00000001;
				boolData[1].Value = prefs.PrefsFlags & 0b00000010;
			}
			else
				readFromFile = false;

			fclose(prefsFile);
		}
		else
			readFromFile = false;
	}
	else
		readFromFile = false;

	if (!readFromFile)
	{
		boolData[0].Value = false;
		boolData[1].Value = false;
	}
	*/

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
	*fileCount = 1;

	return AuCarExpErrorCode_Success;
}

AuCarExpErrorCode AuCarExportDLL::GetLuaFileLength(unsigned int* retLength, unsigned int FileNum)
{
	//*retLength = 0;

	/*
	HRSRC   hRes;              // handle/ptr to res. info.

	HMODULE module = GetModuleHandle(PROJECT_FILENAME);

	hRes = FindResource(module, MAKEINTRESOURCE(IDR_LUA_FILE), TEXT("BINARY"));

	if (!hRes)
	{
		return AuCarExpErrorCode_UnknownError;
	}

	unsigned int size = SizeofResource(module, hRes);

	*retLength = size + 1; //size in chars (what we need) is the byte size. We add one for a null terminator
	*/

	*retLength = AuExpManager::GetInstance()->GetLuaFileLength();

	return AuCarExpErrorCode_Success;
}

AuCarExpErrorCode AuCarExportDLL::GetLuaFile(AuCarExpArray<wchar_t>& stringBuffer, unsigned int FileNum)
{
	if (!stringBuffer.GetData())
	{
		return AuCarExpErrorCode_UnknownError;
	}

	/*
	HGLOBAL hResourceLoaded;  // handle to loaded resource
	HRSRC   hRes;              // handle/ptr to res. info.

	HMODULE module = GetModuleHandle(PROJECT_FILENAME);

	hRes = FindResource(module, MAKEINTRESOURCE(IDR_LUA_FILE), TEXT("BINARY"));

	if (!hRes)
	{
		return AuCarExpErrorCode_UnknownError;
	}

	unsigned int size = SizeofResource(module, hRes);

	hResourceLoaded = LoadResource(module, hRes);
	char* data = (char*)LockResource(hResourceLoaded);
	*/

	size_t size = AuExpManager::GetInstance()->GetLuaFileLength();
	std::string data = AuExpManager::GetInstance()->GetLuaFile();

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
