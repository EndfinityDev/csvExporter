#pragma once
#include <string>
#include <codecvt>

class StringConverter 
{
public:
	static inline std::wstring to_wstring(const std::string& utf8_str) {
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.from_bytes(utf8_str);
	}

	static inline std::string to_utf8string(const std::wstring& wstr) {
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.to_bytes(wstr);
	}
};