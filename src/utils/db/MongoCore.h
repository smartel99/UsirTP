/**
 ******************************************************************************
 * @addtogroup MongoCore
 * @{
 * @file    MongoCore
 * @author  Samuel Martel
 * @brief   Header for the MongoCore module.
 *
 * @date 1/16/2020 12:09:50 PM
 *
 *
 * @attention   The c++ library for mongodb and its mandatory c drivers are
 * pretty... meh. I got it working, but it took a lot of guessing around.
 * A word of warning: The documentation for mongocxx, bsoncxx and the C drivers
 * are out of date, so I strongly advise against modifying this file or its
 * .cpp counterpart (MongoCore.cpp)
 *
 ******************************************************************************
 */
#ifndef _MongoCore
#define _MongoCore

 /*****************************************************************************/
 /* Includes */

#include <iostream>
#include "utils/db/Mongo.h"

/**
 * @namespace DB MongoCore.h MongoCore
 * @brief   The namespace for everything related to the database.
 */
namespace DB
{
/*****************************************************************************/
/* Exported defines */
#ifdef USE_DEBUG_DB
#define DATABASE "CEP_DEBUG"
#else
#define DATABASE "CEP"
#endif

/*****************************************************************************/
/* Exported macro */


/*****************************************************************************/
/* Exported types */


/*****************************************************************************/
/* Exported functions */

bool Init(const std::string& host = "mongodb://localhost:27017",
          const mongocxx::options::client& options = mongocxx::options::client());

bsoncxx::document::view GetDocument(const std::string& db = "",
                                    const std::string& col = "",
                                    const bsoncxx::document::value& filter = bsoncxx::document::value({}));
bsoncxx::stdx::optional<mongocxx::cursor>  GetAllDocuments(std::string db = "",
                                                           std::string col = "",
                                                           const bsoncxx::document::value& filter =
                                                           bsoncxx::document::value({}));
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

bool HasUserWritePrivileges(const std::string& db = "admin");
bool Login(const std::string& username, const std::string& pwd, const std::string& authDb = "admin");
}
/* Have a wonderful day :) */
#endif /* _MongoCore */
/**
 * @}
 */
 /****** END OF FILE ******/
