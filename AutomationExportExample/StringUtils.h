#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

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
};