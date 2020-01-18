#include "Config.h"
#include "utils/Document.h"
#include "widgets/Logger.h"
#include <fstream>
#include <iomanip>
#include "StringUtils.h"

namespace Config::DO_NOT_USE
{
    json config;
    bool isConfigLoaded = false;
}


void Config::Load(void)
{
    Config::DO_NOT_USE::isConfigLoaded = true;
    std::string path = File::GetPathOfFile("Config.json");
    // Open the file or create it if it doesn't exists.
    std::fstream file(path, std::ios::out | std::ios::app);
    file.close();
    std::ifstream j(path);


    std::string fullFile;
    std::string line;

    while ( std::getline(j, line) )
    {
        fullFile += line;
    }

    try
    {
        DO_NOT_USE::config = json::parse(fullFile);
    }
    catch ( json::parse_error )
    {
        DO_NOT_USE::config = "{}"_json;
        Save();
    }

    j.close();
}

void Config::Save(void)
{
    std::fstream file(File::GetPathOfFile("Config.json"), std::ios::out);

    if ( file.is_open() == false )
    {
        Logging::System.Error("Unable to open file!");
    }

    std::string j = DO_NOT_USE::config.dump(4, ' ', false, json::error_handler_t::replace);
    file << std::setw(4) << DO_NOT_USE::config;

    file.close();
}

json Config::GetConfig(void)
{
    return DO_NOT_USE::config;
}

