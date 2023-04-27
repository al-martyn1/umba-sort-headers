#pragma once

#include "encoding/encoding.h"
#include <string>

inline
std::string toUtf8(const std::wstring &str)
{
    auto pApi = getEncodingsApi();
    return pApi->encode(str, EncodingsApi::cpid_UTF8);
}

inline
std::string toUtf8(wchar_t ch)
{
    auto wstr = std::wstring(1,ch);
    return toUtf8(wstr);
}

inline
std::wstring fromUtf8(const std::string &str)
{
    auto pApi = getEncodingsApi();
    return pApi->decode(str, EncodingsApi::cpid_UTF8);
}
