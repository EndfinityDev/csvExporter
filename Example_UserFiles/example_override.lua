-- Example for overriding Lua code by loading this file later than the target

local original_ExportCarFiles = CExporter.ExportCarFiles

function CExporter.ExportCarFiles(CarCalculator)
	files = original_ExportCarFiles(CarCalculator)

	local AestheticsFile = ""
	AestheticsFile = AestheticsFile .. "CarCalculator.CarInfo.TrimInfo.AestheticSettings.WindowTint = " .. CarCalculator.CarInfo.TrimInfo.AestheticSettings.WindowTint .. "\n"
	AestheticsFile = AestheticsFile .. "CarCalculator.CarInfo.TrimInfo.AestheticSettings.WindowTransparency = " .. CarCalculator.CarInfo.TrimInfo.AestheticSettings.WindowTransparency .. "\n"
	
	files["AestheticSettings.txt"] = AestheticsFile

	return files
end