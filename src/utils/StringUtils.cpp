#include "StringUtils.h"
#include "widgets/Logger.h"
#include <Windows.h>
#include <time.h>
#include <stdio.h>
#include <sstream>
#include <algorithm>


std::string StringUtils::ReplaceAll(const std::string& str,
                                    const std::string& toReplace,
                                    const std::string& replaceBy)
{
    std::string newStr = str;

    std::string::size_type n = 0;
    while ((n = newStr.find(toReplace, n)) != std::string::npos)
    {
        newStr.replace(n, toReplace.size(), replaceBy);
        n += replaceBy.size();
    }

    return newStr;
}

std::string StringUtils::GetFullNameFromPath(const std::wstring& path)
{
    std::string strTo = LongStringToString(path);
    size_t startOfName = strTo.find_last_of("/\\");
    strTo = strTo.substr(startOfName + 1);
    return strTo;
}

std::string StringUtils::GetFullNameFromPath(const std::string& path)
{
    std::string strTo = path;
    size_t startOfName = strTo.find_last_of("/\\");
    strTo = strTo.substr(startOfName + 1);
    return strTo;
}

std::string StringUtils::GetNameFromPath(const std::string& path)
{
    std::string strTo = GetFullNameFromPath(path);
    size_t startOfExt = strTo.find_last_of(".");
    return strTo.substr(0, startOfExt);
}

std::string StringUtils::RemoveNameFromPath(const std::wstring& path)
{
    std::string strTo = LongStringToString(path);
    size_t startOfName = strTo.find_last_of("/\\");
    strTo = strTo.substr(0, strTo.size() - startOfName);
    return strTo;
}

std::string StringUtils::RemoveNameFromPath(const std::string& path)
{
    std::string strTo = path;
    size_t startOfName = strTo.find_last_of("/\\");
    strTo = strTo.substr(0, startOfName + 1);
    return strTo;
}

std::string StringUtils::LongStringToString(const std::wstring& src)
{
    std::string strTo;
    char* szTo = new char[src.length() + 1];
    szTo[src.size()] = '\0';
    WideCharToMultiByte(CP_ACP, 0, src.c_str(), -1, szTo, (int)src.length(), NULL, NULL);
    strTo = szTo;
    delete[] szTo;

    return strTo;
}

std::wstring StringUtils::StringToLongString(const std::string& src)
{
    return std::wstring(src.begin(), src.end());
}

std::string StringUtils::GetCurrentTimeFormated(void)
{
    char timeStamp[100] = { 0 };
    time_t raw_now = time(0);
    struct tm now;
    localtime_s(&now, &raw_now);

    strftime(timeStamp, sizeof(timeStamp), "[%x - %X]", &now);

    return std::string(timeStamp);
}

template<typename T>
std::string _NumToString(T val, bool zeroPadded)
{
    std::stringstream ss;

    // The number is converted to string with the help of stringstream.
    ss << val;
    std::string ret;
    ss >> ret;

    if (zeroPadded)
    {
        ret = ret.substr(0, 4); // Only keep 4 chars.
        // Append zero chars.
        int strLen = int(ret.length());
        for (int i = 0; i < 4 - strLen; i++)
        {
            ret = "0" + ret;
        }
    }

    return ret;
}

std::string StringUtils::NumToString(unsigned int val, bool zeroPadded)
{
    return _NumToString(val, zeroPadded);
}

std::string StringUtils::NumToString(int val, bool zeroPadded)
{
    return _NumToString(val, zeroPadded);
}

std::string StringUtils::NumToString(float val)
{
    std::stringstream ss;

    // The number is converted to string with the help of stringstream.
    ss << val;
    std::string ret;
    ss >> ret;

    return ret;
}

bool StringUtils::StringIsValidUrl(const std::string& str)
{
    if (str.find_first_of(":/.\\") != std::string::npos)
    {
        return true;
    }
    else
    {
        return false;
    }
}

template<> int StringUtils::StringToNum<int>(const std::string& val)
{
    std::string s = val;
    // Remove all prefacing characters.
    s.erase(0, MIN(s.find_first_of("123456789"), s.size() - 1));

    return s.empty() ? 0 : std::atoi(s.c_str());
}

template<> long StringUtils::StringToNum<long>(const std::string& val)
{
    std::string s = val;
    // Remove all prefacing characters.
    s.erase(0, MIN(s.find_first_of("123456789"), s.size() - 1));

    return s.empty() ? 0 : std::atol(s.c_str());
}

template<> unsigned long StringUtils::StringToNum<unsigned long>(const std::string& val)
{
    std::string s = val;
    // Remove all prefacing characters.
    s.erase(0, MIN(s.find_first_of("123456789"), s.size() - 1));

    char* end;
    return s.empty() ? 0 : std::strtoul(s.c_str(), &end, 10);
}

template<> float StringUtils::StringToNum<float>(const std::string& val)
{
    std::string s = val;
    // Remove all prefacing characters.
    s.erase(0, MIN(s.find_first_of("123456789"), s.size() - 1));

    return s.empty() ? 0.f : std::stof(s);
}
