#include "Config.h"
#include "utils/Document.h"
#include "widgets/Logger.h"
#include <fstream>
#include <iomanip>
#include "StringUtils.h"

namespace Config::DO_NOT_USE
{
json config;    /**< The configuration cache */
bool isConfigLoaded = false;
}

/**
 * @brief   Loads the configuration of the software from the Config.json file.
 *          The Config.json file must be in the same directory as the executable
 *          to be found by File::GetPathOfFile.
 * @param   None
 * @retval  None
 */
void Config::Load()
{
    Config::DO_NOT_USE::isConfigLoaded = true;
    std::string path = File::GetPathOfFile("Config.json");
    // Open the file or create it if it doesn't exists.
    std::fstream file(path, std::ios::out | std::ios::app);
    file.close();
    std::ifstream j(path);


    std::string fullFile;
    std::string line;

    // Read the entire file.
    while (std::getline(j, line))
    {
        fullFile += line;
    }

    // Try to parse what we've read from the file.
    try
    {
        DO_NOT_USE::config = json::parse(fullFile);
    }
    catch (json::parse_error)
    {
        // If the parser is unable to do its job, create an empty json object and save it.
        DO_NOT_USE::config = "{}"_json;
        Save();
    }

    // Close the file handler.
    j.close();
}

/**
 * @brief   Save the config object in the Config.json file.
 * @param   None
 * @retval  None
 */
void Config::Save()
{
    // Open the Config.json file in output mode.
    std::fstream file(File::GetPathOfFile("Config.json"), std::ios::out);

    // Make sure the file is open.
    if (file.is_open() == false)
    {
        // Unable to open file.
        Logging::System.Error("Unable to open file!");
    }

    // Dump the json object into a string.
    std::string j = DO_NOT_USE::config.dump(4, ' ', false, json::error_handler_t::replace);
    // Save the dumped object into the file, pretty-printed.
    file << std::setw(4) << DO_NOT_USE::config;

    // Close the file handler.
    file.close();
}

/**
 * @brief   Get a copy of the configuration.
 * @param   None
 * @retval  The configuration.
 */
json Config::GetConfig()
{
    return DO_NOT_USE::config;
}
