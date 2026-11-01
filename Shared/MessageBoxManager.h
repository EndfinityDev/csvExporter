#pragma once
#include <string>
#include <Shlobj.h>
#include <Windows.h>
#include <Shellapi.h>
#pragma comment(lib, "Shell32.lib")

class MessageBoxManager
{
public:
	static inline void ShowMessageBox(const std::wstring& text, const std::wstring& title)
	{
		MessageBox(NULL, text.c_str(), title.c_str(), 0);
	}

	static inline void ShowUpdateMessageBox(const std::wstring& text, const std::wstring& title, const std::wstring& updateURL)
	{

		int response = MessageBoxW(
			NULL,
			text.c_str(),
			title.c_str(),
			MB_YESNO | MB_ICONQUESTION | MB_TOPMOST
		);

		if (response == IDYES && !updateURL.empty())
		{
			ShellExecuteW(
				NULL,          // Parent window handle
				L"open",       // Operation verb
				updateURL.c_str(), // File/URL target path
				NULL,          // Parameters
				NULL,          // Default working directory
				SW_SHOWNORMAL  // Window display state flag
			);
		}
	}
};