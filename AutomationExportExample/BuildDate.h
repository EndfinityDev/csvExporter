#ifndef BUILD_DATE_H
#define BUILD_DATE_H

#include <cstddef>
#include <cstdint>

// Parse __DATE__ string (format: "Mmm dd yyyy")
// Returns year as int
constexpr int parse_year(const char* date_str) {
    return (date_str[7] - '0') * 1000 +
        (date_str[8] - '0') * 100 +
        (date_str[9] - '0') * 10 +
        (date_str[10] - '0');
}

// Returns month as int (1-12)
constexpr int parse_month(const char* date_str) {
    switch (date_str[0]) {
    case 'J': if (date_str[1] == 'a') return 1;  // Jan
        if (date_str[1] == 'u' && date_str[2] == 'n') return 6; // Jun
        if (date_str[1] == 'u' && date_str[2] == 'l') return 7; // Jul
        return 0;
    case 'F': return 2; // Feb
    case 'M': if (date_str[2] == 'r') return 3;  // Mar
        if (date_str[2] == 'y') return 5;  // May
        return 0;
    case 'A': if (date_str[1] == 'p') return 4;  // Apr
        if (date_str[1] == 'u') return 8;  // Aug
        return 0;
    case 'S': return 9;  // Sep
    case 'O': return 10; // Oct
    case 'N': return 11; // Nov
    case 'D': return 12; // Dec
    default:  return 0;
    }
}

// Returns day as int (1-31)
constexpr int parse_day(const char* date_str) {
    // __DATE__ day is space-padded: " 1" or "15"
    if (date_str[4] == ' ') {
        return date_str[5] - '0';
    }
    else {
        return (date_str[4] - '0') * 10 + (date_str[5] - '0');
    }
}

// Combine into yyyymmdd integer
constexpr int get_build_date_yyyymmdd() {
    return parse_year(__DATE__) * 10000 +
        parse_month(__DATE__) * 100 +
        parse_day(__DATE__);
}

// Compile-time constant for the date as an integer (e.g., 20260105)
constexpr int BUILD_DATE_INT = get_build_date_yyyymmdd();

// Helper function to convert the integer to a string at runtime
// (Since constexpr string literal construction from __DATE__ is complex)
inline const char* get_build_date_string() {
    static thread_local char buffer[9];
    // Format: yyyymmdd\0 (8 digits + null terminator)
    int yyyymmdd = BUILD_DATE_INT;
    int i = 8;
    buffer[8] = '\0';
    for (; i > 0; --i) {
        buffer[i - 1] = (yyyymmdd % 10) + '0';
        yyyymmdd /= 10;
    }
    return buffer;
}

#endif // BUILD_DATE_H