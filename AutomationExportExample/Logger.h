#pragma once

#include <cstdio>
#include <string>
#include <ShlObj.h>
#include <mutex>

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
    static void CreateLogger(const std::wstring& filename, bool truncateFile = false) { s_Logger = new Logger(filename, truncateFile); }

    void Log(const std::wstring& message) {
        if (!m_File || m_FD < 0)
        {
            return;
        }

        std::lock_guard<std::mutex> lock(m_Mutex);

        std::fputws(message.c_str(), m_File);
        std::fputws(L"\n", m_File);

        //commit(m_FD);
    }

    static Logger* GetLogger() { return s_Logger; }
    static void FreeLogger() 
    {
        if (s_Logger)
        {
            delete s_Logger;
            s_Logger = nullptr;
        }
    }

private:
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

        const wchar_t* fileMode = truncateFile ? L"w, ccs=UTF-8" : L"a, ccs=UTF-8";

        errno_t err = _wfopen_s(&m_File, filePath.c_str(), fileMode);
        if (!err && m_File) {
            std::setvbuf(m_File, nullptr, _IONBF, 0);
            m_FD = fileno(m_File);
        }
    }

    ~Logger() {
        if (m_File) {
            std::fclose(m_File);
        }
    }

    static Logger* s_Logger;
    std::FILE* m_File = nullptr;
    int m_FD = -1;
    std::mutex m_Mutex;
};