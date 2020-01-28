/**
 ******************************************************************************
 * @addtogroup Config
 * @{
 * @file    Config
 * @author  Samuel Martel
 * @brief   Header for the Config module.
 *
 * @date 1/4/2020 1:41:45 PM
 *
 ******************************************************************************
 */
#ifndef _Config
#define _Config

/*****************************************************************************/
/* Includes */
#include "vendor/json/json.hpp"
#include <iostream>
#include <stdexcept>

using json = nlohmann::json;
/**
 * @namespace Config Config.h Config
 * @brief The namespace for everything related to software configuration.
 */
namespace Config
{
/*****************************************************************************/
/* Exported defines */
#define FIELD_NOT_FOUND T(NULL)

/*****************************************************************************/
/* Exported macro */


/*****************************************************************************/
/* Exported types */


/*****************************************************************************/
/* Exported variables */
/**
 * @namespace DO_NOT_USE Config.h Config
 * @brief   As the name implies, please do not use this namespace or its content.
 */
namespace DO_NOT_USE
{
extern json config;
extern bool isConfigLoaded;
}

/*****************************************************************************/
/* Exported functions */
void Load();
void Save();

json GetConfig();

/**
 * @brief   Set the field `key` with the value `val`.
 * @param   key: The key of the field to set.
 * @param   val: The value to assign to that key.
 * @retval  None
 */
template<class T>
void SetField(const std::string& key, T val)
{
    // If the configurations has not been loaded yet:
    if (DO_NOT_USE::isConfigLoaded == false)
    {
        // Load it.
        Load();
    }
    // Set the key-value pair
    DO_NOT_USE::config[key] = val;
    // Save the config in the config.json file.
    Save();
}

/**
 * @brief   Get the value of the corresponding key, if it exists.
 * @param   key: The key of the field to get.
 * @retval  The value of the field. If the field doesn't exist, the default constructor
 *          of the requested type is returned instead.
 */
template<class T>
T GetField(const std::string& key)
{
    if (DO_NOT_USE::isConfigLoaded == false)
    {
        Load();
    }
    try
    {
        return DO_NOT_USE::config[key];
    }
    catch (json::exception)
    {
        // Requested item doesn't exist.
        return T();
    }
}
} // Namespace Config.
/* Have a wonderful day :) */
#endif /* _Config */
/**
 * @}
 */
/****** END OF FILE ******/
