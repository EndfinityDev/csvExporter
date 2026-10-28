local original_ExportCarFiles = CExporter.ExportCarFiles

local function GetParamNames(func)
    if not debug or not debug.getinfo then
        return "..."
    end
    local info = debug.getinfo(func, "u")
    local nparams = info.nparams or 0
    local params = {}
    
    for i = 1, nparams do
        local name = debug.getlocal(func, i)
        if name then
            table.insert(params, name)
        else
            table.insert(params, "arg" .. i)
        end
    end
    
    if info.isvararg then
        table.insert(params, "...")
    end
    
    return table.concat(params, ", ")
end

local function jsonEscape(str)
    return str:gsub("\\", "\\\\"):gsub('"', '\\"'):gsub("\n", "\\n"):gsub("\r", "\\r"):gsub("\t", "\\t")
end

local function DumpTableToJson(inTable, buffer, visited, functionsOnly, depth)
    buffer = buffer or {}
    depth = depth or 0
    local indent = string.rep("  ", depth)
    local childIndent = string.rep("  ", depth + 1)
    
    visited = visited or {}
    if visited[inTable] then
        table.insert(buffer, '"<Circular Reference / Already Dumped>"')
        return buffer
    end
    visited[inTable] = true

    table.insert(buffer, "{\n")
    local isFirst = true

    local function writePair(key, valueStr)
        if not isFirst then table.insert(buffer, ",\n") end
        isFirst = false
        table.insert(buffer, string.format('%s"%s": %s', childIndent, jsonEscape(tostring(key)), valueStr))
    end

    for k, v in pairs(inTable) do
        local t = type(v)
        if t == "table" and not functionsOnly then
            if not visited[v] then
                if not isFirst then table.insert(buffer, ",\n") end
                isFirst = false
                table.insert(buffer, string.format('%s"%s": ', childIndent, jsonEscape(tostring(k))))
                DumpTableToJson(v, buffer, visited, false, depth + 1)
            else
                writePair(k, '"<Circular Reference / Already Dumped>"')
            end
        elseif t == "function" then
            local paramList = GetParamNames(v)
            writePair(k, string.format('{\n%s"type": "function",\n%s"parameters": "(%s)"\n%s}', childIndent .. "  ", childIndent .. "  ", jsonEscape(paramList), childIndent))
        elseif not functionsOnly then
            if t == "number" or t == "boolean" then
                writePair(k, tostring(v))
            else
                writePair(k, string.format('"%s"', jsonEscape(tostring(v))))
            end
        end
    end

    local mt = getmetatable(inTable)
    if mt and type(mt.__index) == "table" then
        if not isFirst then table.insert(buffer, ",\n") end
        isFirst = false
        table.insert(buffer, string.format('%s"__metatable_methods__": ', childIndent))
        DumpTableToJson(mt.__index, buffer, visited, true, depth + 1)
    end

    table.insert(buffer, "\n" .. indent .. "}")
    
    visited[inTable] = nil
    return buffer
end

function CExporter.ExportCarFiles(CarCalculator)
    local files = original_ExportCarFiles(CarCalculator)

    local buffer = DumpTableToJson(CarCalculator)
    files["CarDataDump.json"] = table.concat(buffer)

    return files
end
