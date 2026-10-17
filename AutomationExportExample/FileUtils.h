#pragma once
#include <string>

class FileUtils
{
public:
	static std::wstring ValidateFileName(std::wstring filename, std::wstring defaultExtension)
	{
		size_t filenameSize = filename.size();

		for (size_t i = filenameSize; i > 0; i--)
		{
			if (filename[i - 1] != L' ')
				break;
			filename[i - 1] = L'\0'; // Remove trailing whitespaces by replacing them with a null terminator
		}

		std::wstring sanitizedName = L"";
		bool startOfString = true;

		for (size_t i = 0; i < filenameSize; i++)
		{
			wchar_t currentChar = filename[i];

			if (currentChar == L' ' && startOfString)
				continue;
			else
				startOfString = false;

			//if (currentChar == L'\\' || currentChar == L'\'' || currentChar == L'"' || currentChar == L'/')
			//	return false;

			sanitizedName += currentChar;
		}

		bool hasExtension = false;
		for (size_t i = 1; i < sanitizedName.size(); i++)
		{
			if (sanitizedName[i] == L'.')
			{
				hasExtension = true;
				break;
			}
		}

		if (!hasExtension)
		{
			sanitizedName += defaultExtension;
		}

		return sanitizedName;
	}
};