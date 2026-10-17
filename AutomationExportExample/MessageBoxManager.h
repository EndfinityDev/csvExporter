#pragma once
#include <string>
#include <Shlobj.h>

class MessageBoxManager
{
public:
	static inline void ShowMessageBox(std::wstring text, std::wstring title)
	{
		MessageBox(NULL, text.c_str(), title.c_str(), 0);
	}
};