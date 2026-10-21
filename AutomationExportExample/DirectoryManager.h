#pragma once
#include "stdafx.h"

#include <string>
#include <vector>

#include "StringUtils.h"

class DirectoryManager
{
public:
	static bool TryCreateDirectoryIfNotExists(const std::wstring& path, DWORD* outAtt = nullptr)
	{
		bool creationFailed = false;
		DWORD att = GetFileAttributes(path.c_str());

		if (att == INVALID_FILE_ATTRIBUTES || !(att & FILE_ATTRIBUTE_DIRECTORY))
		{
			CreateDirectory(path.c_str(), nullptr);
			att = GetFileAttributes(path.c_str());
			creationFailed = att == INVALID_FILE_ATTRIBUTES || !(att & FILE_ATTRIBUTE_DIRECTORY);
		}
		if (outAtt)
		{
			*outAtt = att;
		}
		return !creationFailed;
	}

	static bool TryCreateDirectoryRecursive(const std::wstring& path, DWORD* outAtt = nullptr)
	{
		//create directory, one level at a time:
		size_t slashPos = FindDirDelimiter(path, 0);
		size_t offset = 0;

		//bool directoryCreated = false;

		while (slashPos != std::wstring::npos)
		{
			//CreateDirectory(m_ExportDirectory.substr(offset, slashPos - offset).c_str(), nullptr);
			//directoryCreated = DirectoryManager::TryCreateDirectoryIfNotExists(path.substr(offset, slashPos - offset).c_str(), outAtt);
			if (!TryCreateDirectoryIfNotExists(path.substr(offset, slashPos - offset).c_str(), outAtt))
			{
				return false;
			}
			slashPos = FindDirDelimiter(path, slashPos + 1);
		}

		//last one:
		//CreateDirectory(m_ExportDirectory.c_str(), nullptr);
		//att = GetFileAttributes(m_ExportDirectory.c_str());
		//if (DirectoryManager::TryCreateDirectoryIfNotExists(path.c_str(), outAtt))
		//{
		//	return true;
		//}

		return TryCreateDirectoryIfNotExists(path.c_str(), outAtt);
	}

	static std::vector<std::wstring> GetDirectoryFilesWithExt(const std::wstring& path, const std::vector<std::wstring>& extensions)
	{
		std::vector<std::wstring> foundFiles;

		std::wstring searchQuery = path + L"\\*";

		WIN32_FIND_DATAW fileData;
		HANDLE searchHandle = FindFirstFileW(searchQuery.c_str(), &fileData);
		if (searchHandle == INVALID_HANDLE_VALUE) 
		{
			//TODO
			return foundFiles; // No files found or directory doesn't exist
		}

		do 
		{
			// Skip directories
			if (!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) 
			{
				std::wstring fileName = fileData.cFileName;
				for (const auto& ext : extensions) 
				{
					if (StringUtils::CheckEndsWith(fileName, ext)) 
					{
						foundFiles.push_back(fileName);
						break;
					}
				}
			}
		} 
		while (FindNextFileW(searchHandle, &fileData) != 0);

		FindClose(searchHandle);
		return foundFiles;
	}

	inline static bool CheckPathExists(const std::wstring& path)
	{
		DWORD att = GetFileAttributes(path.c_str());
		return att != INVALID_FILE_ATTRIBUTES;
	}

	inline static bool CheckPathIsDirectory(const std::wstring& path)
	{
		DWORD att = GetFileAttributes(path.c_str());
		return att != INVALID_FILE_ATTRIBUTES && (att & FILE_ATTRIBUTE_DIRECTORY);
	}

private:
	inline static size_t FindDirDelimiter(const std::wstring& dir, size_t start)
	{
		size_t slashPos = dir.find(L"\\", start);

		if (slashPos == std::wstring::npos)
		{
			return dir.find(L"/", start);
		}

		return slashPos;
	}
};