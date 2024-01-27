
#include <cstdarg>
#include "stringutil.h"

string wstring2string(wstring &ws) {
    char chars[ws.size() * 2];
    sprintf(chars, "%ws", ws.c_str());
    auto str = string(chars);
    return str;
}

wstring string2wstring(string &s) {
    wchar_t wchars[s.size() * 2];
    swprintf(wchars, L"%s", s.c_str());
    wstring wstr(wchars);
    return wstr;
}


string str_fmt(const char *fmt, ...) {
    const int max = 100;
    char chars[max];
    va_list args;
    va_start(args, fmt);
    vsnprintf(chars, max, fmt, args);
    va_end(args);
    return chars;
}
