const fs = require('fs');

const expPrefsVersion = 0;

let outputText = "#pragma once\n\n#define EXPVERSION ";
let date = new Date();
outputText = outputText + date.getFullYear() + (date.getMonth() + 1)
let day = date.getDate();
if (day < 10) {
    outputText = outputText + "0" + day;
}
else {
    outputText = outputText + day;
}

outputText = outputText + "\n#define EXPVERSIONWS std::to_wstring(EXPVERSION)\n\n";
outputText = outputText + "#define EXPPREFSVERSION " + expPrefsVersion + "\n#define EXPPREFSVERSIONWS std::to_string(EXPPREFSVERSION)";

let path = process.argv[2]
//console.log(path);

try {
    const readFileData = fs.readFileSync(path + "AutomationExportExample\\EXPVERSION.h", 'utf8');

    if(readFileData != outputText) {
    fs.writeFileSync(path + "AutomationExportExample\\EXPVERSION.h", outputText);
    // file written successfully
    console.log("Updated build version");
    }
} 
catch (err) {
    console.error(err);
}
//console.log(outputText);