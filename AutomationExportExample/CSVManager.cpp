#include "CSVManager.h"

#include "stdafx.h"
#include <codecvt>

#include "BuildDate.h"
#include "MessageBoxManager.h"

CSVManager::CSVManager(std::wstring filepath, std::wstring delimiter, bool requireSameHeader) :
	m_FilePath(filepath),
	m_Delimiter(delimiter),
	m_RequireSameHeader(requireSameHeader),
	m_ExportManager(AuExpManager::GetInstance())
{
}

/*
void CSVManager::AddData(const LuaData& data)
{
	if (VerifyHeaders(data))
	{
		m_Data.push_back(data);
	}
	else
	{
		throw "Non-matching headers encountered";
	}
}
*/

std::wstring CSVManager::BuildCSVHeaderString(std::vector<LuaDataHeader>& luaDataHeaders, const std::locale& locale) const
{
	std::wstringstream stringStream;
	stringStream.imbue(locale);

	size_t dataCount = luaDataHeaders.size();
	for (size_t i = 0; i < dataCount; i++)
	{
		if (i > 0)
			stringStream << m_Delimiter;

		stringStream << luaDataHeaders[i].HeaderTitle;
	}
	return stringStream.str();
}

void CSVManager::WriteData(std::wofstream& outputFile, std::vector<LuaDataHeader>& luaDataHeaders) const
{
	outputFile << L"\n";
	//std::wstringstream stringStream;
	size_t dataCount = luaDataHeaders.size();
	for (size_t i = 0; i < dataCount; i++)
	{
		if (i > 0)
			outputFile << m_Delimiter;

		const LuaDataHeader& data = luaDataHeaders[i];
		switch (data.DataType)
		{
		case DataType_String:
		{
			std::wstring string = L"???";
			m_ExportManager->TryGetStringData(data.HeaderName, string);
			if (!data.DoNotTranslate)
			{
				m_ExportManager->GetTranslation(&string);
			}
			outputFile << string;
		}
			break;
		case DataType_Float:
		{
			float num;
			if (m_ExportManager->TryGetFloatData(data.HeaderName, &num))
			{
				outputFile << std::to_wstring(num);
			}
			else
			{
				outputFile << L"???";
			}
		}
			break;
		case DataType_CarName:
			outputFile << m_ExportManager->GetCarName();
			break;
		case DataType_ExporterVersion:
			outputFile << BUILD_DATE_INT;
			break;
		default:
			// error out
			break;
		}
	}
}

void CSVManager::Save(std::vector<LuaDataHeader>& luaDataHeaders) const
{
	std::wstring localeName = m_ExportManager->GetLocale();
	std::locale locale(std::locale(std::locale::classic(), new std::codecvt_utf8_utf16<wchar_t>));
	if (!localeName.empty())
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
		std::string localeNameA = converter.to_bytes(localeName);
		try 
		{
			std::locale baseLocale(localeNameA);
			//std::locale localeWithUTF16(baseLocale, new std::codecvt_utf8_utf16<wchar_t>);
			locale = std::locale(baseLocale, new std::codecvt_utf8_utf16<wchar_t>);
			//outputFile.imbue(localeWithUTF16);
		}
		catch (const std::runtime_error&) 
		{
			//outputFile.imbue(std::locale(std::locale::classic(), new std::codecvt_utf8_utf16<wchar_t>));
			//locale = std::locale(std::locale::classic(), new std::codecvt_utf8_utf16<wchar_t>);
			//TODO
		}
	}

	DWORD att = GetFileAttributes(m_FilePath.c_str());
	if (att != INVALID_FILE_ATTRIBUTES && att != FILE_ATTRIBUTE_DIRECTORY)
	{
		// check headers
		if (m_RequireSameHeader)
		{
			std::wstring headerString = BuildCSVHeaderString(luaDataHeaders, locale);
			bool isSameHeader = CheckSameHeader(headerString, locale);
			if (!isSameHeader)
			{
				// error out
				MessageBoxManager::ShowMessageBox(L"The exported CSV header does not match the existing file header", L"Failed to write CSV");
				return;
			}
		}

		std::wofstream outputFile(m_FilePath, std::ios::app | std::ios::binary);
		outputFile.imbue(locale);

		WriteData(outputFile, luaDataHeaders);
		outputFile.close();
		
	}
	else if (att == FILE_ATTRIBUTE_DIRECTORY)
	{
		// error out
	}
	else
	{
		//make new file safely
		std::wstring headerString = BuildCSVHeaderString(luaDataHeaders, locale);
		std::wofstream outputFile(m_FilePath, std::ios::out | std::ios::binary);
		outputFile.imbue(locale);
		WriteHeader(outputFile, headerString);
		WriteData(outputFile, luaDataHeaders);
		outputFile.close();
	}
}

bool CSVManager::CheckSameHeader(const std::wstring& headerString, const std::locale& locale) const
{
	std::wifstream readFileStream(m_FilePath.c_str(), std::ios::in | std::ios::binary);
	readFileStream.imbue(locale);

	bool isSameHeader = false;

	std::wstring firstLine;
	if (std::getline(readFileStream, firstLine)) {
		if (!firstLine.empty() && firstLine.back() == L'\r') {
			firstLine.pop_back();
		}

		const wchar_t UTF8Bom = L'\xFEFF';
		if (!firstLine.empty() && firstLine[0] == UTF8Bom) {
			firstLine.erase(0, 1);
		}
		isSameHeader = firstLine == headerString;
	}

	/*
	wchar_t currentChar;
	size_t currentIndex = 0;
	bool sameHeader = true;
	while (readFileStream >> currentChar)
	{
		if (currentChar == L'\n' || currentChar == L'\r')
			break;

		if (currentChar != headerString[currentIndex])
		{
			sameHeader = false;
			break;
		}

		currentIndex++;
	}
	*/

	readFileStream.close();
	return isSameHeader;
}
