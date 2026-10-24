#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cwctype>
#include <unordered_set>

class StringUtils 
{
public:
    static std::vector<std::string> SplitByChar(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::stringstream ss(str); // Convert string to a stream
        std::string token;

        // Read tokens separated by the delimiter
        while (std::getline(ss, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    static std::vector<std::wstring> SplitByCharW(const std::wstring& str, wchar_t delimiter) {
        std::vector<std::wstring> tokens;
        std::wstringstream ss(str); // Convert string to a stream
        std::wstring token;

        // Read tokens separated by the delimiter
        while (std::getline(ss, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    static bool CheckEndsWith(const std::wstring& str, const std::wstring& suffix, bool ignoreCase = true) {
        const size_t strLen = str.length();
        const size_t sufLen = suffix.length();

        if (strLen < sufLen) return false;

        const wchar_t* strPtr = str.data() + (strLen - sufLen);
        const wchar_t* sufPtr = suffix.data();

        if (ignoreCase) {
            for (size_t i = 0; i < sufLen; ++i) {
                wchar_t a = strPtr[i];
                wchar_t b = sufPtr[i];

                // Standard ASCII fast-lowercase conversion
                if (a >= L'A' && a <= L'Z') a += 32;
                if (b >= L'A' && b <= L'Z') b += 32;

                if (a != b) return false;
            }
            return true;
        }

        return wmemcmp(strPtr, sufPtr, sufLen) == 0;
    }

    static bool ValidateLuaVariable(const std::wstring& input)
    {
        if (input.empty()) 
        {
            return false;
        }

        static const std::unordered_set<std::wstring> keywords = {
            L"and", L"break", L"do", L"else", L"elseif", L"end",
            L"false", L"for", L"function", L"goto", L"if", L"in",
            L"local", L"nil", L"not", L"or", L"repeat", L"return",
            L"then", L"true", L"until", L"while"
        };

        if (keywords.count(input)) 
        {
            return false;
        }

        wchar_t first = input[0];
        if (!std::iswalpha(first) && first != L'_') 
        {
            return false;
        }

        for (size_t i = 1; i < input.length(); ++i) 
        {
            wchar_t c = input[i];
            if (!std::iswalnum(c) && c != L'_') 
            {
                return false;
            }
        }

        return true;
    }

    static std::wstring SanitizeLuaString(const std::wstring& input) 
    {
        std::wostringstream ss;

        for (wchar_t c : input) 
        {
            switch (c) 
            {
            case L'\\': ss << L"\\\\"; break;
            case L'"':  ss << L"\\\""; break;
            case L'\n': ss << L"\\n";  break;
            case L'\r': ss << L"\\r";  break;
            case L'\t': ss << L"\\t";  break;
            default:
                // Escape control characters below ASCII 32 to prevent corruption
                if (c < 32) 
                {
                    ss << L"\\" << (c < 10 ? L"00" : L"0") << std::to_wstring(c);
                }
                else 
                {
                    ss << c;
                }
                break;
            }
        }

        return ss.str();
    }

    static std::wstring Trim(const std::wstring& str) {
        const std::wstring whitespace = L" \t\n\v\f\r";

        const auto start = str.find_first_not_of(whitespace);
        if (start == std::wstring::npos) {
            return L""; // Camera or string is all whitespace
        }

        const auto end = str.find_last_not_of(whitespace);
        const auto range = end - start + 1;

        return str.substr(start, range);
    }
};