--D:\workspace\AutomationCarExport\csvExporter\csvExporter\Resources\ExportExample.lua
---------------------------------------------------
-- Copyright (c) 2021 -- Camshaft Software PTY LTD
---------------------------------------------------

function DoExport(CarCalculator, CarFile)
	UAPI.Log("DoExport: ")

	local value = {}

	local CarData = CExporter.ExportCarData(CarCalculator)
	local CarFiles = CExporter.ExportCarFiles(CarCalculator)
	CarFiles[string.format("%s - %s.car", CarCalculator.CarInfo.PlatformInfo.Name, CarCalculator.CarInfo.TrimInfo.Name)] = CarFile

	local Data = {}

	for k,v in pairs(CarData) do
		Data[k] = v
	end

	local Files = {}

	for k,v in pairs(CarFiles) do
		Files[k] = v
	end

	return Files, Data
end



if CExporter == nil then
	CExporter = {}
	CExporter.__index = CExporter
end



function CExporter.ExportCarData(CarCalculator)
	local CarInfo = CarCalculator.CarInfo
	local EngineInfo = CarInfo.TrimInfo.EngineInfo
	local GearboxInfo = CarInfo.TrimInfo.Gearbox
	local Results = CarInfo.TrimInfo.Results
	local SuspensionDetails = CarInfo.TrimInfo.SuspensionDetails

	local carParameters = CarCalculator:GetCarParameters()

	local Data = {}

	Data.ChassisType = CarInfo.PlatformInfo.Chassis.Name
	Data.ChassisMat = CarInfo.PlatformInfo.ChassisMaterial.Name
	Data.PanelMat = CarInfo.PlatformInfo.PanelMaterial.Name
	Data.SuspensionFront = CarCalculator.CarInfo.PlatformInfo.FrontSuspension.Name
	Data.SuspensionRear = CarCalculator.CarInfo.PlatformInfo.RearSuspension.Name

	Data.TrimYear = CarInfo.TrimInfo.Time.Date.Year
	Data.ModelYear = CarInfo.PlatformInfo.Time.Date.Year
	Data.ModelName = CarInfo.PlatformInfo.Name
	Data.TrimName = CarInfo.TrimInfo.Name
	
	Data.ModelTechpoolBody = CarInfo.PlatformInfo.TechPool.Body
	Data.ModelTechpoolChassis = CarInfo.PlatformInfo.TechPool.Chassis
	Data.ModelQualityBody = CarInfo.PlatformInfo.QualitySettings.Body
	Data.ModelQualityChassis = CarInfo.PlatformInfo.QualitySettings.Chassis
	
	Data.TrimTechpoolChassis = CarInfo.TrimInfo.TechPool.Chassis
	Data.TrimTechpoolAerodynamics = CarInfo.TrimInfo.TechPool.Aerodynamics
	Data.TrimTechpoolBody = CarInfo.TrimInfo.TechPool.Body
	Data.TrimTechpoolInterior = CarInfo.TrimInfo.TechPool.Interior
	Data.TrimTechpoolFixtures = CarInfo.TrimInfo.TechPool.Fixtures
	Data.TrimTechpoolTyres = CarInfo.TrimInfo.TechPool.Tyres
	Data.TrimTechpoolSuspension = CarInfo.TrimInfo.TechPool.Suspension
	Data.TrimTechpoolSafety = CarInfo.TrimInfo.TechPool.Safety
	Data.TrimTechpoolBrakes = CarInfo.TrimInfo.TechPool.Brakes
	Data.TrimTechpoolAssists = CarInfo.TrimInfo.TechPool.DriverAssists
	Data.TrimTechpoolDrivetrain = CarInfo.TrimInfo.TechPool.DriveTrain
	
	Data.TrimQualityChassis = CarInfo.TrimInfo.QualitySettings.Chassis
	Data.TrimQualityAerodynamics = CarInfo.TrimInfo.QualitySettings.Aerodynamics
	Data.TrimQualityBody = CarInfo.TrimInfo.QualitySettings.Body
	Data.TrimQualityInterior = CarInfo.TrimInfo.QualitySettings.Interior
	Data.TrimQualityFixtures = CarInfo.TrimInfo.QualitySettings.Fixtures
	Data.TrimQualityTyres = CarInfo.TrimInfo.QualitySettings.Tyres
	Data.TrimQualitySuspension = CarInfo.TrimInfo.QualitySettings.Suspension
	Data.TrimQualitySafety = CarInfo.TrimInfo.QualitySettings.Safety
	Data.TrimQualityBrakes = CarInfo.TrimInfo.QualitySettings.Brakes
	Data.TrimQualityAssists = CarInfo.TrimInfo.QualitySettings.DriverAssists
	Data.TrimQualityDrivetrain = CarInfo.TrimInfo.QualitySettings.DriveTrain
	
	Data.BodyWheelbase = CarInfo.TrimInfo.Body.WheelBase
	Data.BodyType = CarInfo.TrimInfo.Body.BodyType
	Data.BodyDoors = CarInfo.TrimInfo.Body.Doors
	--Data.BodyHardtop = CarInfo.TrimInfo.Body.ConvertableHard
	--Data.BodySofttop = CarInfo.TrimInfo.Body.ConvertableSoft
	Data.BodyName = CarInfo.TrimInfo.Body.Name
	--Data.BodyFrontalArea = CarInfo.PlatformInfo.Body.FrontalArea
	
	Data.ConvertibleType = CarInfo.TrimInfo.ConvertibleType.Name
	Data.FrontSeatsFull = CarInfo.TrimInfo.FrontRowSeats.FullSeatCount
	Data.FrontSeatsTemp = CarInfo.TrimInfo.FrontRowSeats.TempSeatCount
	Data.MidSeatsFull = CarInfo.TrimInfo.SecondRowSeats.FullSeatCount
	Data.MidSeatsTemp = CarInfo.TrimInfo.SecondRowSeats.TempSeatCount
	Data.RearSeatsFull = CarInfo.TrimInfo.ThirdRowSeats.FullSeatCount
	Data.RearSeatsTemp = CarInfo.TrimInfo.ThirdRowSeats.TempSeatCount
	
	Data.EngineOrientation = CarInfo.PlatformInfo.EnginePlacement.Orientation
	
	Data.PowerDistribution = GearboxInfo.PowerDistribution
	Data.GearCount = GearboxInfo.Ratios.RatioCount
	Data.GearSpacing = GearboxInfo.RatioSpacing
	Data.FinalDrive = GearboxInfo.DiffRatio

	Data.DriveType = GearboxInfo.DriveType.Name
	Data.GearboxType = GearboxInfo.Type.Name
	Data.DiffType = GearboxInfo.Differential.Name
	Data.SpeedLimit = GearboxInfo.SpeedLimiter
	
	Data.ActiveCooling = CarInfo.TrimInfo.ActiveCooling.Name
	Data.ActiveWing = CarInfo.TrimInfo.ActiveWing.Name
	Data.Cooling = CarInfo.TrimInfo.CoolingAirflowFraction
	Data.WingAngleRear = CarInfo.TrimInfo.InclinationRear
	Data.WingAngleFront = CarInfo.TrimInfo.InclinationFront
	Data.UnderTray = CarInfo.TrimInfo.Undertray.Name
	
	Data.Entertainment = CarInfo.TrimInfo.Entertainment.Name
	Data.Interior = CarInfo.TrimInfo.Interior.Name
	Data.PowerSteering = CarInfo.TrimInfo.PowerSteering.Name
	
	Data.AssistPackage = CarInfo.TrimInfo.Assist.Name
	
	Data.Safety = CarInfo.TrimInfo.Safety.Name
	
	Data.SwayBars = CarInfo.TrimInfo.SwayBars.Name
	Data.Dampers = CarInfo.TrimInfo.Dampers.Name
	Data.Springs = CarInfo.TrimInfo.Springs.Name
	
	Data.RideHeight = CarInfo.TrimInfo.SuspensionDetails.RideHeight
	Data.RearCamber = CarInfo.TrimInfo.SuspensionDetails.Rear.Camber
	Data.FrontCamber = CarInfo.TrimInfo.SuspensionDetails.Front.Camber
	Data.RearARBStiffness = CarInfo.TrimInfo.SuspensionDetails.Rear.ARBStiffness
	Data.FrontARBStiffness = CarInfo.TrimInfo.SuspensionDetails.Front.ARBStiffness
	Data.RearDamperStiffness = CarInfo.TrimInfo.SuspensionDetails.Rear.DamperStiffness
	Data.FrontDamperStiffness = CarInfo.TrimInfo.SuspensionDetails.Front.DamperStiffness
	Data.RearSpringStiffness = CarInfo.TrimInfo.SuspensionDetails.Rear.SpringStiffness
	Data.FrontSpringStiffness = CarInfo.TrimInfo.SuspensionDetails.Front.SpringStiffness
	
	Data.FrontRimSize = CarInfo.TrimInfo.TyreDetails.Front.Rim
	Data.RearRimSize = CarInfo.TrimInfo.TyreDetails.Rear.Rim
	Data.FrontWheelOffset = CarInfo.TrimInfo.TyreDetails.Front.Offset
	Data.RearWheelOffset = CarInfo.TrimInfo.TyreDetails.Rear.Offset
	Data.FrontTyreProfile = CarInfo.TrimInfo.TyreDetails.Front.Profile
	Data.RearTyreProfile = CarInfo.TrimInfo.TyreDetails.Rear.Profile
	Data.FrontTyreWidth = CarInfo.TrimInfo.TyreDetails.Front.Width
	Data.RearTyreWidth = CarInfo.TrimInfo.TyreDetails.Rear.Width
	Data.FrontOverallDiameter = CarInfo.TrimInfo.TyreDetails.Front.OverallDiameter
	Data.RearOverallDiameter = CarInfo.TrimInfo.TyreDetails.Rear.OverallDiameter
	
	Data.RimMaterial = CarInfo.TrimInfo.RimMaterial.Name
	Data.TyreType = CarInfo.TrimInfo.TyreType.Name
	Data.TyreCompound = CarInfo.TrimInfo.Tyres.Name
	
	Data.FrontBrakeType = CarInfo.TrimInfo.Brakes.Front.Discs.Name
	Data.RearBrakeType = CarInfo.TrimInfo.Brakes.Rear.Discs.Name
	Data.FrontCaliperPistons = CarInfo.TrimInfo.Brakes.Front.Calipers.PistonCount
	Data.RearCaliperPistons = CarInfo.TrimInfo.Brakes.Rear.Calipers.PistonCount
	Data.FrontBrakeDiameter = CarInfo.TrimInfo.Brakes.Front.Diameter
	Data.RearBrakeDiameter = CarInfo.TrimInfo.Brakes.Rear.Diameter
	Data.FrontBrakePadType = CarInfo.TrimInfo.Brakes.Front.PadType
	Data.RearBrakePadType = CarInfo.TrimInfo.Brakes.Rear.PadType
	
	Data.FrontBrakeForce = CarInfo.TrimInfo.Brakes.Front.BrakeForce
	Data.RearBrakeForce = CarInfo.TrimInfo.Brakes.Rear.BrakeForce
	
	Data.WeightOptimizationTune = CarInfo.TrimInfo.WeightOptimizationFraction
	Data.WeightDistributionTune = CarInfo.TrimInfo.WeightDistributionFraction
	
	Data.CarWeight = Results.Weight
	Data.TowWeight = Results.TowWeight
	
	Data.SportinessRating 	= Results.SportinessValue
	Data.DrivabilityRating 	= Results.DrivabilityValue
	Data.UtilityRating 		= Results.Utility
	Data.EnvResistance		= Results.EnvironmentalResistance
	Data.Emissions 			= Results.Emissions
	Data.BodyStiffness 		= Results.Stiffness
	Data.OffroadRating 		= Results.Offroad
	Data.PrestigeRating 	= Results.PrestigeValue
	Data.ReliabilityRating 	= Results.Reliability
	Data.Economy 			= Results.Economy
	Data.PracticalityRating = Results.Practicality
	Data.SafetyRating 		= Results.SafetyValue
	Data.ComfortRating 		= Results.ComfortValue
	
	Data.PassengerVolume = Results.PassengerVolume
	Data.CargoVolume = Results.CargoVolume
	
	Data.TrimEngineeringTime = Results.EngineeringTime
	Data.TrimManHours = Results.ManHours
	
	Data.TrimPrice = Results.Price
	
	Data.ToolingCosts = Results.ToolingCosts
	Data.ServiceCosts = Results.ServiceCosts
	Data.TotalTrimCosts = Results.TotalCosts
	Data.TrimCost = Results.Cost
	Data.MatCost = Results.MaterialCost
	Data.TrimEngCosts = Results.EngineeringCosts
	
	Data.BrakingDistance = Results.BrakingDistance
	Data.MaxBodyRoll = Results.MaxRollAngle
	Data.Cornering = Results.Cornering
	
	Data.RearDownforce = Results.RearDownForce
	Data.FrontDownforce = Results.FrontDownForce
	
	Data.TyreSpeedIndex = Results.TyreSpeedIndex
	Data.TyreSpeedRating = Results.TyreSpeedRating
	
	Data.TopSpeed = Results.TopSpeed
	Data.TopSpeedGear = Results.TopSpeedGear
	Data.TopSpeedEnginePower = Results.TopSpeedEnginePower
	Data.TopSpeedEngineRPM = Results.TopSpeedEngineRPM
	
	Data.KilometerTime = Results.KilometerTime
	Data.KilometerSpeed = Results.KilometerSpeed
	Data.EightyToHundredAndTwentyTime = Results.EightyToHundredAndTwentyTime
	Data.QuarterMileTime = Results.QuarterMileTime
	Data.QuarterMileSpeed = Results.QuarterMileSpeed
	Data.SixtyTime = Results.SixtyTime
	Data.TwoHundredTime = Results.TwoHundredTime
	Data.HundredTime = Results.HundredTime
	Data.HundredAndTwentyTime = Results.HundredAndTwentyTime
	Data.EightyTime = Results.EightyTime



	Data.FamilyYear = EngineInfo.PlatformInfo.Time.Date.Year
	Data.FamilyName = EngineInfo.PlatformInfo.Name
	Data.FamilyBore = EngineInfo.PlatformInfo.Bore
	Data.FamilyStroke = EngineInfo.PlatformInfo.Stroke
	Data.FamilyDisplacement = EngineInfo.PlatformInfo.Capacity

	Data.BlockType = EngineInfo.PlatformInfo.BlockType.Name
	Data.Cylinders = EngineInfo.PlatformInfo.BlockConfig.Cylinders
	Data.BlockMaterial = EngineInfo.PlatformInfo.BlockMaterial.Name
	
	Data.HeadMaterial = EngineInfo.PlatformInfo.HeadMaterial.Name
	Data.HeadType = EngineInfo.PlatformInfo.Head.Name
	Data.IntakeValves = EngineInfo.PlatformInfo.Valves.IntakeValves
	Data.ExhaustValves = EngineInfo.PlatformInfo.Valves.ExhaustValves
	
	Data.FamilyQuality = EngineInfo.PlatformInfo.QualitySettings.Family
	Data.FamilyTechpool = EngineInfo.PlatformInfo.TechPool.Family
	Data.FamilyTopEndTechpool = EngineInfo.PlatformInfo.TechPool.TopEnd
	
	Data.VariantYear = EngineInfo.ModelInfo.Time.Date.Year
	Data.VariantName = EngineInfo.ModelInfo.Name
	Data.VariantBore = EngineInfo.ModelInfo.Bore
	Data.VariantStroke = EngineInfo.ModelInfo.Stroke
	Data.VariantDisplacement = EngineInfo.ModelInfo.Capacity
	
	Data.BalanceShaft = EngineInfo.ModelInfo.BalanceShaft.Name
	Data.BalancingMass = EngineInfo.ModelInfo.BalancingMassSetting
	Data.Crankshaft = EngineInfo.ModelInfo.Crank.Name
	Data.Conrods = EngineInfo.ModelInfo.Conrods.Name
	Data.Pistons = EngineInfo.ModelInfo.Pistons.Name
	
	Data.Compression = EngineInfo.ModelInfo.Compression
	Data.CamProfile = EngineInfo.ModelInfo.CamProfileSetting
	Data.ValveSpringStiffness = EngineInfo.ModelInfo.SpringStiffnessSetting
	
	Data.VVT = EngineInfo.ModelInfo.VVT.Name
	Data.VVL = EngineInfo.ModelInfo.VVL.Name
	Data.VVLCamProfile = EngineInfo.ModelInfo.VVLCamProfileSetting or 0
	Data.VVLRPM = EngineInfo.ModelInfo.VVLRPM or 0
	
	Data.Aspiration = EngineInfo.ModelInfo.AspirationSetup.Name
	Data.IntercoolerSize = EngineInfo.ModelInfo.IntercoolerSetting
	
	Data.AspirationItem1 = EngineInfo.ModelInfo.AspirationItemOption_1.Name
	Data.AspirationSubItem1 = EngineInfo.ModelInfo.AspirationItemSubOption_1.Name
	Data.AspirationItem2 = EngineInfo.ModelInfo.AspirationItemOption_2.Name
	Data.AspirationSubItem2 = EngineInfo.ModelInfo.AspirationItemSubOption_2.Name
	
	Data.BoostControl = EngineInfo.ModelInfo.AspirationBoostControl.Name
	Data.ChargerSize1 = EngineInfo.ModelInfo.ChargerSize_1
	Data.ChargerSize2 = EngineInfo.ModelInfo.ChargerSize_2
	Data.TurbineSize1 = EngineInfo.ModelInfo.TurbineSize_1
	Data.TurbineSize2 = EngineInfo.ModelInfo.TurbineSize_2
	Data.ChargerTune1 = EngineInfo.ModelInfo.ChargerTune_1
	Data.ChargerTune2 = EngineInfo.ModelInfo.ChargerTune_2
	Data.ChargerMaxBoost1 = EngineInfo.ModelInfo.ChargerMaxBoost_1
	Data.ChargerMaxBoost2 = EngineInfo.ModelInfo.ChargerMaxBoost_2
	
	Data.FuelSystemType = EngineInfo.ModelInfo.FuelSystemType.Name
	Data.FuelSystem = EngineInfo.ModelInfo.FuelSystem.Name
	Data.Throttles = EngineInfo.ModelInfo.IntakeManifold.Name
	Data.CarbSize = EngineInfo.ModelInfo.CarbSize or 0
	Data.Intake = EngineInfo.ModelInfo.Intake.Name
	Data.IntakeSize = EngineInfo.ModelInfo.IntakeSize or 0
	
	Data.FuelLeaded = EngineInfo.ModelInfo.Fuel.FuelLeaded == true and "TRUE" or "FALSE"
	Data.FuelOctane = EngineInfo.ModelInfo.Fuel.ListedOctane
	Data.OctaneOffset = EngineInfo.ModelInfo.Fuel.TuneOctaneOffset
	Data.FuelType = EngineInfo.ModelInfo.Fuel.Type
	
	--Data.AFR = EngineInfo.ModelInfo.AFR
	--Data.IgnitionTiming = EngineInfo.ModelInfo.IgnitionTimingSetting
	Data.FuelMap = EngineInfo.ModelInfo.FuelMap or 0
	
	Data.RPMLimit = EngineInfo.ModelInfo.RPMLimit
	
	Data.Headers = EngineInfo.ModelInfo.Headers.Name
	Data.HeaderSize = EngineInfo.ModelInfo.HeaderSize
	Data.ExhaustCount = EngineInfo.ModelInfo.ExhaustCount.Name
	Data.BypassValves = EngineInfo.ModelInfo.ExhaustBypassValves.Name
	Data.ExhaustDiameter = EngineInfo.ModelInfo.ExhaustDiameter
	Data.Cat = EngineInfo.ModelInfo.Cat.Name
	Data.Muffler1 = EngineInfo.ModelInfo.Muffler1.Name
	Data.Muffler2 = EngineInfo.ModelInfo.Muffler2.Name
	
	Data.VariantTopEndQuality = EngineInfo.ModelInfo.QualitySettings.TopEnd
	Data.VariantBottomEndQuality = EngineInfo.ModelInfo.QualitySettings.BottomEnd
	Data.VariantAspirationQuality = EngineInfo.ModelInfo.QualitySettings.Aspiration
	Data.VariantFuelSystemQuality = EngineInfo.ModelInfo.QualitySettings.FuelSystem
	Data.VariantExhaustQuality = EngineInfo.ModelInfo.QualitySettings.Exhaust
	
	Data.VariantTopEndTechpool = EngineInfo.ModelInfo.TechPool.TopEnd
	Data.VariantFamilyTechpool = EngineInfo.ModelInfo.TechPool.Family
	Data.VariantBottomEndTechpool = EngineInfo.ModelInfo.TechPool.BottomEnd
	Data.VariantAspirationTechpool = EngineInfo.ModelInfo.TechPool.Aspiration
	Data.VariantFuelSystemTechpool = EngineInfo.ModelInfo.TechPool.FuelSystem
	Data.VariantExhaustTechpool = EngineInfo.ModelInfo.TechPool.Exhaust
	

	Data.EnginePower = EngineInfo.ModelInfo.Results.PeakPower
	Data.EnginePowerRPM = EngineInfo.ModelInfo.Results.PeakPowerRPM
	Data.EngineTorque = EngineInfo.ModelInfo.Results.PeakTorque
	Data.EngineTorqueRPM = EngineInfo.ModelInfo.Results.PeakTorqueRPM
	Data.EngineMaxRPM = EngineInfo.ModelInfo.Results.MaxRPM
	Data.EngineIdleRPM = EngineInfo.ModelInfo.Results.Idle
	Data.EnginePeakBoost = EngineInfo.ModelInfo.Results.PeakBoost
	Data.EnginePeakBoostRPM = EngineInfo.ModelInfo.Results.PeakBoostRPM
	
	--Data.MinEconomyRPM = EngineInfo.ModelInfo.Results.MinEconRPM
	
	Data.EngineWeight = EngineInfo.ModelInfo.Results.Weight
	Data.EngineFlywheelWeight = EngineInfo.ModelInfo.Results.FlyWheelWeight
	Data.EngineRequiredCooling = EngineInfo.ModelInfo.Results.CoolingRequired
	Data.EnginePerformanceIndex = EngineInfo.ModelInfo.Results.PerformanceIndex
	Data.EngineResponsiveness = EngineInfo.ModelInfo.Results.Responsiveness
	Data.EngineSmoothness = EngineInfo.ModelInfo.Results.Smoothness

	Data.EngineReliability = EngineInfo.ModelInfo.Results.MTTF

	Data.EngineEmissions = EngineInfo.ModelInfo.Results.Emissions
	Data.EngineEfficiency = EngineInfo.ModelInfo.Results.EconEff
	Data.EngineNoise = EngineInfo.ModelInfo.Results.Noise
	Data.IntakeNoise = EngineInfo.ModelInfo.Results.IntakeNoise
	Data.ExhaustNoise = EngineInfo.ModelInfo.Results.ExhaustNoise
	
	Data.EngineRONResult = EngineInfo.ModelInfo.Results.RON
	
	Data.EngineEngineeringTime = EngineInfo.ModelInfo.Results.EngineeringTime
	Data.EngineProductionUnits = EngineInfo.ModelInfo.Results.ManHours
	
	Data.EngineEngineeringCost = EngineInfo.ModelInfo.Results.EngineeringCost
	Data.EngineToolingCosts = EngineInfo.ModelInfo.Results.ToolingCosts
	Data.EngineMaterialCost = EngineInfo.ModelInfo.Results.MaterialCost
	Data.EngineTotalCost = EngineInfo.ModelInfo.Results.TotalCost
	
	Data.EngineServiceCost = EngineInfo.ModelInfo.Results.ServiceCost
	
	Data.CrankMaxRPM = EngineInfo.ModelInfo.Results.PartsFailureInfo.Crank.RPM
	Data.CrankMaxTorque = EngineInfo.ModelInfo.Results.PartsFailureInfo.Crank.Torque
	Data.ConrodMaxRPM = EngineInfo.ModelInfo.Results.PartsFailureInfo.Conrods.RPM
	Data.ConrodMaxTorque = EngineInfo.ModelInfo.Results.PartsFailureInfo.Conrods.Torque
	Data.PistonMaxRPM = EngineInfo.ModelInfo.Results.PartsFailureInfo.Pistons.RPM
	Data.PistonMaxTorque = EngineInfo.ModelInfo.Results.PartsFailureInfo.Pistons.Torque
	--Data.TurboMaxRPM = EngineInfo.ModelInfo.Results.PartsFailureInfo.Turbo.RPM	

	Data.GameVersion = UAPI.GetGameVersion()

	Data.EngineUID = EngineInfo.ModelInfo.UID

	Data.EnginePlacement = CarInfo.PlatformInfo.EnginePlacement.Placement
	Data.EngineOrientation = CarInfo.PlatformInfo.EnginePlacement.Orientation

	local DiscWeightToUnsprung = 1

	Data.WeightDistribution = CarCalculator:CalculateDynamicCG(0).WeightDistribution

	--Cost calculations

	local function CalculateCost(engineeringTime, productionUnits, engineeringCost, materialCost, toolingCosts, 
									employeeCount, employeeWage, automationCoef, shiftCount)
		local employeeCostsPerShift = employeeCount * employeeWage * 8
		local factoryProductionUnits = productionUnits / (employeeCount * automationCoef)

		local carsMadePerShift = 8 / factoryProductionUnits
		local carsMadePerDay = carsMadePerShift * shiftCount
		local employeeCostsPerDay = employeeCostsPerShift * shiftCount

		local carsMadePerMonth = carsMadePerDay * 30
		local employeeCostsPerMonth = employeeCostsPerDay * 30
		local employeeCostsPerCar = employeeCostsPerShift / carsMadePerShift

		local monthlyEngineeringCosts = engineeringCost / 60
		local engineeringCostsPerCar = monthlyEngineeringCosts / carsMadePerMonth

		local toolingCosts = toolingCosts * (shiftCount / 2)

		local totalCostPerCar = materialCost + engineeringCostsPerCar + employeeCostsPerCar + toolingCosts

		return totalCostPerCar
	end --CalculateCost

		--Data.EngineEngineeringTime
		--Data.EngineProductionUnits
		--Data.EngineEngineeringCost
		--Data.EngineMaterialCost
		--Data.EngineToolingCosts

		--Data.TrimEngineeringTime
		--Data.TrimManHours
		--Data.TrimEngCosts
		--Data.MatCost
		--Data.ToolingCosts

	--Medium factory, cheap labor, medium automation, 2 shifts
	Data.TrimCostPreset0 = CalculateCost(Data.TrimEngineeringTime, Data.TrimManHours, Data.TrimEngCosts, Data.MatCost, Data.ToolingCosts,
											200, 10, 1.5, 2)
	Data.EngineCostPreset0 = CalculateCost(Data.EngineEngineeringTime, Data.EngineProductionUnits, Data.EngineEngineeringCost, Data.EngineMaterialCost, Data.EngineToolingCosts,
											200, 10, 1.5, 2)
	Data.CarCostPreset0 = Data.TrimCostPreset0 + Data.EngineCostPreset0

	--Large factory, cheap labor, medium automation, 2 shifts
	Data.TrimCostPreset1 = CalculateCost(Data.TrimEngineeringTime, Data.TrimManHours, Data.TrimEngCosts, Data.MatCost, Data.ToolingCosts,
											500, 10, 1.5, 2)
	Data.EngineCostPreset1 = CalculateCost(Data.EngineEngineeringTime, Data.EngineProductionUnits, Data.EngineEngineeringCost, Data.EngineMaterialCost, Data.EngineToolingCosts,
											500, 10, 1.5, 2)
	Data.CarCostPreset1 = Data.TrimCostPreset1 + Data.EngineCostPreset1

	--Medium factory, average labor, high automation, 2 shifts
	Data.TrimCostPreset2 = CalculateCost(Data.TrimEngineeringTime, Data.TrimManHours, Data.TrimEngCosts, Data.MatCost, Data.ToolingCosts,
											150, 20, 2, 2)
	Data.EngineCostPreset2 = CalculateCost(Data.EngineEngineeringTime, Data.EngineProductionUnits, Data.EngineEngineeringCost, Data.EngineMaterialCost, Data.EngineToolingCosts,
											150, 20, 2, 2)
	Data.CarCostPreset2 = Data.TrimCostPreset2 + Data.EngineCostPreset2

	--Medium factory, average labor, high automation, 3 shifts
	Data.TrimCostPreset3 = CalculateCost(Data.TrimEngineeringTime, Data.TrimManHours, Data.TrimEngCosts, Data.MatCost, Data.ToolingCosts,
											100, 20, 2, 3)
	Data.EngineCostPreset3 = CalculateCost(Data.EngineEngineeringTime, Data.EngineProductionUnits, Data.EngineEngineeringCost, Data.EngineMaterialCost, Data.EngineToolingCosts,
											100, 20, 2, 3)
	Data.CarCostPreset3 = Data.TrimCostPreset3 + Data.EngineCostPreset3

	--Small factory, average labor, medium automation, 2 shifts
	Data.TrimCostPreset4 = CalculateCost(Data.TrimEngineeringTime, Data.TrimManHours, Data.TrimEngCosts, Data.MatCost, Data.ToolingCosts,
											50, 20, 1.5, 2)
	Data.EngineCostPreset4 = CalculateCost(Data.EngineEngineeringTime, Data.EngineProductionUnits, Data.EngineEngineeringCost, Data.EngineMaterialCost, Data.EngineToolingCosts,
											50, 20, 1.5, 2)
	Data.CarCostPreset4 = Data.TrimCostPreset4 + Data.EngineCostPreset4

	--Small factory, cheap labor, medium automation, 3 shifts
	Data.TrimCostPreset5 = CalculateCost(Data.TrimEngineeringTime, Data.TrimManHours, Data.TrimEngCosts, Data.MatCost, Data.ToolingCosts,
											50, 10, 1.5, 3)
	Data.EngineCostPreset5 = CalculateCost(Data.EngineEngineeringTime, Data.EngineProductionUnits, Data.EngineEngineeringCost, Data.EngineMaterialCost, Data.EngineToolingCosts,
											50, 10, 1.5, 3)
	Data.CarCostPreset5 = Data.TrimCostPreset5 + Data.EngineCostPreset5

	--Tiny factory, cheap labor, no automation, 2 shifts
	Data.TrimCostPreset6 = CalculateCost(Data.TrimEngineeringTime, Data.TrimManHours, Data.TrimEngCosts, Data.MatCost, Data.ToolingCosts,
											10, 10, 1, 2)
	Data.EngineCostPreset6 = CalculateCost(Data.EngineEngineeringTime, Data.EngineProductionUnits, Data.EngineEngineeringCost, Data.EngineMaterialCost, Data.EngineToolingCosts,
											10, 10, 1, 2)
	Data.CarCostPreset6 = Data.TrimCostPreset6 + Data.EngineCostPreset6

	--Tiny factory, average labor, no automation, 2 shifts
	Data.TrimCostPreset7 = CalculateCost(Data.TrimEngineeringTime, Data.TrimManHours, Data.TrimEngCosts, Data.MatCost, Data.ToolingCosts,
											10, 20, 1, 2)
	Data.EngineCostPreset7 = CalculateCost(Data.EngineEngineeringTime, Data.EngineProductionUnits, Data.EngineEngineeringCost, Data.EngineMaterialCost, Data.EngineToolingCosts,
											10, 20, 1, 2)
	Data.CarCostPreset7 = Data.TrimCostPreset7 + Data.EngineCostPreset7

	--Tiny factory, expensive labor, no automation, 1 shift
	Data.TrimCostPreset8 = CalculateCost(Data.TrimEngineeringTime, Data.TrimManHours, Data.TrimEngCosts, Data.MatCost, Data.ToolingCosts,
											10, 30, 1, 1)
	Data.EngineCostPreset8 = CalculateCost(Data.EngineEngineeringTime, Data.EngineProductionUnits, Data.EngineEngineeringCost, Data.EngineMaterialCost, Data.EngineToolingCosts,
											10, 30, 1, 1)
	Data.CarCostPreset8 = Data.TrimCostPreset8 + Data.EngineCostPreset8

	--Medium factory, very cheap labor, low automation, 3 shifts
	Data.TrimCostPreset9 = CalculateCost(Data.TrimEngineeringTime, Data.TrimManHours, Data.TrimEngCosts, Data.MatCost, Data.ToolingCosts,
											200, 5, 1.2, 3)
	Data.EngineCostPreset9 = CalculateCost(Data.EngineEngineeringTime, Data.EngineProductionUnits, Data.EngineEngineeringCost, Data.EngineMaterialCost, Data.EngineToolingCosts,
											200, 5, 1.2, 3)
	Data.CarCostPreset9 = Data.TrimCostPreset9 + Data.EngineCostPreset9

	--Large factory, very cheap labor, low automation, 3 shifts
	Data.TrimCostPreset10 = CalculateCost(Data.TrimEngineeringTime, Data.TrimManHours, Data.TrimEngCosts, Data.MatCost, Data.ToolingCosts,
											500, 5, 1.2, 3)
	Data.EngineCostPreset10 = CalculateCost(Data.EngineEngineeringTime, Data.EngineProductionUnits, Data.EngineEngineeringCost, Data.EngineMaterialCost, Data.EngineToolingCosts,
											500, 5, 1.2, 3)
	Data.CarCostPreset10 = Data.TrimCostPreset10 + Data.EngineCostPreset10

	--Large factory, average labor, medium automation, 2 shifts
	Data.TrimCostPreset11 = CalculateCost(Data.TrimEngineeringTime, Data.TrimManHours, Data.TrimEngCosts, Data.MatCost, Data.ToolingCosts,
											500, 20, 1.5, 2)
	Data.EngineCostPreset11 = CalculateCost(Data.EngineEngineeringTime, Data.EngineProductionUnits, Data.EngineEngineeringCost, Data.EngineMaterialCost, Data.EngineToolingCosts,
											500, 20, 1.5, 2)
	Data.CarCostPreset11 = Data.TrimCostPreset11 + Data.EngineCostPreset11

	--Small factory, expensive labor, high automation, 2 shifts
	Data.TrimCostPreset12 = CalculateCost(Data.TrimEngineeringTime, Data.TrimManHours, Data.TrimEngCosts, Data.MatCost, Data.ToolingCosts,
											50, 30, 2, 2)
	Data.EngineCostPreset12 = CalculateCost(Data.EngineEngineeringTime, Data.EngineProductionUnits, Data.EngineEngineeringCost, Data.EngineMaterialCost, Data.EngineToolingCosts,
											50, 30, 2, 2)
	Data.CarCostPreset12 = Data.TrimCostPreset12 + Data.EngineCostPreset12

	--Tiny factory, expensive labor, high automation, 2 shifts
	Data.TrimCostPreset13 = CalculateCost(Data.TrimEngineeringTime, Data.TrimManHours, Data.TrimEngCosts, Data.MatCost, Data.ToolingCosts,
											10, 30, 2, 2)
	Data.EngineCostPreset13 = CalculateCost(Data.EngineEngineeringTime, Data.EngineProductionUnits, Data.EngineEngineeringCost, Data.EngineMaterialCost, Data.EngineToolingCosts,
											10, 30, 2, 2)
	Data.CarCostPreset13 = Data.TrimCostPreset13 + Data.EngineCostPreset13

	return Data
end


function CExporter.ExportCarFiles(CarCalculator)
	--local CarInfo = CarCalculator.CarInfo
	Files = { }
	--local AestheticsFile = ""
	--AestheticsFile = AestheticsFile .. "CarCalculator.CarInfo.TrimInfo.AestheticSettings.WindowTint = " .. CarCalculator.CarInfo.TrimInfo.AestheticSettings.WindowTint .. "\n"
	--AestheticsFile = AestheticsFile .. "CarCalculator.CarInfo.TrimInfo.AestheticSettings.WindowTransparency = " .. CarCalculator.CarInfo.TrimInfo.AestheticSettings.WindowTransparency .. "\n"
	
	--Files["AestheticSettings.txt"] = AestheticsFile

	return Files
end