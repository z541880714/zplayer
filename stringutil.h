//
// Created by lionel on 2024/1/26.
//

#ifndef Z_STRING_UTIL_H
#define Z_STRING_UTIL_H

#include <string>

using namespace std;

string str_fmt(const char *fmt ...);

string wstring2string(wstring &ws);

wstring string2wstring(string &s);


#endif  //Z_STRING_UTIL_H
