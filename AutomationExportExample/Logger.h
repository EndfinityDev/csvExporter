#pragma once

#include <cstdio>
#include <string>
#include <ShlObj.h>

#ifdef _WIN32
#include <io.h>
#define fileno _fileno
//#define commit _commit
#else
#include <unistd.h>
#define commit fsync
#endif

#include "DirectoryManager.h"

//#define CSVE_LOGGER_PATH L"\\AutomationGame\\CSVExporter 2\\Logs\\"
static const std::wstring LOGGER_PATH = L"\\AutomationGame\\CSVExporter 2\\Logs";

class Logger {
public:
    Logger(const std::wstring& filename, bool truncateFile = false) 
    {
        TCHAR path[MAX_PATH];
        std::wstring filePath;
        if (SHGetFolderPathW(0, CSIDL_LOCAL_APPDATA, 0, SHGFP_TYPE_CURRENT, path) == S_OK)
        {
            filePath = path + LOGGER_PATH;
        }
        else
        {
            return;
        }

        if (!DirectoryManager::CheckPathIsDirectory(filePath))
        {
            if (!DirectoryManager::TryCreateDirectoryRecursive(filePath))
            {
                std::wstring error = L"Could not create directory: ";
                error += filePath;

                MessageBox(nullptr, error.c_str(), TEXT("Error creating directory"), MB_OK);

                return;
            }
        }

        filePath += L"\\" + filename;

        const wchar_t* fileMode = truncateFile ? L"w, cccs=UTF-8" : L"a, cccs=UTF-8";

        m_File = _wfopen(filePath.c_str(), fileMode);
        if (m_File) {
            std::setvbuf(m_File, nullptr, _IONBF, 0);
            m_FD = fileno(m_File);
        }
    }

    ~Logger() {
        if (m_File) {
            std::fclose(m_File);
        }
    }

    void log(const std::wstring& message) {
        if (!m_File || m_FD < 0)
        {
            return;
        }

        std::fputws(message.c_str(), m_File);
        std::fputws(L"\n", m_File);

        //commit(m_FD);
    }

private:
    std::FILE* m_File = nullptr;
    int m_FD = -1;
};