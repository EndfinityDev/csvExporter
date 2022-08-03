#include "CSVManager.h"
#include "stdafx.h"

CSVManager::CSVManager(std::wstring filepath, std::wstring delimiter) :
	m_FilePath(filepath),
	m_Delimiter(delimiter)
{
}

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

void CSVManager::Save() const
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

bool CSVManager::TryCompareFileHeaders(FileManager fileManager) const
{
	if (fileManager.CheckFileExists(m_FilePath.c_str()))
	{
		return true;
		/*
		std::wstring header = L"";
		header += m_Data[0][0].HeaderTitle;

		for (uint32_t i = 1; i < m_Data[0].Size(); i++)
		{
			header += m_Delimeter;
			header += m_Data[0][i].HeaderTitle;
		}
		header += L"\n";

		std::wstring fileHeader = fileManager.ReadFileHeader(m_FilePath.c_str());

		if (fileHeader == header)
			return true;
		*/
	}

	return false;
}