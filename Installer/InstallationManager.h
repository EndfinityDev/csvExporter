#pragma once
#include <string>
#include <functional>
#include <iostream>
#include <windows.h>
#include <fstream>

#include <miniz.h>

#include "DirectoryManager.h"
#include "StringConverter.h"

class InstallationManager
{
public:
	static bool UnpackResource(unsigned int resourceID, const std::wstring& targetDir, 
								std::function<bool(const std::wstring&, const char*, size_t, bool, const wchar_t*)> unpackFunction,
								bool overwriteExisting = false,
								const wchar_t* fileName = nullptr);

	static bool WriteFile(const std::wstring& targetDir, const char* data, size_t dataSize, bool overwriteExisting, const wchar_t* fileName);
	static bool UnpackArchive(const std::wstring& targetDir, const char* data, size_t dataSize, bool overwriteExisting, const wchar_t* fileName);
};

