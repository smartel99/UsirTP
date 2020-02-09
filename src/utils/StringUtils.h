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

/**
 * @namespace    StringUtils
 * @brief        The namespace containing all the utility fonctions for strings.
 */
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

/**
 * @brief   Convert a string into a value of type `T`.
 * @param   val: The string to convert.
 * @retval  The value extracted from the string
 *
 * @note    Currently, only `int`, `unsigned int`, `long`,
 *          `unsigned long` and `float` are valid types for `T`.
 *          Any other types will result in the code not compiling.
 */
template<class T>
T StringToNum(const std::string& val);

}
/* Have a wonderful day :) */
#endif /* _StringUtils */
/**
 * @}
 */
/****** END OF FILE ******/
