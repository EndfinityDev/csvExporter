-- Example for using custom export parameters, meant to be loaded after the main file with multi-file Lua loading

local original_ExportCarFiles = CExporter.ExportCarFiles

function CExporter.ExportCarFiles(CarCalculator)
	files = original_ExportCarFiles(CarCalculator)

	local exportParamsDump = "ExportParameters:\n"

	for key, value in pairs(ExportParameters.StringParameters) do
		exportParamsDump = exportParamsDump .. "\n" .. key .. " = " .. value .. " (" .. tostring(type(value)) .. ")"
	end

	for key, value in pairs(ExportParameters.BoolParameters) do
		exportParamsDump = exportParamsDump .. "\n" .. key .. " = " .. tostring(value) .. " (" .. tostring(type(value)) .. ")"
	end

	files["ExportParameters.txt"] = exportParamsDump

	return files
end