#include "StringUtils.h"
#include "widgets/Logger.h"
#include <Windows.h>
#include <time.h>
#include <stdio.h>


std::string StringUtils::ReplaceAll(const std::string& str,
                                    const std::string& toReplace,
                                    const std::string& replaceBy)
{
    std::string newStr = str;

    std::string::size_type n = 0;
    while ( (n = newStr.find(toReplace, n)) != std::string::npos )
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
