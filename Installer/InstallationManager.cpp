#include "InstallationManager.h"


bool InstallationManager::UnpackResource(unsigned int resourceID, const std::wstring& targetDir, 
                                            std::function<bool(const std::wstring&, const char*, size_t, bool, const wchar_t*)> unpackFunction,
                                            bool overwriteExisting,
                                            const wchar_t* fileName)
{

    // 1. Locate the embedded resource block in the current EXE module
    HRSRC hRes = FindResourceW(NULL, MAKEINTRESOURCEW(resourceID), RT_RCDATA);
    if (!hRes)
    {
        return false;
    }

    // 2. Load the resource data metadata references into memory
    HGLOBAL hData = LoadResource(NULL, hRes);
    if (!hData)
    {
        return false;
    }

    DWORD dataSize = SizeofResource(NULL, hRes);
    char* pData = (char*)LockResource(hData);
    if (!pData)
    {
        return false;
    }

    // 3. Create the destination folder directory structure if missing
    if (!DirectoryManager::CheckPathIsDirectory(targetDir))
    {
        if (!DirectoryManager::TryCreateDirectoryRecursive(targetDir))
        {
            return false;
        }
    }

    return unpackFunction(targetDir, pData, dataSize, overwriteExisting, fileName);
}

bool InstallationManager::WriteFile(const std::wstring& targetDir, const char* data, size_t dataSize, bool overwriteExisting, const wchar_t* fileName)
{
    if (!fileName)
    {
        return false;
    }

    std::wstring filePath = targetDir + L"\\" + fileName;

    if (!overwriteExisting && DirectoryManager::CheckPathExists(filePath))
    {
        return true;
    }

    std::ofstream outFile(filePath, std::ios::binary);
    if (!outFile)
    {
        return false;
    }

    outFile.write(data, dataSize);
    outFile.close();

    return true;
}

bool InstallationManager::UnpackArchive(const std::wstring& targetDir, const char* data, size_t dataSize, bool overwriteExisting, const wchar_t* fileName)
{
    bool success = true;
    mz_zip_archive archive;
    memset(&archive, 0, sizeof(archive));
    std::vector<char> fileBuffer;

    if (!mz_zip_reader_init_mem(&archive, data, dataSize, 0)) 
    {
        return false;
    }

    mz_uint numFiles = mz_zip_reader_get_num_files(&archive);
    for (mz_uint i = 0; i < numFiles; i++)
    {
        mz_zip_archive_file_stat fileStat;
        if (!mz_zip_reader_file_stat(&archive, i, &fileStat)) 
        {
            continue;
        }

        std::wstring internalFilePath = StringConverter::to_wstring(fileStat.m_filename);
        DirectoryManager::NormalizePath(internalFilePath);
        std::wstring outputFilePath = targetDir + L"\\" + internalFilePath;

        if (mz_zip_reader_is_file_a_directory(&archive, i))
        {
            if (!DirectoryManager::TryCreateDirectoryRecursive(outputFilePath))
            {
                success = false;
                break;
            }
            continue;
        }

        size_t lastSlash = outputFilePath.find_last_of(L"\\");
        if (lastSlash != std::wstring::npos)
        {
            std::wstring parentDir = outputFilePath.substr(0, lastSlash);
            if (!DirectoryManager::TryCreateDirectoryRecursive(parentDir))
            {
                success = false;
                break;
            }
        }

        if (!overwriteExisting && DirectoryManager::CheckPathExists(outputFilePath))
        {
            continue;
        }

        /*
        std::string outputFilePathUTF8 = StringConverter::to_utf8string(outputFilePath);
        if (!mz_zip_reader_extract_to_file(&archive, i, outputFilePathUTF8.c_str(), 0))
        {
            success = false;
            break;
        }
        */

        size_t uncompressedSize = static_cast<size_t>(fileStat.m_uncomp_size);
        if (uncompressedSize > 0)
        {
            fileBuffer.resize(uncompressedSize);
        }
        if (!mz_zip_reader_extract_to_mem(&archive, i, fileBuffer.data(), uncompressedSize, 0))
        {
            success = false;
            break;
        }
        std::ofstream outFile(outputFilePath, std::ios::binary);
        if (!outFile)
        {
            success = false;
            break;
        }
        if (uncompressedSize > 0)
        {
            outFile.write(fileBuffer.data(), fileBuffer.size());
        }
        outFile.close();
    }

    mz_zip_reader_end(&archive);

    return success;
}
