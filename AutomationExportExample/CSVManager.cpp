#include "CSVManager.h"
#include "stdafx.h"
#include <codecvt>

CSVManager::CSVManager(std::wstring filepath, std::wstring delimiter) :
	m_FilePath(filepath),
	m_Delimiter(delimiter),
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

std::wstring CSVManager::BuildCSVHeaderString() const
{
	std::wstringstream stringStream;
	size_t dataCount = m_Data->size();
	for (size_t i = 0; i < dataCount; i++)
	{
		if (i > 0)
			stringStream << m_Delimiter;

		stringStream << m_Data->at(i).HeaderTitle;
	}
	return stringStream.str();
}

void CSVManager::WriteData(std::wofstream& outputFile) const
{
	outputFile << L"\n";
	//std::wstringstream stringStream;
	size_t dataCount = m_Data->size();
	for (size_t i = 0; i < dataCount; i++)
	{
		if (i > 0)
			outputFile << m_Delimiter;

		const LuaDataHeader& data =  m_Data->at(i);
		switch (data.DataType)
		{
		case DataType_String:
		{
			std::wstring string = L"???";
			m_ExportManager->TryGetStringData(data.HeaderName, string);
			m_ExportManager->GetTranslation(&string);
			outputFile << string;
		}
			break;
		case DataType_Float:
		{
			float num;
			if (m_ExportManager->TryGetFloatData(data.HeaderName, &num))
			{
				outputFile << num;
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
			outputFile << EXPVERSION;
			break;
		default:
			// error out
			break;
		}
	}
}

void CSVManager::Save() const
{
	std::wstring headerString = BuildCSVHeaderString();

	DWORD att = GetFileAttributes(m_FilePath.c_str());
	if (att != INVALID_FILE_ATTRIBUTES && att != FILE_ATTRIBUTE_DIRECTORY)
	{
		// check headers
		std::wifstream readFileStream(m_FilePath.c_str());
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

		readFileStream.close();

		if (!sameHeader)
		{
			// error out
		}

		std::wofstream outputFile(m_FilePath, std::ios::app);
		
		std::wstring localeName = m_ExportManager->GetLocale();
		if (!localeName.empty())
		{
			std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
			std::string localeNameA = converter.to_bytes(localeName);
			std::locale locale(localeNameA);
			outputFile.imbue(locale);
		}

		WriteData(outputFile);
		outputFile.close();
		
	}
	else if (att == FILE_ATTRIBUTE_DIRECTORY)
	{
		// error out
	}
	else
	{
		//make new file safely
		std::wofstream outputFile(m_FilePath);
		WriteHeader(outputFile, headerString);
		WriteData(outputFile);
		outputFile.close();
	}
}

/*
void CSVManager::SaveOld() const
{
	if (m_Data.size() == 0 || m_Data[0].Size() == 0)
		throw "No data to write was found";

	FileManager fileManager = FileManager();

	FILE* file;

	if (TryCompareFileHeaders(fileManager)) 
	{
		file = fileManager.OpenFile(m_FilePath.c_str(), L"ab");
	}
	else
	{
		file = fileManager.OpenFile(m_FilePath.c_str(), L"wb");

		if (!file)
		{
			MessageBox(NULL, m_FilePath.c_str(), TEXT("Failed to open file"), 0);
			return;
		}

		// Populating the header
		for (uint32_t i = 0; i < m_Data[0].Size(); i++)
		{
			if (i != 0)
				fwprintf_s(file, m_Delimiter.c_str());
			fwprintf_s(file, m_Data[0][i].HeaderTitle.c_str());
		}

		fwprintf_s(file, L"\n");
	}

	// Populating the data
	for (uint32_t i = 0; i < m_Data.size(); i++) // Rows
	{
		for (uint32_t j = 0; j < m_Data[i].Size(); j++) // Columns
		{
			if (j != 0)
				fwprintf_s(file, m_Delimiter.c_str());

			fwprintf_s(file, m_Data[i][j].Data.c_str());
		}
		fwprintf_s(file, L"\n");
	}
}
*/

/*
bool CSVManager::VerifyHeaders(const LuaData& data)
{
	if (m_Data.size() == 0)
		return true;

	LuaData& tempData = m_Data[0];
	uint32_t size = tempData.Size();

	if (size != data.Size())
		return false;

	for (uint32_t i = 0; i < size; i++)
	{
		if (tempData[i].HeaderName != data[i].HeaderName)
			return false;
	}

	return true;
}
*/

//bool CSVManager::TryCompareFileHeaders(FileManager fileManager) const
//{
//	if (fileManager.CheckFileExists(m_FilePath.c_str()))
//	{
//		return true;
//		/*
//		std::wstring header = L"";
//		header += m_Data[0][0].HeaderTitle;
//
//		for (uint32_t i = 1; i < m_Data[0].Size(); i++)
//		{
//			header += m_Delimeter;
//			header += m_Data[0][i].HeaderTitle;
//		}
//		header += L"\n";
//
//		std::wstring fileHeader = fileManager.ReadFileHeader(m_FilePath.c_str());
//
//		if (fileHeader == header)
//			return true;
//		*/
//	}
//
//	return false;
//}