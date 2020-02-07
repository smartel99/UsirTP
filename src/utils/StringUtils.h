/**
 ******************************************************************************
 * @addtogroup StringUtils
 * @{
 * @file    StringUtils
 * @author  Samuel Martel
 * @brief   Header for the StringUtils module.
 *
 * @date 1/3/2020 12:09:56 PM
 *
 ******************************************************************************
 */
#ifndef _StringUtils
#define _StringUtils

/*****************************************************************************/
/* Includes */
#include <iostream>
#include "utils/misc.h"

namespace StringUtils
{
/*****************************************************************************/
/* Exported defines */


/*****************************************************************************/
/* Exported macro */


/*****************************************************************************/
/* Exported types */


/*****************************************************************************/
/* Exported functions */
std::string  ReplaceAll(const std::string& str,
                        const std::string& toReplace,
                        const std::string& replaceBy);
std::string GetFullNameFromPath(const std::wstring& path);
std::string GetFullNameFromPath(const std::string& path);
std::string GetNameFromPath(const std::string& path);
std::string RemoveNameFromPath(const std::wstring& path);
std::string RemoveNameFromPath(const std::string& path);
std::string LongStringToString(const std::wstring& src);
std::wstring StringToLongString(const std::string& src);

std::string GetCurrentTimeFormated();

bool StringIsValidUrl(const std::string& str);


std::string NumToString(unsigned int val, bool zeroPadded = true);
std::string NumToString(int val, bool zeroPadded = true);
std::string NumToString(float val);

template<class T>
T StringToNum(const std::string& val);

}
/* Have a wonderful day :) */
#endif /* _StringUtils */
/**
 * @}
 */
/****** END OF FILE ******/
