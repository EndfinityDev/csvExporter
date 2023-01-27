/************************************************* *
** Copyright(c) 2021 -- Camshaft Software PTY LTD
** All Rights Reserved
************************************************** */

#include "stdafx.h"
#include <Shlobj.h>
#include "ExportPrefs.h"

size_t FindDirDelimiter(std::wstring dir, size_t start)
{
	size_t slashPos = dir.find(L"\\", start);

	if (slashPos == std::wstring::npos)
	{
		return dir.find(L"/", start);
	}

	return slashPos;
}




AuExpManager* AuExpManager::s_Instance = nullptr;

AuExpManager::AuExpManager() :
	m_ExportDirectory(L"")
{
	s_Instance = this;

	//PopulateExportDataHeader();

	BuildTranslations();
}


AuCarExpErrorCode AuExpManager::Init(const AuCarExpCarData* carData)
{
	TCHAR path[MAX_PATH];

	m_CarData = carData;

	//get the user's documents directory:
	if (SHGetFolderPathW(0, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_CURRENT, path) == S_OK)
	{
		m_ExportDirectory = path;
		m_ExportDirectory += L"\\AutomationGame\\csvExporter";
	}
	else
	{
		return AuCarExpErrorCode_CouldNotObtainOutputPathFatal;
	}


	m_ExportDirectory += L"\\";
	//m_ExportDirectory += carData->GetCarName();//TODO: sanitise filename

	// Sanitizing Filename
	std::wstring exportFileName = m_CarData->GetStringData(0)->Value;

	for (std::wstring::iterator i = exportFileName.begin(); i < exportFileName.end(); i++)
	{
		if (IllegalCharacters.find(*i) != std::wstring::npos)
		{
			*i = L'_';
		}
	}

	if (exportFileName[exportFileName.size() - 1] == L' ')
	{
		exportFileName.resize(exportFileName.size() - 1);
	}

	//m_ExportDirectory += m_CarData->GetStringData(0)->Value;
	m_ExportDirectory += exportFileName;

	//ensure target directory exists:
	DWORD att = GetFileAttributes(m_ExportDirectory.c_str());

	if (att == INVALID_FILE_ATTRIBUTES)
	{
		//create directory, one level at a time:
		size_t slashPos = FindDirDelimiter(m_ExportDirectory, 0);
		size_t offset = 0;

		while (slashPos != std::wstring::npos)
		{
			CreateDirectory(m_ExportDirectory.substr(offset, slashPos - offset).c_str(), nullptr);
			slashPos = FindDirDelimiter(m_ExportDirectory, slashPos + 1);
		}

		//last one:
		CreateDirectory(m_ExportDirectory.c_str(), nullptr);

		att = GetFileAttributes(m_ExportDirectory.c_str());
	}

	if (att != INVALID_FILE_ATTRIBUTES && att & FILE_ATTRIBUTE_DIRECTORY)
	{
		//directory exists, all good to go:
		return AuCarExpErrorCode_Success;
	}
	else
	{
		std::wstring error = L"Could not create directory: ";
		error += m_ExportDirectory;

		MessageBox(nullptr, error.c_str(), TEXT("Error creating directory"), MB_OK);

		return AuCarExpErrorCode_CouldNotObtainOutputPathFatal;
	}

}

void AuExpManager::EndExport() 
{
	m_IsExportInProcess = true;

	PopulateExportDataHeader();

	ExportInternal();

	TCHAR path[MAX_PATH];
	if (SHGetFolderPathW(0, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_CURRENT, path) == S_OK)
	{
		std::wstring prefsFilePath = path;
		prefsFilePath += L"\\AutomationGame\\ExporterPrefs";

		DWORD att = GetFileAttributes(prefsFilePath.c_str());

		if (att == INVALID_FILE_ATTRIBUTES)
		{
			//create directory, one level at a time:
			size_t slashPos = FindDirDelimiter(prefsFilePath, 0);
			size_t offset = 0;

			while (slashPos != std::wstring::npos)
			{
				CreateDirectory(prefsFilePath.substr(offset, slashPos - offset).c_str(), nullptr);
				slashPos = FindDirDelimiter(prefsFilePath, slashPos + 1);
			}

			//last one:
			CreateDirectory(prefsFilePath.c_str(), nullptr);

			att = GetFileAttributes(prefsFilePath.c_str());
		}

		if (att != INVALID_FILE_ATTRIBUTES && att & FILE_ATTRIBUTE_DIRECTORY)
		{
			//directory exists, all good to go:

			prefsFilePath += L"\\csvExporter.prefs";

			FILE* prefsFile;
			_wfopen_s(&prefsFile, prefsFilePath.c_str(), L"wb");

			if (prefsFile)
			{
				ExportPrefs prefs;
				prefs.PrefsVersion = EXPPREFSVERSION;
				prefs.PrefsFlags = 0b00000000 | (m_CarData->GetBoolData(0)->Value << 0) | (m_CarData->GetBoolData(0)->Value << 1);

				fwrite(&prefs, sizeof(prefs), 1, prefsFile);

				fclose(prefsFile);
			}
		}
	}

	m_IsExportInProcess = false;
}

void AuExpManager::ExportInternal()
{
	bool isEngineSeparate = m_CarData->GetBoolData(0)->Value;
	bool areChoicesSeparate = m_CarData->GetBoolData(1)->Value;

	m_FileName = m_CarData->GetStringData(0)->Value;

	for (std::wstring::iterator i = m_FileName.begin(); i < m_FileName.end(); i++)
	{
		if (IllegalCharacters.find(*i) != std::wstring::npos)
		{
			*i = L'_';
		}
	}
	
	m_Delimiter = m_CarData->GetStringData(1)->Value;


	if (!isEngineSeparate) 
	{
		if (!areChoicesSeparate) 
		{
			BuildSingleCSV();
		}
		else
		{
			BuildResultAndChoiceCSVs();
		}
	}
	else
	{
		if (!areChoicesSeparate)
		{
			BuildCarAndEngineCSVs();
		}
		else
		{
			BuildCarAndEngineChoicesAndResultsCSVs();
		}
	}
	//return true;
}

void AuExpManager::BuildCarAndEngineChoicesAndResultsCSVs()
{
	LuaData exportCarChoiceData;
	exportCarChoiceData.Add(m_GeneralData);
	exportCarChoiceData.Add(m_CarChoicesData);

	LuaData exportCarResultData;
	exportCarResultData.Add(m_GeneralData);
	exportCarResultData.Add(m_CarResultsData);

	LuaData exportEngineChoiceData;
	exportEngineChoiceData.Add(m_GeneralData);
	exportEngineChoiceData.Add(m_EngineChoicesData);

	LuaData exportEngineResultData;
	exportEngineResultData.Add(m_GeneralData);
	exportEngineResultData.Add(m_EngineResultsData);

	FillTableData(exportCarChoiceData);
	FillTableData(exportCarResultData);
	FillTableData(exportEngineChoiceData);
	FillTableData(exportEngineResultData);

	CSVManager csvCarChoicesManager(m_FileName + L"_Car_Choices.csv", m_Delimiter);

	csvCarChoicesManager.AddData(exportCarChoiceData);
	csvCarChoicesManager.Save();

	CSVManager csvCarResultsManager(m_FileName + L"_Car_Results.csv", m_Delimiter);

	csvCarResultsManager.AddData(exportCarResultData);
	csvCarResultsManager.Save();

	CSVManager csvEngineChoicesManager(m_FileName + L"_Engine_Choices.csv", m_Delimiter);

	csvEngineChoicesManager.AddData(exportEngineChoiceData);
	csvEngineChoicesManager.Save();

	CSVManager csvEngineResultsManager(m_FileName + L"_Engine_Results.csv", m_Delimiter);

	csvEngineResultsManager.AddData(exportEngineResultData);
	csvEngineResultsManager.Save();
}

void AuExpManager::BuildCarAndEngineCSVs()
{
	LuaData exportCarData;
	exportCarData.Add(m_GeneralData);
	exportCarData.Add(m_CarChoicesData);
	exportCarData.Add(m_CarResultsData);

	LuaData exportEngineData;
	exportEngineData.Add(m_GeneralData);
	exportEngineData.Add(m_EngineChoicesData);
	exportEngineData.Add(m_EngineResultsData);


	FillTableData(exportCarData);
	FillTableData(exportEngineData);

	CSVManager csvCarManager(m_FileName + L"_Car.csv", m_Delimiter);

	csvCarManager.AddData(exportCarData);
	csvCarManager.Save();

	CSVManager csvEngineManager(m_FileName + L"_Engine.csv", m_Delimiter);

	csvEngineManager.AddData(exportEngineData);
	csvEngineManager.Save();
}

void AuExpManager::BuildResultAndChoiceCSVs()
{
	LuaData exportDataChoices;
	LuaData exportDataResults;
	exportDataChoices.Add(m_GeneralData);
	exportDataChoices.Add(m_CarChoicesData);
	exportDataChoices.Add(m_EngineChoicesData);

	exportDataResults.Add(m_GeneralData);
	exportDataResults.Add(m_CarResultsData);
	exportDataResults.Add(m_EngineResultsData);

	FillTableData(exportDataChoices);
	FillTableData(exportDataResults);

	CSVManager csvManagerChoices(m_FileName + L"_Choices.csv", m_Delimiter);
	CSVManager csvManagerResults(m_FileName + L"_Results.csv", m_Delimiter);

	csvManagerChoices.AddData(exportDataChoices);
	csvManagerChoices.Save();

	csvManagerResults.AddData(exportDataResults);
	csvManagerResults.Save();
}

void AuExpManager::BuildSingleCSV()
{
	LuaData exportData;
	exportData.Add(m_GeneralData);
	exportData.Add(m_CarChoicesData);
	exportData.Add(m_CarResultsData);
	exportData.Add(m_EngineChoicesData);
	exportData.Add(m_EngineResultsData);

	FillTableData(exportData);

	CSVManager csvManager(m_FileName + L".csv", m_Delimiter);

	csvManager.AddData(exportData);
	csvManager.Save();
}

void AuExpManager::FillTableData(LuaData& data)
{
	std::wstring ret;

	for (uint32_t i = 0; i < data.Size(); i++)
	{
		ret = L"???";

		switch (data[i].DataType)
		{
		case DataType_String:
			TryGetStringData(data[i].HeaderName, ret);
			break;

		case DataType_Float:
			TryGetFloatDataString(data[i].HeaderName, &ret);
			break;

		case DataType_ExporterVersion:
			ret = EXPVERSIONWS;
			break;

		case DataType_CarName:
			ret = m_CarData->GetCarName();
			break;

		default:
			break;
		}
		GetTranslation(&ret);
		data[i].SetData(ret);
	}
}

void AuExpManager::AddImage(const AuCarExpTexture* texture)
{
	if (!texture)
	{
		return;
	}

	//only add unique textures
	for (unsigned int i = 0; i < m_AllImages.size(); i++)
	{
		if (m_AllImages[i] == texture)
		{
			return;
		}
	}

	m_AllImages.push_back(texture);
}

void AuExpManager::SaveImages()
{
	for (unsigned int i = 0; i < m_AllImages.size(); i++)
	{
		AuExpImage::SaveImageFile(m_AllImages[i], m_ExportDirectory.c_str());
	}
}

void AuExpManager::SaveMesh(const AuCarExpMesh* mesh, const wchar_t* name)
{
	return;
}

void AuExpManager::AddLuaFloatData(const AuCarExpArray<AuCarExpLuaFloatData>& Data)
{
	for (int i = 0; i < Data.GetCount(); i++)
	{
		m_LuaFloatData[Data[i].ValueName] = Data[i].Value;
	}
}

void AuExpManager::AddLuaStringData(const AuCarExpArray<AuCarExpLuaStringData>& Data)
{
	for (int i = 0; i < Data.GetCount(); i++)
	{
		m_LuaStringData[Data[i].ValueName] = Data[i].Buffer;
	}
}

bool AuExpManager::TryGetFloatData(const std::wstring& key, float* retValue)
{
	if (m_LuaFloatData.find(key) == m_LuaFloatData.end())
	{
		return false;
	}

	*retValue = m_LuaFloatData[key];
	return true;
}

bool AuExpManager::TryGetFloatDataString(const std::wstring& key, std::wstring* retValue)
{
	float val = 0.0f;
	if (AuExpManager::Instance()->TryGetFloatData(key, &val))
	{
		wchar_t valStr[64];
		valStr[63] = '\0';

		swprintf_s(valStr, 63, L"%f", val);
		*retValue = valStr;

		return true;
	}

	return false;
}

bool AuExpManager::TryGetStringData(const std::wstring& key, std::wstring& value)
{
	if (m_LuaStringData.find(key) == m_LuaStringData.end())
	{
		return false;
	}

	value = m_LuaStringData[key];
	return true;
}

void AuExpManager::AddLuaFiles(const AuCarExpArray<AuCarLuaDataFile>& data)
{
	FileManager fileManager;

	for (int i = 0; i < data.GetCount(); i++)
	{
		m_LuaFDataFiles[data[i].FileName] = data[i];

		AuCarLuaDataFile& File = data[i];

		std::wstring FileName = File.FileName;
		//FileName = TextUtils::SanitiseFileName(FileName);
		//FILE* OutFile; //= m_FileManager->OpenFile(m_Directory + L"/" + FileName, L"wb");
		FILE* OutFile = fileManager.OpenFile((FileName).c_str(), L"wb");
		if (!OutFile)
		{
			return;
		}

		fwrite(File.Buffer, 1, File.BufferSize, OutFile);

	}
}

void AuExpManager::PopulateExportDataHeader()
{
	m_GeneralData.Add(L"Exporter Version", L"ExporterVersion", DataType_ExporterVersion);
	m_GeneralData.Add(L"Game Version", L"GameVersion", DataType_Float);
	m_GeneralData.Add(L"Car Name", L"CarName", DataType_CarName);

	m_CarChoicesData.Add(L"Model Name", L"ModelName", DataType_String);
	m_CarChoicesData.Add(L"Model Year", L"ModelYear", DataType_Float);
	m_CarChoicesData.Add(L"Wheelbase", L"BodyWheelbase", DataType_Float);
	m_CarChoicesData.Add(L"Chassis Type", L"ChassisType", DataType_String);
	m_CarChoicesData.Add(L"Chassis Material", L"ChassisMat", DataType_String);
	m_CarChoicesData.Add(L"Panel Material", L"PanelMat", DataType_String);
	m_CarChoicesData.Add(L"Front Suspension", L"SuspensionFront", DataType_String);
	m_CarChoicesData.Add(L"Rear Suspension", L"SuspensionRear", DataType_String);
	m_CarChoicesData.Add(L"Engine Placement", L"EnginePlacement", DataType_String);
	m_CarChoicesData.Add(L"Engine Orientation", L"EngineOrientation", DataType_String);

	m_CarChoicesData.Add(L"Model Body Quality", L"ModelQualityBody", DataType_Float);
	m_CarChoicesData.Add(L"Model Chassis Quality", L"ModelQualityChassis", DataType_Float);

	m_CarChoicesData.Add(L"Model Body Techpool", L"ModelTechpoolBody", DataType_Float);
	m_CarChoicesData.Add(L"Model Chassis Techpool", L"ModelTechpoolChassis", DataType_Float);

	m_CarChoicesData.Add(L"Trim Name", L"TrimName", DataType_String);
	m_CarChoicesData.Add(L"Trim Year", L"TrimYear", DataType_Float);

	//m_CarChoicesData.Add(	L"Trim Chassis Quality",		L"TrimQualityChassis",			DataType_Float);
	m_CarChoicesData.Add(L"Trim Body Quality", L"TrimQualityBody", DataType_Float);
	m_CarChoicesData.Add(L"Trim Aerodynamics Quality", L"TrimQualityAerodynamics", DataType_Float);
	m_CarChoicesData.Add(L"Trim Interior Quality", L"TrimQualityInterior", DataType_Float);
	m_CarChoicesData.Add(L"Trim Fixture Quality", L"TrimQualityFixtures", DataType_Float);
	m_CarChoicesData.Add(L"Trim Tyre Quality", L"TrimQualityTyres", DataType_Float);
	m_CarChoicesData.Add(L"Trim Suspension Quality", L"TrimQualitySuspension", DataType_Float);
	m_CarChoicesData.Add(L"Trim Safety Quality", L"TrimQualitySafety", DataType_Float);
	m_CarChoicesData.Add(L"Trim Brake Quality", L"TrimQualityBrakes", DataType_Float);
	m_CarChoicesData.Add(L"Trim Assist Quality", L"TrimQualityAssists", DataType_Float);
	m_CarChoicesData.Add(L"Trim Drivetrain Quality", L"TrimQualityDrivetrain", DataType_Float);

	m_CarChoicesData.Add(L"Trim Chassis Techpool", L"TrimTechpoolChassis", DataType_Float);
	m_CarChoicesData.Add(L"Trim Body Techpool", L"TrimTechpoolBody", DataType_Float);
	m_CarChoicesData.Add(L"Trim Aerodynamics Techpool", L"TrimTechpoolAerodynamics", DataType_Float);
	m_CarChoicesData.Add(L"Trim Interior Techpool", L"TrimTechpoolInterior", DataType_Float);
	m_CarChoicesData.Add(L"Trim Fixture Techpool", L"TrimTechpoolFixtures", DataType_Float);
	m_CarChoicesData.Add(L"Trim Tyre Techpool", L"TrimTechpoolTyres", DataType_Float);
	m_CarChoicesData.Add(L"Trim Suspension Techpool", L"TrimTechpoolSuspension", DataType_Float);
	m_CarChoicesData.Add(L"Trim Safety Techpool", L"TrimTechpoolSafety", DataType_Float);
	m_CarChoicesData.Add(L"Trim Brake Techpool", L"TrimTechpoolBrakes", DataType_Float);
	m_CarChoicesData.Add(L"Trim Assist Techpool", L"TrimTechpoolAssists", DataType_Float);
	m_CarChoicesData.Add(L"Trim Drivetrain Techpool", L"TrimTechpoolDrivetrain", DataType_Float);

	m_CarChoicesData.Add(L"Body Type", L"BodyType", DataType_String);
	m_CarChoicesData.Add(L"Doors", L"BodyDoors", DataType_Float);
	m_CarChoicesData.Add(L"Body Name", L"BodyName", DataType_String);
	m_CarChoicesData.Add(L"Convertible Type", L"ConvertibleType", DataType_String);

	m_CarChoicesData.Add(L"Full 1st Row Seats", L"FrontSeatsFull", DataType_Float);
	m_CarChoicesData.Add(L"Small 1st Row Seats", L"FrontSeatsTemp", DataType_Float);
	m_CarChoicesData.Add(L"Full 2nd Row Seats", L"MidSeatsFull", DataType_Float);
	m_CarChoicesData.Add(L"Small 2nd Row Seats", L"MidSeatsTemp", DataType_Float);
	m_CarChoicesData.Add(L"Full 3rd Row Seats", L"RearSeatsFull", DataType_Float);
	m_CarChoicesData.Add(L"Small 3rd Row Seats", L"RearSeatsTemp", DataType_Float);

	m_CarChoicesData.Add(L"Drive Type", L"DriveType", DataType_String);
	m_CarChoicesData.Add(L"Gearbox Type", L"GearboxType", DataType_String);
	m_CarChoicesData.Add(L"Gear Count", L"GearCount", DataType_Float);
	m_CarChoicesData.Add(L"Gear Spacing", L"GearSpacing", DataType_Float);
	m_CarChoicesData.Add(L"Power To Front", L"PowerDistribution", DataType_Float);

	m_CarChoicesData.Add(L"Differential Type", L"DiffType", DataType_String);
	m_CarChoicesData.Add(L"Final Drive", L"FinalDrive", DataType_Float);

	m_CarChoicesData.Add(L"Speed Limiter", L"SpeedLimit", DataType_Float);

	m_CarChoicesData.Add(L"Active Cooling", L"ActiveCooling", DataType_String);
	m_CarChoicesData.Add(L"Cooling Amount", L"Cooling", DataType_Float);
	m_CarChoicesData.Add(L"Undertray", L"UnderTray", DataType_String);
	m_CarChoicesData.Add(L"Active Aero", L"ActiveWing", DataType_String);
	m_CarChoicesData.Add(L"Rear Wing Angle", L"WingAngleRear", DataType_Float);
	m_CarChoicesData.Add(L"Front Wing Angle", L"WingAngleFront", DataType_Float);

	m_CarChoicesData.Add(L"Interior", L"Interior", DataType_String);
	m_CarChoicesData.Add(L"Entertainment", L"Entertainment", DataType_String);
	m_CarChoicesData.Add(L"Power Steering", L"PowerSteering", DataType_String);

	m_CarChoicesData.Add(L"Assists", L"AssistPackage", DataType_String);
	m_CarChoicesData.Add(L"Safety", L"Safety", DataType_String);

	m_CarChoicesData.Add(L"Springs", L"Springs", DataType_String);
	m_CarChoicesData.Add(L"Front Spring Stiffness", L"FrontSpringStiffness", DataType_Float);
	m_CarChoicesData.Add(L"Rear Spring Stiffness", L"RearSpringStiffness", DataType_Float);

	m_CarChoicesData.Add(L"Dampers", L"Dampers", DataType_String);
	m_CarChoicesData.Add(L"Front Damper Stiffness", L"FrontDamperStiffness", DataType_Float);
	m_CarChoicesData.Add(L"Rear Damper Stiffness", L"RearDamperStiffness", DataType_Float);

	m_CarChoicesData.Add(L"Sway Bars", L"SwayBars", DataType_String);
	m_CarChoicesData.Add(L"Front Sway Bar Stiffness", L"FrontARBStiffness", DataType_Float);
	m_CarChoicesData.Add(L"Rear Sway Bar Stiffness", L"RearARBStiffness", DataType_Float);

	m_CarChoicesData.Add(L"Ride Height", L"RideHeight", DataType_Float);
	m_CarChoicesData.Add(L"Front Camber", L"FrontCamber", DataType_Float);
	m_CarChoicesData.Add(L"Rear Camber", L"RearCamber", DataType_Float);

	m_CarChoicesData.Add(L"Rim Material", L"RimMaterial", DataType_String);
	m_CarChoicesData.Add(L"Tyre Type", L"TyreType", DataType_String);
	m_CarChoicesData.Add(L"Tyre Compound", L"TyreCompound", DataType_String);

	m_CarChoicesData.Add(L"Front Rim Size", L"FrontRimSize", DataType_Float);
	m_CarChoicesData.Add(L"Rear Rim Size", L"RearRimSize", DataType_Float);
	m_CarChoicesData.Add(L"Front Wheel Offset", L"FrontWheelOffset", DataType_Float);
	m_CarChoicesData.Add(L"Rear Wheel Offset", L"RearWheelOffset", DataType_Float);
	m_CarChoicesData.Add(L"Front Tyre Profile", L"FrontTyreProfile", DataType_Float);
	m_CarChoicesData.Add(L"Rear Tyre Profile", L"RearTyreProfile", DataType_Float);
	m_CarChoicesData.Add(L"Front Tyre Width", L"FrontTyreWidth", DataType_Float);
	m_CarChoicesData.Add(L"Rear Tyre Width", L"RearTyreWidth", DataType_Float);
	m_CarChoicesData.Add(L"Front Wheel Diameter", L"FrontOverallDiameter", DataType_Float);
	m_CarChoicesData.Add(L"Rear Wheel Diameter", L"RearOverallDiameter", DataType_Float);

	m_CarChoicesData.Add(L"Front Wheel Diameter", L"FrontOverallDiameter", DataType_Float);
	m_CarChoicesData.Add(L"Rear Wheel Diameter", L"RearOverallDiameter", DataType_Float);

	m_CarChoicesData.Add(L"Front Brake Type", L"FrontBrakeType", DataType_String);
	m_CarChoicesData.Add(L"Rear Brake Type", L"RearBrakeType", DataType_String);
	m_CarChoicesData.Add(L"Front Caliper Pistons", L"FrontCaliperPistons", DataType_Float);
	m_CarChoicesData.Add(L"Rear Caliper Pistons", L"RearCaliperPistons", DataType_Float);

	m_CarChoicesData.Add(L"Front Brake Diameter", L"FrontBrakeDiameter", DataType_Float);
	m_CarChoicesData.Add(L"Rear Brake Diameter", L"RearBrakeDiameter", DataType_Float);
	m_CarChoicesData.Add(L"Front Brake Pad Type", L"FrontBrakePadType", DataType_Float);
	m_CarChoicesData.Add(L"Rear Brake Pad Type", L"RearBrakePadType", DataType_Float);
	m_CarChoicesData.Add(L"Front Brake Force", L"FrontBrakeForce", DataType_Float);
	m_CarChoicesData.Add(L"Rear Brake Force", L"RearBrakeForce", DataType_Float);

	m_CarChoicesData.Add(L"Weight Optimization Tune", L"WeightOptimizationTune", DataType_Float);
	m_CarChoicesData.Add(L"Weight Distribution Tune", L"WeightDistributionTune", DataType_Float);



	m_CarResultsData.Add(L"Trim Weight", L"CarWeight", DataType_Float);
	m_CarResultsData.Add(L"Tow Weight", L"TowWeight", DataType_Float);

	m_CarResultsData.Add(L"Body Stiffness", L"BodyStiffness", DataType_Float);

	m_CarResultsData.Add(L"Sportiness Rating", L"SportinessRating", DataType_Float);
	m_CarResultsData.Add(L"Drivability Rating", L"DrivabilityRating", DataType_Float);
	m_CarResultsData.Add(L"Utility Rating", L"UtilityRating", DataType_Float);
	m_CarResultsData.Add(L"Environmental Resistance", L"EnvResistance", DataType_Float);
	m_CarResultsData.Add(L"Trim Emissions", L"Emissions", DataType_Float);
	m_CarResultsData.Add(L"Offroad Rating", L"OffroadRating", DataType_Float);
	m_CarResultsData.Add(L"Prestige Rating", L"PrestigeRating", DataType_Float);
	m_CarResultsData.Add(L"Trim Reliability", L"ReliabilityRating", DataType_Float);
	m_CarResultsData.Add(L"Trim Economy", L"Economy", DataType_Float);
	m_CarResultsData.Add(L"Practicality Rating", L"PracticalityRating", DataType_Float);
	m_CarResultsData.Add(L"Safety Rating", L"SafetyRating", DataType_Float);
	m_CarResultsData.Add(L"Comfort Rating", L"ComfortRating", DataType_Float);

	m_CarResultsData.Add(L"Passenger Volume", L"PassengerVolume", DataType_Float);
	m_CarResultsData.Add(L"Cargo Volume", L"CargoVolume", DataType_Float);

	m_CarResultsData.Add(L"Trim Engineering Time", L"TrimEngineeringTime", DataType_Float);
	m_CarResultsData.Add(L"Trim Production Units", L"TrimManHours", DataType_Float);

	m_CarResultsData.Add(L"Trim Price", L"TrimPrice", DataType_Float);

	m_CarResultsData.Add(L"Trim Tooling Costs", L"ToolingCosts", DataType_Float);
	m_CarResultsData.Add(L"Trim Service Costs", L"ServiceCosts", DataType_Float);
	m_CarResultsData.Add(L"Trim Total Costs", L"TotalTrimCosts", DataType_Float);
	m_CarResultsData.Add(L"Trim Cost", L"TrimCost", DataType_Float);
	m_CarResultsData.Add(L"Trim Material Cost", L"MatCost", DataType_Float);
	m_CarResultsData.Add(L"Trim Engineering Costs", L"TrimEngCosts", DataType_Float);

	m_CarResultsData.Add(L"Braking Distance", L"BrakingDistance", DataType_Float);
	m_CarResultsData.Add(L"Max Body Roll", L"MaxBodyRoll", DataType_Float);
	m_CarResultsData.Add(L"Cornering", L"Cornering", DataType_Float);

	m_CarResultsData.Add(L"Front Downforce", L"FrontDownforce", DataType_Float);
	m_CarResultsData.Add(L"Rear Downforce", L"RearDownforce", DataType_Float);

	m_CarResultsData.Add(L"Tyre Speed Index", L"TyreSpeedIndex", DataType_String);
	m_CarResultsData.Add(L"Tyre Speed Rating (km/h)", L"TyreSpeedRating", DataType_Float);

	m_CarResultsData.Add(L"Top Speed (km/h)", L"TopSpeed", DataType_Float);
	m_CarResultsData.Add(L"Top Speed Gear", L"TopSpeedGear", DataType_Float);
	m_CarResultsData.Add(L"Top Speed Engine Power", L"TopSpeedEnginePower", DataType_Float);
	m_CarResultsData.Add(L"Top Speed Engine RPM", L"TopSpeedEngineRPM", DataType_Float);

	m_CarResultsData.Add(L"60 Time", L"SixtyTime", DataType_Float);
	m_CarResultsData.Add(L"80 Time", L"EightyTime", DataType_Float);
	m_CarResultsData.Add(L"100 Time", L"HundredTime", DataType_Float);
	m_CarResultsData.Add(L"200 Time", L"TwoHundredTime", DataType_Float);
	m_CarResultsData.Add(L"120 Time", L"HundredAndTwentyTime", DataType_Float);
	m_CarResultsData.Add(L"80 To 120 Time", L"EightyToHundredAndTwentyTime", DataType_Float);
	m_CarResultsData.Add(L"Kilometer Time", L"KilometerTime", DataType_Float);
	m_CarResultsData.Add(L"Kilometer Speed (km/h)", L"KilometerSpeed", DataType_Float);
	m_CarResultsData.Add(L"1/4 Mile Time", L"QuarterMileTime", DataType_Float);
	m_CarResultsData.Add(L"1/4 Mile Speed (km/h)", L"QuarterMileSpeed", DataType_Float);



	m_EngineChoicesData.Add(	L"Engine Family Name",				L"FamilyName",						DataType_String);
	m_EngineChoicesData.Add(	L"Engine Family Year",				L"FamilyYear",						DataType_Float);

	m_EngineChoicesData.Add(	L"Family Quality",					L"FamilyQuality",					DataType_Float);
	m_EngineChoicesData.Add(	L"Family Techpool",					L"FamilyTechpool",					DataType_Float);
	m_EngineChoicesData.Add(	L"Family Top End Techpool",			L"FamilyTopEndTechpool",			DataType_Float);

	m_EngineChoicesData.Add(	L"Family Bore",						L"FamilyBore",						DataType_Float);
	m_EngineChoicesData.Add(	L"Family Stroke",					L"FamilyStroke",					DataType_Float);
	m_EngineChoicesData.Add(	L"Family Displacement",				L"FamilyDisplacement",				DataType_Float);

	m_EngineChoicesData.Add(	L"Block Type",						L"BlockType",						DataType_String);
	m_EngineChoicesData.Add(	L"Cylinder Count",					L"Cylinders",						DataType_Float);
	m_EngineChoicesData.Add(	L"Block Material",					L"BlockMaterial",					DataType_String);
	m_EngineChoicesData.Add(	L"Head Material",					L"HeadMaterial",					DataType_String);
	m_EngineChoicesData.Add(	L"Head Type",						L"HeadType",						DataType_String);

	m_EngineChoicesData.Add(	L"Intake Valves",					L"IntakeValves",					DataType_Float);
	m_EngineChoicesData.Add(	L"Exhaust Valves",					L"ExhaustValves",					DataType_Float);


	m_EngineChoicesData.Add(	L"Variant Name",					L"VariantName",						DataType_String);
	m_EngineChoicesData.Add(	L"Variant Year",					L"VariantYear",						DataType_Float);

	m_EngineChoicesData.Add(	L"Variant Top End Quality",			L"VariantTopEndQuality",			DataType_Float);
	m_EngineChoicesData.Add(	L"Variant Bottom End Quality",		L"VariantBottomEndQuality",			DataType_Float);
	m_EngineChoicesData.Add(	L"Variant Aspiration Quality",		L"VariantAspirationQuality",		DataType_Float);
	m_EngineChoicesData.Add(	L"Variant Fuel System Quality",		L"VariantFuelSystemQuality",		DataType_Float);
	m_EngineChoicesData.Add(	L"Variant Exhaust Quality",			L"VariantExhaustQuality",			DataType_Float);

	m_EngineChoicesData.Add(	L"Variant Top End Techpool",		L"VariantTopEndTechpool",			DataType_Float);
	m_EngineChoicesData.Add(	L"Variant Family Techpool",			L"VariantFamilyTechpool",			DataType_Float);
	m_EngineChoicesData.Add(	L"Variant Bottom End Techpool",		L"VariantBottomEndTechpool",		DataType_Float);
	m_EngineChoicesData.Add(	L"Variant Aspiration Techpool",		L"VariantAspirationTechpool",		DataType_Float);
	m_EngineChoicesData.Add(	L"Variant Fuel System Techpool",	L"VariantFuelSystemTechpool",		DataType_Float);
	m_EngineChoicesData.Add(	L"Variant Exhaust Techpool",		L"VariantExhaustTechpool",			DataType_Float);
									
	m_EngineChoicesData.Add(	L"Variant Bore",					L"VariantBore",						DataType_Float);
	m_EngineChoicesData.Add(	L"Variant Stroke",					L"VariantStroke",					DataType_Float);
	m_EngineChoicesData.Add(	L"Variant Displacement",			L"VariantDisplacement",				DataType_Float);

	m_EngineChoicesData.Add(	L"Balance Shaft",					L"BalanceShaft",					DataType_String);
	m_EngineChoicesData.Add(	L"Balancing Mass Setting",			L"BalancingMass",					DataType_Float);
	m_EngineChoicesData.Add(	L"Crankshaft",						L"Crankshaft",						DataType_String);
	m_EngineChoicesData.Add(	L"Conrods",							L"Conrods",							DataType_String);
	m_EngineChoicesData.Add(	L"Pistons",							L"Pistons",							DataType_String);
	
	m_EngineChoicesData.Add(	L"Compression",						L"Compression",						DataType_Float);
	m_EngineChoicesData.Add(	L"Cam Profile",						L"CamProfile",						DataType_Float);
	m_EngineChoicesData.Add(	L"Valve Spring Stiffness",			L"ValveSpringStiffness",			DataType_Float);
	m_EngineChoicesData.Add(	L"RPM Limit",						L"RPMLimit",						DataType_Float);

	m_EngineChoicesData.Add(	L"VVT",								L"VVT",								DataType_String);
	m_EngineChoicesData.Add(	L"VVL",								L"VVL",								DataType_String);
	m_EngineChoicesData.Add(	L"VVL Cam Profile",					L"VVLCamProfile",					DataType_Float);
	m_EngineChoicesData.Add(	L"VVL RPM",							L"VVLRPM",							DataType_Float);

	m_EngineChoicesData.Add(	L"Aspiration",						L"Aspiration",						DataType_String);
	m_EngineChoicesData.Add(	L"Intercooler Size",				L"IntercoolerSize",					DataType_Float);

	m_EngineChoicesData.Add(	L"Aspiration Item 1",				L"AspirationItem1",					DataType_String);
	m_EngineChoicesData.Add(	L"Aspiration Sub Item 1",			L"AspirationSubItem1",				DataType_String);
	m_EngineChoicesData.Add(	L"Aspiration Item 2",				L"AspirationItem2",					DataType_String);
	m_EngineChoicesData.Add(	L"Aspiration Sub Item 2",			L"AspirationSubItem2",				DataType_String);

	m_EngineChoicesData.Add(	L"Boost Control",					L"BoostControl",					DataType_String);

	m_EngineChoicesData.Add(	L"Compressor Size 1",				L"ChargerSize1",					DataType_Float);
	m_EngineChoicesData.Add(	L"Compressor Size 2",				L"ChargerSize2",					DataType_Float);
	m_EngineChoicesData.Add(	L"Turbine Size 1",					L"TurbineSize1",					DataType_Float);
	m_EngineChoicesData.Add(	L"Turbine Size 2",					L"TurbineSize2",					DataType_Float);
	m_EngineChoicesData.Add(	L"Aspiration Tune 1",				L"ChargerTune1",					DataType_Float);
	m_EngineChoicesData.Add(	L"Aspiration Tune 2",				L"ChargerTune2",					DataType_Float);

	m_EngineChoicesData.Add(	L"Fuel System Type",				L"FuelSystemType",					DataType_String);
	m_EngineChoicesData.Add(	L"Fuel System",						L"FuelSystem",						DataType_String);
	m_EngineChoicesData.Add(	L"Throttle Configuration",			L"Throttles",						DataType_String);
	m_EngineChoicesData.Add(	L"Carb Size",						L"CarbSize",						DataType_Float);
	m_EngineChoicesData.Add(	L"Intake",							L"Intake",							DataType_String);
	m_EngineChoicesData.Add(	L"Intake Size",						L"IntakeSize",						DataType_Float);

	m_EngineChoicesData.Add(	L"Leaded Fuel",						L"FuelLeaded",						DataType_String);
	m_EngineChoicesData.Add(	L"Fuel Type",						L"FuelType",						DataType_String);
	m_EngineChoicesData.Add(	L"Fuel Octane",						L"FuelOctane",						DataType_Float);
	m_EngineChoicesData.Add(	L"Octane Offset",					L"OctaneOffset",					DataType_Float);

	//m_EngineChoicesData.Add(	L"AFR",								L"AFR",								DataType_Float);
	//m_EngineChoicesData.Add(	L"Ignition Timing",					L"IgnitionTiming",					DataType_Float);
	m_EngineChoicesData.Add(	L"Fuel Map",						L"FuelMap",							DataType_Float);

	m_EngineChoicesData.Add(	L"Headers",							L"Headers",							DataType_String);
	m_EngineChoicesData.Add(	L"Header Size",						L"HeaderSize",						DataType_Float);
	m_EngineChoicesData.Add(	L"Exhaust Count",					L"ExhaustCount",					DataType_String);
	m_EngineChoicesData.Add(	L"Exhaust Diameter",				L"ExhaustDiameter",					DataType_Float);
	m_EngineChoicesData.Add(	L"Exhaust Bypass Valves",			L"BypassValves",					DataType_String);
	m_EngineChoicesData.Add(	L"Catalytic Converter",				L"Cat",								DataType_String);
	m_EngineChoicesData.Add(	L"Muffler 1",						L"Muffler1",						DataType_String);
	m_EngineChoicesData.Add(	L"Muffler 2",						L"Muffler2",						DataType_String);

	m_EngineResultsData.Add(	L"Peak Power",						L"EnginePower",						DataType_Float);
	m_EngineResultsData.Add(	L"Peak Power RPM",					L"EnginePowerRPM",					DataType_Float);
	m_EngineResultsData.Add(	L"Peak Torque",						L"EngineTorque",					DataType_Float);
	m_EngineResultsData.Add(	L"Peak Torque RPM",					L"EngineTorqueRPM",					DataType_Float);
	m_EngineResultsData.Add(	L"Peak Boost",						L"EnginePeakBoost",					DataType_Float);
	m_EngineResultsData.Add(	L"Peak Boost RPM",					L"EnginePeakBoostRPM",				DataType_Float);

	m_EngineResultsData.Add(	L"Idle RPM",						L"EngineIdleRPM",					DataType_Float);
	m_EngineResultsData.Add(	L"Max RPM",							L"EngineMaxRPM",					DataType_Float);

	//m_EngineResultsData.Add(	L"Min Economy RPM",					L"MinEconomyRPM",					DataType_Float);

	m_EngineResultsData.Add(	L"Engine Weight",					L"EngineWeight",					DataType_Float);
	m_EngineResultsData.Add(	L"Flywheel Weight",					L"EngineFlywheelWeight",			DataType_Float);
	m_EngineResultsData.Add(	L"Required Cooling",				L"EngineRequiredCooling",			DataType_Float);

	m_EngineResultsData.Add(	L"Engine Performance Index",		L"EnginePerformanceIndex",			DataType_Float);
	m_EngineResultsData.Add(	L"Engine Responsiveness",			L"EngineResponsiveness",			DataType_Float);
	m_EngineResultsData.Add(	L"Engine Smoothness",				L"EngineSmoothness",				DataType_Float);

	m_EngineResultsData.Add(	L"Engine Reliability",				L"EngineReliability",				DataType_Float);

	m_EngineResultsData.Add(	L"Engine Emissions",				L"EngineEmissions",					DataType_Float);
	m_EngineResultsData.Add(	L"Engine Efficiency",				L"EngineEfficiency",				DataType_Float);
	m_EngineResultsData.Add(	L"Engine Noise",					L"EngineNoise",						DataType_Float);
	m_EngineResultsData.Add(	L"Intake Noise",					L"IntakeNoise",						DataType_Float);
	m_EngineResultsData.Add(	L"Exhaust Noise",					L"ExhaustNoise",					DataType_Float);

	m_EngineResultsData.Add(	L"Engine RON Rating",				L"EngineRONResult",					DataType_Float);

	m_EngineResultsData.Add(	L"Engine Engineering Time",			L"EngineEngineeringTime",			DataType_Float);
	m_EngineResultsData.Add(	L"Engine Production Units",			L"EngineProductionUnits",			DataType_Float);

	m_EngineResultsData.Add(	L"Engine Engineering Costs",		L"EngineEngineeringCost",			DataType_Float);
	m_EngineResultsData.Add(	L"Engine Tooling Costs",			L"EngineToolingCosts",				DataType_Float);
	m_EngineResultsData.Add(	L"Engine Material Cost",			L"EngineMaterialCost",				DataType_Float);
	m_EngineResultsData.Add(	L"Engine Total Cost",				L"EngineTotalCost",					DataType_Float);

	m_EngineResultsData.Add(	L"Engine Service Cost",				L"EngineServiceCost",				DataType_Float);

	m_EngineResultsData.Add(	L"Crankshaft Max RPM",				L"CrankMaxRPM",						DataType_Float);
	m_EngineResultsData.Add(	L"Crankshaft Max Torque",			L"CrankMaxTorque",					DataType_Float);
	m_EngineResultsData.Add(	L"Conrods Max RPM",					L"ConrodMaxRPM",					DataType_Float);
	m_EngineResultsData.Add(	L"Conrods Max Torque",				L"ConrodMaxTorque",					DataType_Float);
	m_EngineResultsData.Add(	L"Pistons Max RPM",					L"PistonMaxRPM",					DataType_Float);
	m_EngineResultsData.Add(	L"Pistons Max Torque",				L"PistonMaxTorque",					DataType_Float);

	//Cost calculations
	std::wstring costPreset = m_CarData->GetStringData(2)->Value;
	uint8_t costPresetID = (uint8_t)_wtoi(costPreset.c_str());
	if (costPresetID > 13)
		costPresetID = 0;

	m_CarResultsData.Add(L"Costs Per Trim", L"TrimCostPreset" + std::to_wstring(costPresetID), DataType_Float);
	m_CarResultsData.Add(L"Costs Per Car", L"CarCostPreset" + std::to_wstring(costPresetID), DataType_Float);
	m_EngineResultsData.Add(L"Costs Per Engine", L"EngineCostPreset" + std::to_wstring(costPresetID), DataType_Float);
}

void AuExpManager::BuildTranslations()
{
	m_Translations[L"Chassis_Monocoque_Name"] = L"Monocoque";
	m_Translations[L"Chassis_Ladder_Name"] = L"Ladder";
	m_Translations[L"Chassis_SemiSpaceFrame_Name"] = L"Semi-Spaceframe";
	m_Translations[L"Chassis_SpaceFrame_Name"] = L"Space Frame";
	m_Translations[L"Chassis_LightTruckMonocoque_Name"] = L"Light Truck Monocoque";

	m_Translations[L"F"] = L"Front";
	m_Translations[L"R"] = L"Rear";
	m_Translations[L"M"] = L"Middle";

	m_Translations[L"ChassisMat_CRESteel_Name"] = L"Corrosion Resistant Steel";
	m_Translations[L"ChassisMat_AHSSteel_Name"] = L"AHS Steel";
	m_Translations[L"ChassisMat_AHSLight_Name"] = L"Light AHS Steel";
	m_Translations[L"ChassisMat_Steel_Name"] = L"Steel";
	m_Translations[L"ChassisMat_Alu_Name"] = L"Aluminum";
	m_Translations[L"ChassisMat_GalvanizedSteel_Name"] = L"Galvanized Steel";
	m_Translations[L"ChassisMat_CarFibre_Name"] = L"Carbon Fiber";
	m_Translations[L"ChassisMat_GluedAlu_Name"] = L"Glued Aluminum";

	m_Translations[L"Suspend_SolAxCoil_Name"] = L"Coil Solid Axle";
	m_Translations[L"Suspend_SolAxLeaf_Name"] = L"Leaf Solid Axle";
	m_Translations[L"Suspend_DblWishbone_Name"] = L"Double Wishbone";
	m_Translations[L"Suspend_Pushrod_Name"] = L"Pushrod";
	m_Translations[L"Suspend_Multilink_Name"] = L"Multilink";
	m_Translations[L"Suspend_SemiTrailArm_Name"] = L"Semi-Trailing Arm";
	m_Translations[L"Suspend_MacPher_Name"] = L"MacPherson";
	m_Translations[L"Suspend_Torsion_Name"] = L"Torsion Beam";

	m_Translations[L"PanelMat_Steel_Name"] = L"Steel";
	m_Translations[L"PanelMat_TreatedSteel_Name"] = L"Treated Steel";
	m_Translations[L"PanelMat_Alu_Name"] = L"Aluminum";
	m_Translations[L"PanelMat_PartialAlu_Name"] = L"Partial Aluminum";
	m_Translations[L"PanelMat_PartialCF_Name"] = L"Partial Carbon Fiber";
	m_Translations[L"PanelMat_FibreGlass_Name"] = L"Fiber Glass";
	m_Translations[L"PanelMat_CarbonFibre_Name"] = L"Carbon Fiber";

	m_Translations[L"TRAV"] = L"Transverse";
	m_Translations[L"LONG"] = L"Longitudinal";

	m_Translations[L"DriveType_TransFWD_Name"] = L"Transverse FWD";
	m_Translations[L"DriveType_LongFWD_Name"] = L"Longitudinal FWD";
	m_Translations[L"DriveType_Trans4WD_Name"] = L"Transverse AWD";
	m_Translations[L"DriveType_TransRWD_Name"] = L"Transverse RWD";
	m_Translations[L"DriveType_Long4WD_Name"] = L"Longitudinal AWD";
	m_Translations[L"DriveType_LongRWD_Name"] = L"Longitudinal RWD";
	m_Translations[L"DriveType_Long4X4_Name"] = L"Longitudinal 4x4";

	m_Translations[L"GearboxType_Sequential_Name"] = L"Sequential";
	m_Translations[L"GearboxType_AutoManual_Name"] = L"Automanual";
	m_Translations[L"GearboxType_SequentialDblClutch_Name"] = L"Double Clutch";
	m_Translations[L"GearboxType_Auto_Name"] = L"Automatic";
	m_Translations[L"GearboxType_AutoAdv_Name"] = L"Advanced Automatic";
	m_Translations[L"GearboxType_Manual_Name"] = L"Manual";

	m_Translations[L"Diff_Standard_Name"] = L"Open";
	m_Translations[L"Diff_Locker_Name"] = L"Locking";
	m_Translations[L"Diff_AutoLocker_Name"] = L"Autolocking";
	m_Translations[L"Diff_GearedLSD_Name"] = L"Geared LSD";
	m_Translations[L"Diff_ClutchedLSD_Name"] = L"Clutched LSD";
	m_Translations[L"Diff_ViscLSD_Name"] = L"Viscous LSD";
	m_Translations[L"Diff_ElecLSD_Name"] = L"Electric LSD";

	m_Translations[L"EBodyType::EBT_COUPE"] = L"Coupe";
	m_Translations[L"EBodyType::EBT_HATCHBACK"] = L"Hatchback";
	m_Translations[L"EBodyType::EBT_WAGON"] = L"Station Wagon";
	m_Translations[L"EBodyType::EBT_TRUCK"] = L"Pickup";
	m_Translations[L"EBodyType::EBT_VAN"] = L"Van";
	m_Translations[L"EBodyType::EBT_SUV"] = L"SUV";
	m_Translations[L"EBodyType::EBT_PEOPLEMOVER"] = L"People Mover";
	m_Translations[L"EBodyType::EBT_SEDAN"] = L"Sedan";
	m_Translations[L"EBodyType::EBT_CONVERTIBLE"] = L"Convertible";

	m_Translations[L"ConvertibleTypes_None_Name"] = L"None";
	m_Translations[L"ConvertibleTypes_RemovableSoft_Name"] = L"Detachable Soft Top";
	m_Translations[L"ConvertibleTypes_RemovableHard_Name"] = L"Detachable Hard Top";
	m_Translations[L"ConvertibleTypes_ManualSoft_Name"] = L"Manual Soft Top";
	m_Translations[L"ConvertibleTypes_ManualHard_Name"] = L"Manual Hard Top";
	m_Translations[L"ConvertibleTypes_AutomaticSoft_Name"] = L"Automatic Soft Top";
	m_Translations[L"ConvertibleTypes_AutomaticHard_Name"] = L"Automatic Hard Top";
	m_Translations[L"ConvertibleTypes_AutomaticAdv_Name"] = L"Hidden Automatic Soft Top";

	m_Translations[L"ActiveAero_None_Name"] = L"None";
	m_Translations[L"ActiveAero_Wing_Name"] = L"Active Wing";
	m_Translations[L"ActiveAero_Cooling_Name"] = L"Active Cooling";

	m_Translations[L"Undertray_None_Name"] = L"None";
	m_Translations[L"Undertray_Offroad_Name"] = L"Offroad";
	m_Translations[L"Undertray_SemiClad_Name"] = L"Semi-Clad";
	m_Translations[L"Undertray_FullyClad_Name"] = L"Fully Clad";
	m_Translations[L"Undertray_Sport_Name"] = L"Sport";
	m_Translations[L"Undertray_FlowOptimized_Name"] = L"Flow Optimized";
	m_Translations[L"Undertray_Race_Name"] = L"Race";

	m_Translations[L"Interior_Basic_Name"] = L"Basic";
	m_Translations[L"Interior_Standard_Name"] = L"Standard";
	m_Translations[L"Interior_Premium_Name"] = L"Premium";
	m_Translations[L"Interior_SuperLight_Name"] = L"Sport";
	m_Translations[L"Interior_Luxury_Name"] = L"Luxury";
	m_Translations[L"Interior_HandMade_Name"] = L"Hand Made";


	m_Translations[L"Entertain_Basic_HUD_Name"] = L"Basic HUD";
	m_Translations[L"Entertain_Standard_HUD_Name"] = L"Standard HUD";
	m_Translations[L"Entertain_Premium_HUD_Name"] = L"Premium HUD";
	m_Translations[L"Entertain_Luxury_HUD_Name"] = L"Luxury HUD";

	m_Translations[L"Entertain_Basic_Infotainment_Name"] = L"Basic Infotainment";
	m_Translations[L"Entertain_Standard_Infotainment_Name"] = L"Standard Infotainment";
	m_Translations[L"Entertain_Premium_Infotainment_Name"] = L"Premium Infotainment";
	m_Translations[L"Entertain_Luxury_Infotainment_Name"] = L"Luxury Infotainment";

	m_Translations[L"Entertain_Basic_SatNav_Name"] = L"Basic SatNav";
	m_Translations[L"Entertain_Standard_SatNav_Name"] = L"Standard SatNav";
	m_Translations[L"Entertain_Premium_SatNav_Name"] = L"Premium SatNav";
	m_Translations[L"Entertain_Luxury_SatNav_Name"] = L"Luxury SatNav";

	m_Translations[L"Entertain_Basic_CD_Name"] = L"Basic CD";
	m_Translations[L"Entertain_Standard_CD_Name"] = L"Standard CD";
	m_Translations[L"Entertain_Premium_CD_Name"] = L"Premium CD";
	m_Translations[L"Entertain_Luxury_CD_Name"] = L"Luxury CD";

	m_Translations[L"Entertain_Basic_Cassette_Name"] = L"Basic Cassette";
	m_Translations[L"Entertain_Standard_Cassette_Name"] = L"Standard Cassette";
	m_Translations[L"Entertain_Premium_Cassette_Name"] = L"Premium Cassette";
	m_Translations[L"Entertain_Luxury_Cassette_Name"] = L"Luxury Cassette";

	m_Translations[L"Entertain_Basic_8Track_Name"] = L"Basic 8-Track";
	m_Translations[L"Entertain_Standard_8Track_Name"] = L"Standard 8-Track";
	m_Translations[L"Entertain_Premium_8Track_Name"] = L"Premium 8-Track";
	m_Translations[L"Entertain_Luxury_8Track_Name"] = L"Luxury 8-Track";

	m_Translations[L"Entertain_Basic_AM_Name"] = L"Basic AM Radio";
	m_Translations[L"Entertain_Standard_AM_Name"] = L"Standard AM Radio";
	m_Translations[L"Entertain_Premium_AM_Name"] = L"Premium AM Radio";
	m_Translations[L"Entertain_Luxury_AM_Name"] = L"Luxury AM Radio";

	m_Translations[L"Entertain_None_Name"] = L"None";


	m_Translations[L"DriveAssist_PowerSteer_None_Name"] = L"Recirculating Ball";
	m_Translations[L"DriveAssist_PowerSteer_RackAndPinion_Name"] = L"Rack And Pinion";
	m_Translations[L"DriveAssist_PowerSteer_HydraulicBall_Name"] = L"Hydraulic Ball";
	m_Translations[L"DriveAssist_PowerSteer_Name"] = L"Hydraulic R&P";
	m_Translations[L"DriveAssist_PowerSteer_Variable_Name"] = L"Variable Hydraulic";
	m_Translations[L"DriveAssist_PowerSteer_Electric_Name"] = L"Electric";
	m_Translations[L"DriveAssist_PowerSteer_Electric_Variable_Name"] = L"Variable Electric";

	m_Translations[L"DriveAssist_None_Name"] = L"None";
	m_Translations[L"DriveAssist_TractionPackage1_Name"] = L"ABS";
	m_Translations[L"DriveAssist_TractionPackage2_Name"] = L"ABS TC";
	m_Translations[L"DriveAssist_TractionPackage3_Name"] = L"ABS TC ESC";
	m_Translations[L"DriveAssist_TractionPackage4_Name"] = L"ABS TC ESC LC";


	m_Translations[L"Safety_None_Name"] = L"None";

	m_Translations[   L"Safety_Basic_20s_Name"] =    L"Basic 20s";
	m_Translations[L"Safety_Standard_20s_Name"] = L"Standard 20s";
	m_Translations[L"Safety_Advanced_20s_Name"] = L"Advanced 20s";

	m_Translations[   L"Safety_Basic_10s_Name"] =    L"Basic 10s";
	m_Translations[L"Safety_Standard_10s_Name"] = L"Standard 10s";
	m_Translations[L"Safety_Advanced_10s_Name"] = L"Advanced 10s";

	m_Translations[   L"Safety_Basic_00s_Name"] =    L"Basic 00s";
	m_Translations[L"Safety_Standard_00s_Name"] = L"Standard 00s";
	m_Translations[L"Safety_Advanced_00s_Name"] = L"Advanced 00s";

	m_Translations[   L"Safety_Basic_90s_Name"] =    L"Basic 90s";
	m_Translations[L"Safety_Standard_90s_Name"] = L"Standard 90s";
	m_Translations[L"Safety_Advanced_90s_Name"] = L"Advanced 90s";

	m_Translations[   L"Safety_Basic_80s_Name"] =    L"Basic 80s";
	m_Translations[L"Safety_Standard_80s_Name"] = L"Standard 80s";
	m_Translations[L"Safety_Advanced_80s_Name"] = L"Advanced 80s";

	m_Translations[   L"Safety_Basic_70s_Name"] =    L"Basic 70s";
	m_Translations[L"Safety_Standard_70s_Name"] = L"Standard 70s";
	m_Translations[L"Safety_Advanced_70s_Name"] = L"Advanced 70s";

	m_Translations[   L"Safety_Basic_60s_Name"] =    L"Basic 60s";
	m_Translations[L"Safety_Standard_60s_Name"] = L"Standard 60s";
	m_Translations[L"Safety_Advanced_60s_Name"] = L"Advanced 60s";

	m_Translations[   L"Safety_Basic_50s_Name"] =    L"Basic 50s";
	m_Translations[L"Safety_Standard_50s_Name"] = L"Standard 50s";
	m_Translations[L"Safety_Advanced_50s_Name"] = L"Advanced 50s";

	m_Translations[   L"Safety_Basic_40s_Name"] =    L"Basic 40s";
	m_Translations[L"Safety_Standard_40s_Name"] = L"Standard 40s";
	m_Translations[L"Safety_Advanced_40s_Name"] = L"Advanced 40s";


	m_Translations[L"Springs_Passive_Name"] = L"Passive";
	m_Translations[L"Springs_Progressive_Name"] = L"Progressive";
	m_Translations[L"Springs_Air_Name"] = L"Air";
	m_Translations[L"Springs_Hydro_Name"] = L"Hydropneumatic";
	m_Translations[L"Springs_ActiveSport_Name"] = L"Active Sport";
	m_Translations[L"Springs_ActiveComfort_Name"] = L"Active Comfort";
	m_Translations[L"Springs_Active_Name"] = L"Active Comfort";

	m_Translations[L"Dampers_Passive_Name"] = L"Twin-Tube";
	m_Translations[L"Dampers_Advanced_Name"] = L"Gas Mono-Tube";
	m_Translations[L"Dampers_Adaptive_Name"] = L"Adaptive";
	m_Translations[L"Dampers_SemiActive_Name"] = L"Semi-Active";

	m_Translations[L"SwayBars_Passive_Name"] = L"Passive";
	m_Translations[L"SwayBars_Offroad_Name"] = L"Offroad";
	m_Translations[L"SwayBars_SemiActive_Name"] = L"Semi-Active";
	m_Translations[L"SwayBars_Active_Name"] = L"Active";

	m_Translations[L"RimMaterial_Steel_Name"] = L"Steel";
	m_Translations[L"RimMaterial_Alloy_Name"] = L"Alloy";
	m_Translations[L"RimMaterial_Mag_Name"] = L"Magnesium";
	m_Translations[L"RimMaterial_CF_Name"] = L"Carbon Fiber";

	m_Translations[L"TyreType_Radial_Name"] = L"Radial";
	m_Translations[L"TyreType_CrossPly_Name"] = L"Cross-Ply";

	m_Translations[L"TyreType_SemiOffRoad_Name"] = L"Offroad";
	m_Translations[L"TyreType_AllTerrain_Name"] = L"All-Terrain";
	m_Translations[L"TyreType_Utility_Name"] = L"Utility";
	m_Translations[L"TyreType_LongHardRoad_Name"] = L"Hard";
	m_Translations[L"TyreType_MedComp_Name"] = L"Medium";
	m_Translations[L"TyreType_SportComp_Name"] = L"Sport";
	m_Translations[L"TyreType_SemiSlick_Name"] = L"Semi-Slick";

	m_Translations[L"Brake_Drum_Name"] = L"Drum";
	m_Translations[L"Brake_TwinLeadingDrum_Name"] = L"Twin-Leading Drum";
	m_Translations[L"BrakeDisc_Solid_Name"] = L"Solid Disc";
	m_Translations[L"BrakeDisc_Vented_Name"] = L"Vented Disc";
	m_Translations[L"BrakeDisc_CarCeramic_Name"] = L"Carbon-Ceramic";
	

	m_Translations[L"EngBlock_Inl_Name"] = L"Inline";
	m_Translations[L"EngBlock_V90_Name"] = L"V90";
	m_Translations[L"EngBlock_V60_Name"] = L"V60";
	m_Translations[L"EngBlock_Boxer_Name"] = L"Boxer";

	m_Translations[L"EngBlockMat_Iron_Name"] = L"Iron";
	m_Translations[L"EngBlockMat_Alu_Name"] = L"Aluminum";
	m_Translations[L"EngBlockMat_AluSil_Name"] = L"AlSi";
	m_Translations[L"EngBlockMat_Mg_Name"] = L"Magnesium";

	m_Translations[L"HeadMat_Iron_Name"] = L"Iron";
	m_Translations[L"HeadMat_Alu_Name"] = L"Aluminum";
	m_Translations[L"EngHeadMat_AluSil_Name"] = L"AlSi";

	m_Translations[L"Head_PushRod_Name"] = L"Pushrod OHV";
	m_Translations[L"Head_DirectOHC_Name"] = L"DaSOHC";
	m_Translations[L"Head_OHC_Name"] = L"SOHC";
	m_Translations[L"Head_DuelOHC_Name"] = L"DOHC";

	m_Translations[L"BalanceShaft_None_Name"] = L"None";
	m_Translations[L"BalanceShaft_Harmonic_Name"] = L"Harmonic Damper";
	m_Translations[L"BalanceShaft_BalanceShafts_Name"] = L"Balance Shaft";

	m_Translations[L"CrankMat_Iron_Name"] = L"Cast";
	m_Translations[L"CrankMat_Forged_Name"] = L"Forged";
	m_Translations[L"CrankMat_Billet_Name"] = L"Billet";

	m_Translations[L"RodMat_Cast_Name"] = L"Cast";
	m_Translations[L"RodMat_HeavyCast_Name"] = L"Heavy Duty Cast";
	m_Translations[L"RodMat_HBeam_Name"] = L"Heavy Duty Forged";
	m_Translations[L"RodMat_IBSteel_Name"] = L"Lightweight Forged";
	m_Translations[L"RodMat_IBTit_Name"] = L"Titanium";

	m_Translations[L"Piston_Cast_Name"] = L"Cast";
	m_Translations[L"Piston_HeavyCast_Name"] = L"Heavy Duty Cast";
	m_Translations[L"Piston_HypCast_Name"] = L"Hypereutectic Cast";
	m_Translations[L"Piston_LowFCast_Name"] = L"Low-Friction Cast";
	m_Translations[L"Piston_Forged_Name"] = L"Forged";
	m_Translations[L"Piston_LightForged_Name"] = L"Lightweight Forged";

	m_Translations[L"VVT_None_Name"] = L"None";
	m_Translations[L"VVT_VVTSOHC_Name"] = L"SOHC VVT";
	m_Translations[L"VVT_VVTDOHC_Name"] = L"DOHC VVT Both";
	m_Translations[L"VVT_VVTIntakeCam_Name"] = L"DOHC VVT Intake";
	m_Translations[L"VVT_None_Name"] = L"None";
	m_Translations[L"VVT_VVTSOHC_Name"] = L"SOHC VVT";
	m_Translations[L"VVT_VVTDOHC_Name"] = L"DOHC VVT Both";
	m_Translations[L"VVT_VVTIntakeCam_Name"] = L"DOHC VVT Intake";

	m_Translations[L"VarValves_None_Name"] = L"None";
	m_Translations[L"VarValves_VVTSOHC_Name"] = L"SOHC VVT";
	m_Translations[L"VarValves_VVTDOHC_Name"] = L"DOHC VVT Both";
	m_Translations[L"VarValves_VVTIntakeCam_Name"] = L"DOHC VVT Intake";
	m_Translations[L"VarValves_None_Name"] = L"None";
	m_Translations[L"VarValves_VVTSOHC_Name"] = L"SOHC VVT";
	m_Translations[L"VarValves_VVTDOHC_Name"] = L"DOHC VVT Both";
	m_Translations[L"VarValves_VVTIntakeCam_Name"] = L"DOHC VVT Intake";

	m_Translations[L"VarValves_None_Name"] = L"None";
	m_Translations[L"VarValves_VVL_Name"] = L"VVL";

	m_Translations[L"Aspiration_Natural_Name"] = L"Naturally Aspirated";
	m_Translations[L"Aspiration_Turbo_Name"] = L"Turbocharged";

	m_Translations[L"Aspiration_Natural_Setup_Name"] = L"Naturally Aspirated";
	m_Translations[L"Turbo_Single_Name"] = L"Single Turbo";
	m_Translations[L"Turbo_Twin_Name"] = L"Twin Turbo";
	m_Translations[L"Turbo_Quad_Name"] = L"Quad Turbo";

	m_Translations[L"Aspiration_Natural_Boost_Control_Name"] = L"None";
	m_Translations[L"TurboBoostControl_Wastegate_Name"] = L"Wastegate";
	m_Translations[L"TurboBoostControl_BoostControl_Name"] = L"Boost Control";
	m_Translations[L"TurboBoostControl_SmartBoost_Name"] = L"Smart Boost";

	m_Translations[L"NoOption_Name"] = L"None";
	m_Translations[L"Turbo_Standard_Geometry_Name"] = L"Standard Turbo";
	m_Translations[L"Turbo_Twin_Scroll_Geometry_Name"] = L"Twin Scroll Turbo";
	m_Translations[L"Turbo_Variable_Geometry_Name"] = L"Variable Geometry Turbo";

	m_Translations[L"TurboBearing_Journal_Name"] = L"Journal Bearing";
	m_Translations[L"TurboBearing_BallBearing_Name"] = L"Ball Bearing";

	m_Translations[L"FuelSys_Carb_Name"] = L"Carburetor";
	m_Translations[L"FuelSys_Inj_Name"] = L"Injection";

	m_Translations[L"FuelSys_Carb_1Barrel_Name"] = L"1-Barrel Carburetor";
	m_Translations[L"FuelSys_Carb_1BarrelEco_Name"] = L"1-Barrel Eco Carburetor";
	m_Translations[L"FuelSys_Carb_2Barrel_Name"] = L"2-Barrel Carburetor";
	m_Translations[L"FuelSys_Carb_4Barrel_Name"] = L"4-Barrel Carburetor";
	m_Translations[L"FuelSys_Carb_DCOE_Name"] = L"DCOE";
	m_Translations[L"FuelSys_Inj_Mech_Name"] = L"Mechanical Injection";
	m_Translations[L"FuelSys_Inj_SingEFI_Name"] = L"Single-Point EFI";
	m_Translations[L"FuelSys_Inj_MultiEFI_Name"] = L"Multi-Point EFI";
	m_Translations[L"FuelSys_Inj_Direct_Name"] = L"Direct Injection EFI";

	m_Translations[L"FuelSys_CarbIntake_Single_Name"] = L"Single Carburetor";
	m_Translations[L"FuelSys_CarbIntake_Twin_Name"] = L"Twin Carburetor";
	m_Translations[L"FuelSys_CarbIntake_Quad_Name"] = L"Quad Carburetor";
	m_Translations[L"FuelSys_InjIntake_Single_Name"] = L"Single Throttle";
	m_Translations[L"FuelSys_InjIntake_Twin_Name"] = L"Twin Throttle";
	m_Translations[L"FuelSys_InjIntake_PerCyl_Name"] = L"Individual Throttle Bodies";

	m_Translations[L"FuelSys_CarbFilter_Compact_Name"] = L"Compact";
	m_Translations[L"FuelSys_InjFilterr_Compact_Name"] = L"Compact";
	m_Translations[L"FuelSys_CarbFilter_StandardLow_Name"] = L"Standard Low";
	m_Translations[L"FuelSys_InjFilter_StandardLow_Name"] = L"Standard Low";
	m_Translations[L"FuelSys_CarbFilter_StandardMid_Name"] = L"Standard Mid";
	m_Translations[L"FuelSys_InjFilter_StandardMid_Name"] = L"Standard Mid";
	m_Translations[L"FuelSys_CarbFilter_PerfMidRange_Name"] = L"Performance Mid";
	m_Translations[L"FuelSys_InjFilter_PerfMidRange_Name"] = L"Performance Mid";
	m_Translations[L"FuelSys_CarbFilter_PerfHighRange_Name"] = L"Performance High";
	m_Translations[L"FuelSys_InjFilter_PerfHighRange_Name"] = L"Performance High";
	m_Translations[L"FuelSys_InjFilter_Variable_Name"] = L"Variable";
	m_Translations[L"FuelSys_CarbFilter_Race_Name"] = L"Race";
	m_Translations[L"FuelSys_InjFilter_Race_Name"] = L"Race";

	m_Translations[L"Header_CastLog_Name"] = L"Compact Cast";
	//m_Translations[L"Header_ShortCast_Name"] = L"Short Cast";
	m_Translations[L"Header_CastLow_Name"] = L"Cast Low";
	m_Translations[L"Header_CastMid_Name"] = L"Cast Mid";
	m_Translations[L"Header_Tube_Name"] = L"Tubular Mid";
	m_Translations[L"Header_LongTube_Name"] = L"Tubular Long";
	m_Translations[L"Header_RaceTube_Name"] = L"Tubular Race";
	m_Translations[L"Header_TurboShortLog_Name"] = L"Turbo Cast Log";
	m_Translations[L"Header_TurboShortCast_Name"] = L"Turbo Short Cast";
	m_Translations[L"Header_TurboRaceTubular_Name"] = L"Turbo Race Tubular";

	m_Translations[L"Exhausts_1_Name"] = L"Single";
	m_Translations[L"Exhausts_2_Name"] = L"Dual";

	m_Translations[L"ExhaustBypassValvesNone_Name"] = L"None";
	m_Translations[L"ExhaustBypassValves_Name"] = L"Bypass Valves";

	m_Translations[L"CatConvert_None_Name"] = L"None";
	m_Translations[L"CatConvert_2Way_Name"] = L"2-Way";
	m_Translations[L"CatConvert_3Way_Name"] = L"3-Way";
	m_Translations[L"CatConvert_High3Way_Name"] = L"High Flow 3-Way";

	m_Translations[L"Muffler_None_Name"] = L"None";
	m_Translations[L"Muffler_Confused_Name"] = L"Baffled";
	m_Translations[L"Muffler_Reverse_Name"] = L"Reverse Flow";
	m_Translations[L"Muffler_Straight_Name"] = L"Straight Through";

	m_Translations[L"CompressedGas"] = L"Compressed Gas";
	m_Translations[L"Nitro"] = L"Nitromethane";
}

void AuExpManager::GetTranslation(std::wstring* ret)
{
	if (m_Translations.find(*ret) == m_Translations.end())
		return;

	*ret = m_Translations[*ret];
}

