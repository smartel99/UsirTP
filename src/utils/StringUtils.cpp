/**
 * @file StringUtils.cpp
 *
 * @author Samuel Martel
 * @date February 2020
 *
 * @brief Source for the StringUtils module.
 *
 */
#include "StringUtils.h"
#include "widgets/Logger.h"
#include <Windows.h>
#include <time.h>
#include <stdio.h>
#include <sstream>
#include <algorithm>

/**
 * @brief   Find all instances of `toReplace` in `str` and replace them with `replaceBy`.
 * @param   str: The string to use
 * @param   toReplace: The string to search for in `str`
 * @param   replaceBy: The string to replace `toReplace` with.
 * @retval  The newly formed string
 */
std::string StringUtils::ReplaceAll(const std::string& str,
                                    const std::string& toReplace,
                                    const std::string& replaceBy)
{
    // Create a copy of the string to work with.
    std::string newStr = str;

    std::string::size_type n = 0;
    // While `toReplace` is in `str`:
    while ((n = newStr.find(toReplace, n)) != std::string::npos)
    {
        // Replace `toReplace` with `replaceBy`.
        newStr.replace(n, toReplace.size(), replaceBy);
        n += replaceBy.size();
    }

    return newStr;
}

/**
 * @brief   Extract the file name out of the provided path, including the extension.
 * @param   path: The path, absolute or relative, to get a filename out of.
 * @retval  The file name.
 *
 * @note    This not only works with files but also directories.
 *          For example:
 *              With `path` = "C:\path\to\my\dir",
 *              The function will return "dir"
 *          Example 2:
 *              With `path` = "C:\path\to\my\dir\myfile.exe",
 *              The function will return "myfile.exe"
 */
std::string StringUtils::GetFullNameFromPath(const std::wstring& path)
{
    // Convert the wide string into a normal string.
    std::string strTo = LongStringToString(path);
    // Find the start of the file name/directory. 
    // This is what's after the last '/' or '\'
    size_t startOfName = strTo.find_last_of("/\\");
    // Create a string only containing that last part.
    strTo = strTo.substr(startOfName + 1);
    return strTo;
}

/**
 * @brief   Extract the file name out of the provided path, including the extension.
 * @param   path: The path, absolute or relative, to get a filename out of.
 * @retval  The file name.
 *
 * @note    This not only works with files but also directories.
 *          For example:
 *              With `path` = "C:\path\to\my\dir",
 *              The function will return "dir"
 *          Example 2:
 *              With `path` = "C:\path\to\my\dir\myfile.exe",
 *              The function will return "myfile.exe"
 */
std::string StringUtils::GetFullNameFromPath(const std::string& path)
{
    // Create a copy of `path` to work with.
    std::string strTo = path;
    // Find the start of the file name/directory.
    // This is what's after the last '/' or '\'.
    size_t startOfName = strTo.find_last_of("/\\");
    // Create a string that only contains that last part we found.
    strTo = strTo.substr(startOfName + 1);
    return strTo;
}

/**
 * @brief   Extract the file name out of the provided path, without the extension.
 * @param   path: The path, absolute or relative, to get a filename out of.
 * @retval  The file name.
 *
 * @note    This not only works with files but also directories.
 *          For example:
 *              With `path` = "C:\path\to\my\dir",
 *              The function will return "dir"
 *          Example 2:
 *              With `path` = "C:\path\to\my\dir\myfile.exe",
 *              The function will return "myfile"
 */
std::string StringUtils::GetNameFromPath(const std::string& path)
{
    // Get the full file name out of the `path`.
    std::string strTo = GetFullNameFromPath(path);
    // Only keep the part that is before the last '.' in the filename.
    size_t startOfExt = strTo.find_last_of(".");
    return strTo.substr(0, startOfExt);
}

/**
 * @brief   Take the provided path and remove the file name from it.
 * @param   path: The path to remove the filename from.
 * @retval  The path without the file name
 *
 * @example With `path` = "C:\path\to\my\dir\myfile.exe"
 *          The function will return "C:\path\to\my\dir\"
 */
std::string StringUtils::RemoveNameFromPath(const std::wstring& path)
{
    // Convert the wide string into a normal string.
    std::string strTo = LongStringToString(path);
    // Find the file name.
    // This is the part after the last '/' or '\'
    size_t startOfName = strTo.find_last_of("/\\");
    // Make a string out of everything up to the start of the filename.
    strTo = strTo.substr(0, strTo.size() - startOfName);
    return strTo;
}

/**
 * @brief   Take the provided path and remove the file name from it.
 * @param   path: The path to remove the filename from.
 * @retval  The path without the file name
 *
 * @example With `path` = "C:\path\to\my\dir\myfile.exe"
 *          The function will return "C:\path\to\my\dir\"
 */
std::string StringUtils::RemoveNameFromPath(const std::string& path)
{
    // Create a copy of `path` to work with.
    std::string strTo = path;
    // Find the file name.
    // This is the part after the last '/' or '\'
    size_t startOfName = strTo.find_last_of("/\\");
    // Make a string out of everything up to the start of the filename.
    strTo = strTo.substr(0, startOfName + 1);
    return strTo;
}

/**
 * @brief   Convert a wide string (16-bits/char) into a regular 8-bits/char string.
 * @param   src: The wide string to convert
 * @retval  The regular string.
 */
std::string StringUtils::LongStringToString(const std::wstring& src)
{
    std::string strTo;
    // Create a buffer the size of `src` + 1.
    // (std::wstring::length() doesn't count '\0')
    char* szTo = new char[src.length() + 1];
    // Add that '\0' that std::wstring doesn't have.
    szTo[src.size()] = '\0';
    // Convert the wide characters into regular 8bit characters.
    WideCharToMultiByte(CP_ACP, 0, src.c_str(), -1, szTo, (int)src.length(), NULL, NULL);
    strTo = szTo;
    // Free the memory.
    delete[] szTo;

    return strTo;
}

/**
 * @brief   Convert a regular 8-bits/char string into a wide (16-bits/char) string.
 * @param   src: The string to convert
 * @retval  The wide string.
 */
std::wstring StringUtils::StringToLongString(const std::string& src)
{
    // Construct a std::wstring from `src`.
    return std::wstring(src.begin(), src.end());
}

/**
 * @brief   Return the current local time under the form of a string.
 *          The string is formated like this:
 *              "[%Y-%m-%d - %H:%M:%S]"
 * @param   None
 * @retval  A formated string of the current local time.
 */
std::string StringUtils::GetCurrentTimeFormated(void)
{
    char timeStamp[100] = { 0 };
    // Get the current time in POSIX format.
    time_t raw_now = time(0);
    struct tm now;
    // Convert POSIX time into something more friendly.
    localtime_s(&now, &raw_now);

    // Format the time into a string.
    strftime(timeStamp, sizeof(timeStamp), "[%F - %X]", &now);

    return std::string(timeStamp);
}

/**
 * @brief   Takes a value of type `T` and convert it into a string.
 *          Optionally zero pad the value to 4 characters wide.
 * @param   val: The value to convert
 * @param   zeroPadded: Zero pad the value or not.
 *          This also truncates the value to 4 characters wide.
 * @retval  The formated string
 *
 * @note    The way it is used, this template is redundant.
 *
 * #remove_this
 */
template<typename T>
std::string _NumToString(T val, bool zeroPadded)
{
    std::stringstream ss;

    // The number is converted to string with the help of stringstream.
    ss << val;
    std::string ret;
    // Dump the stream into a string.
    ss >> ret;

    // If the string should be zero padded:
    if (zeroPadded)
    {
        // Only keep 4 chars.
        ret = ret.substr(0, 4);
        // Append zero chars.
        int strLen = int(ret.length());
        for (int i = 0; i < 4 - strLen; i++)
        {
            ret = "0" + ret;
        }
    }

    return ret;
}

/**
 * @brief   Convert an unsigned integer into a string, with optional zero padding.
 * @param   val: The value to convert
 * @param   zeroPadded: Zero pad the value or not.
 *          This also truncates the value to 4 characters wide.
 * @retval  The value under the form of a string.
 */
std::string StringUtils::NumToString(unsigned int val, bool zeroPadded)
{
    return _NumToString(val, zeroPadded);
}

/**
 * @brief   Convert an integer into a string, with optional zero padding.
 * @param   val: The value to convert
 * @param   zeroPadded: Zero pad the value or not.
 *          This also truncates the value to 4 characters wide.
 * @retval  The value under the form of a string.
 */
std::string StringUtils::NumToString(int val, bool zeroPadded)
{
    return _NumToString(val, zeroPadded);
}

/**
 * @brief   Convert a float into a string, with optional zero padding.
 * @param   val: The value to convert
 * @param   zeroPadded: Zero pad the value or not.
 *          This also truncates the value to 4 characters wide.
 * @retval  The value under the form of a string.
 */
std::string StringUtils::NumToString(float val)
{
    std::stringstream ss;

    // The number is converted to string with the help of stringstream.
    ss << val;
    std::string ret;
    // Dump the stream into a string.
    ss >> ret;

    return ret;
}

/**
 * @brief   Lazily check if the passed string is a valid URL or path
 * @param   str: The string to check
 * @retval  `true` if the string is a valid URL or path, `false` otherwise.
 *
 * @note    This function doesn't actually verify if the string is either
 *          a valid URL or a valid path, but rather only check if the string
 *          contains any of the following characters: ':', '/', '.' and/or '\'.
 *          It is thus up to the caller to effectuate proper verification.
 */
bool StringUtils::StringIsValidUrl(const std::string& str)
{
    // If we can find one of those characters:
    if (str.find_first_of(":/.\\") != std::string::npos)
    {
        // Assume that the string is a valid URL/path.
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * @brief   Convert a string into a signed integer.
 * @param   val: The string to convert into an integer
 * @retval  The integer.
 *
 * @note    Even though this function has a return type of `int`,
 *          it will not return any negative value.
 *          Also, due to how `std::atoi` behaves, all prefacing
 *          alphabetical characters, white spaces, '-' and zeros
 *          will be stripped out before doing the conversion.
 */
template<> int StringUtils::StringToNum<int>(const std::string& val)
{
    std::string s = val;
    // Remove all prefacing characters.
    s.erase(0, MIN(s.find_first_of("123456789"), s.size() - 1));

    // If the string is not empty, convert it into an integer.
    return s.empty() ? 0 : std::atoi(s.c_str());
}

/**
 * @brief   Convert a string into a signed long integer.
 * @param   val: The string to convert into an integer
 * @retval  The long integer.
 *
 * @note    Even though this function has a return type of `long`,
 *          it will not return any negative value.
 *          Also, due to how `std::atol` behaves, all prefacing
 *          alphabetical characters, white spaces, '-' and zeros
 *          will be stripped out before doing the conversion.
 */
template<> long StringUtils::StringToNum<long>(const std::string& val)
{
    std::string s = val;
    // Remove all prefacing characters.
    s.erase(0, MIN(s.find_first_of("123456789"), s.size() - 1));

    return s.empty() ? 0 : std::atol(s.c_str());
}

/**
 * @brief   Convert a string into an unsigned long integer.
 * @param   val: The string to convert into an integer
 * @retval  The unsigned long integer.
 *
 * @note    Also, due to how `std::atol` behaves, all prefacing
 *          alphabetical characters, white spaces, '-' and zeros
 *          will be stripped out before doing the conversion.
 */
template<> unsigned long StringUtils::StringToNum<unsigned long>(const std::string& val)
{
    std::string s = val;
    // Remove all prefacing characters.
    s.erase(0, MIN(s.find_first_of("123456789"), s.size() - 1));

    char* end;
    return s.empty() ? 0 : std::strtoul(s.c_str(), &end, 10);
}

/**
 * @brief   Convert a string into a floating point number.
 * @param   val: The string to convert into an float
 * @retval  The float.
 */
template<> float StringUtils::StringToNum<float>(const std::string& val)
{
    std::string s = val;
    // Remove all prefacing characters.
    s.erase(0, MIN(s.find_first_of("123456789"), s.size() - 1));

    return s.empty() ? 0.f : std::stof(s);
}
