/**
 ******************************************************************************
 * @addtogroup Config
 * @{
 * @file    Config
 * @author  Client Microdata
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
    namespace DO_NOT_USE
    {
        extern json config;
        extern bool isConfigLoaded;
    }

/*****************************************************************************/
/* Exported functions */
    void Load(void);
    void Save(void);

    json GetConfig(void);

    template<class T>
    void SetField(const std::string& key, T val)
    {
        if ( DO_NOT_USE::isConfigLoaded == false )
        {
            Load();
        }
        DO_NOT_USE::config[key] = val;
        Save();
    }

    template<class T>
    T GetField(const std::string& key)
    {
        if ( DO_NOT_USE::isConfigLoaded == false )
        {
            Load();
        }
        try
        {
            return DO_NOT_USE::config[key];
        }
        catch ( json::exception )
        {
            // Requested item doesn't exist.
            throw std::invalid_argument("Field Not Found!");
        }
    }
} // Namespace Config.
/* Have a wonderful day :) */
#endif /* _Config */
/**
 * @}
 */
/****** END OF FILE ******/
