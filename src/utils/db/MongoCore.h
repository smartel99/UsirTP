/**
 ******************************************************************************
 * @addtogroup MongoCore
 * @{
 * @file    MongoCore
 * @author  Client Microdata
 * @brief   Header for the MongoCore module.
 *
 * @date 1/16/2020 12:09:50 PM
 *
 ******************************************************************************
 */
#ifndef _MongoCore
#define _MongoCore

 /*****************************************************************************/
 /* Includes */

#include <iostream>
#include "utils/db/Mongo.h"

namespace DB
{
/*****************************************************************************/
/* Exported defines */


/*****************************************************************************/
/* Exported macro */


/*****************************************************************************/
/* Exported types */


/*****************************************************************************/
/* Exported functions */

    bool Init(const std::string& host = "mongodb://localhost:27017",
              const mongocxx::options::client& options = mongocxx::options::client());

    std::string GetCurrentClientHostName(void);

    bsoncxx::document::view GetDocument(const std::string& db = "",
                                        const std::string& col = "",
                                        const bsoncxx::document::value& filter = bsoncxx::document::value({}));
    mongocxx::cursor GetAllDocuments(std::string db = "",
                                     std::string col = "",
                                     const bsoncxx::document::value& filter = bsoncxx::document::value({}));
    bool InsertDocument(const bsoncxx::document::value& doc,
                        const std::string& db = "",
                        const std::string& col = "");
    bool UpdateDocument(const bsoncxx::document::value& filter,
                        const bsoncxx::document::value& doc,
                        const std::string& db = "",
                        const std::string& col = "");
    bool DeleteDocument(const bsoncxx::document::value& filter = bsoncxx::document::value({}),
                        const std::string& db = "",
                        const std::string& col = "");
}
/* Have a wonderful day :) */
#endif /* _MongoCore */
/**
 * @}
 */
 /****** END OF FILE ******/
