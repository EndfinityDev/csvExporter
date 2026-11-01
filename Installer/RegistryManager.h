#pragma once
#include <windows.h>
#include <string>
#include <iostream>

class RegistryManager
{
public:
    static std::wstring GetSteamInstallPath() {
        HKEY hKey;
        // Open the 32-bit registry view where Steam stores its location on 64-bit systems
        LONG lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\WOW6432Node\\Valve\\Steam", 0, KEY_READ, &hKey);

        if (lRes != ERROR_SUCCESS) {
            // Fallback for native 32-bit OS layout configurations
            lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Valve\\Steam", 0, KEY_READ, &hKey);
        }

        if (lRes == ERROR_SUCCESS) {
            wchar_t szBuffer[MAX_PATH];
            DWORD dwBufferSize = sizeof(szBuffer);

            // Read the "InstallPath" value string string
            lRes = RegQueryValueExW(hKey, L"InstallPath", NULL, NULL, (LPBYTE)szBuffer, &dwBufferSize);
            RegCloseKey(hKey);

            if (lRes == ERROR_SUCCESS) {
                size_t charCount = (dwBufferSize / sizeof(wchar_t)) - 1;
                return std::wstring(szBuffer, charCount);
            }
        }
        return L"";
    }
};