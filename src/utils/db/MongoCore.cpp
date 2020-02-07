#include "MongoCore.h"
#include "vendor/json/json.hpp"
#include "widgets/Logger.h"
#include <vector>

#define CLIENT client->GetClient()
#define CLIENT_IS_VALID ((client != nullptr) && hasError == false)
static mongocxx::instance instance;

/**
 * @class   Client
 * @brief   At first, this had more to it.
 *          But it is now only a pretty useless wrapper for mongocxx::client.
 *          But it works so I'm not changing it.
 */
class Client
{
public:
    /**
     * Creates a mongodb client from the passed parameters.
     * @param   host: The url to use for the connection.
     * @param   options: Any options to use for the client.
     */
    Client(const std::string& host, const mongocxx::options::client& options) :
        m_client(mongocxx::uri(host), options)
    {
    }
    ~Client()
    {
        m_client.~client();
    }

    /**
     * Get the mongocxx::client.
     */
    inline mongocxx::client& GetClient()
    {
        return m_client;
    }

private:
    mongocxx::client m_client;
};

// A pointer to an instance of Client because mongodb is triggered if you create an empty client.
static Client* client;
static bool isInit = false;
static bool hasError = false;

/**
 * @brief   Initialize the connection to the mongodb database.
 * @param   host: The url to use for the connection.
 * @param   options: Any options to use for the client.
 * @retval  True if the initialization was successful, false otherwise.
 *
 * @note    There's a lot of reasons why the initialization might fail.
 *          The most common one I've seen is an invalid `host` string.
 *          The function should, in theory, catch all of the failed connection,
 *          but I wouldn't 100% rely on it.
 */
bool DB::Init(const std::string& host, const mongocxx::options::client& options)
{
    try
    {
        // Instantiate a new Client.
        client = new Client(host, options);
    }
    catch (const mongocxx::logic_error & e)
    {
        // Something happened, tell the user.
        Logging::System.Critical("An Error Occurred When Logging In: ", e.what());
        hasError = true;
    }
    if (isInit == true)
    {
        return false;
    }

    isInit = true;

    return true;
}

/**
 * @brief   Get a document from the collection `col` in the database `db` that matches the filter.
 * @param   db: The database to get the collection from.
 * @param   col: The collection to query in.
 * @param   filter: The filter to query against.
 * @retval  The document if one is found, else an empty document.
 */
bsoncxx::document::view DB::GetDocument(const std::string& db,
                                        const std::string& col,
                                        const bsoncxx::document::value& filter)
{
    if (!CLIENT_IS_VALID)
    {
        return bsoncxx::document::value({});
    }

    try
    {
        // Query the database for a matching document.
        bsoncxx::stdx::optional<bsoncxx::document::value> result =
            CLIENT.database(db).collection(col).find_one(filter.view());
        // If a matching document is found: 
        // TLDR: result is actually never `{}`, even if nothing is found.
        // Instead, the database returns an empty document, so std::optional is useless here.
        if (result)
        {
            // Return the document.
            return result.value().view();
        }
        else
        {
            // Return an empty document.
            return bsoncxx::document::view({});
        }
    }
    catch (const mongocxx::query_exception & e)
    {
        Logging::System.Critical("An error occurred when getting document from collection \""
                                 + col + "\" of database \"" + db + "\"\n\t", e.what());
        hasError = true;
        return {};
    }
}

/**
 * @brief   Get all documents in a collection of the database.
 * @param   db: The database to get the collection from.
 * @param   col: The collection to query.
 * @param   filter: The optional filter to query against.
 * @retval  If the query returned something that isn't retarded, returns a mongocxx::cursor to iterate over.
 *          Otherwise, returns an empty std::optional.
 */
bsoncxx::stdx::optional<mongocxx::cursor> DB::GetAllDocuments(std::string db,
                                                              std::string col,
                                                              const bsoncxx::document::value& filter)
{
    if (CLIENT_IS_VALID)
    {
        // Query the database.
        mongocxx::cursor cursor = CLIENT.database(db).collection(col).find(filter.view());
        try
        {
            // For some fucked up reason, `mongocxx::client::database::collection::find` is one of the 
            // only functions in the entire library that doesn't use std::optional but that should use it.
            // If the query doesn't return anything, instead of giving out an error, it just keeps on going 'till
            // you try to iterate over it, then crashes because the software can't iterate over something that doesn't
            // exist.
            // So instead of letting the program crash at an unknown place, we forcefully attempt to make it do so
            // here by getting an iterator to the beginning of the cursor. If the cursor is empty, it will trigger
            // the catch block.
            auto v = cursor.begin();
        }
        catch (const mongocxx::query_exception & e)
        {
            Logging::System.Critical("An error occurred when getting all documents from collection \""
                                     + col + "\" of database \"" + db + "\"\n\t", e.what());
            hasError = true;
            return {};
        }
        return cursor;
    }
    else
    {
        return {};
    }
}

/**
 * @brief   Insert a document into the desired collection of the desired database.
 * @param   doc: The document to insert in the collection.
 * @param   db: The database to get the collection from.
 * @param   col: The collection to insert the document into.
 * @retval  True if insertion is successful, false otherwise.
 *
 * @attention   `collection.insert_one` *should* return `{}` if it failed,
 *              but in practice the std::optional returned by that function is always
 *              valid, no matter what happens. Which means that DB::InsertDocument will
 *              never return false.
 *
 * @todo    Find a way to know if the insertion failed.
 */
bool DB::InsertDocument(const bsoncxx::document::value& doc, const std::string& db, const std::string& col)
{
    if (!CLIENT_IS_VALID)
    {
        return false;
    }
    auto collection = CLIENT[db][col];
    bsoncxx::stdx::optional<mongocxx::result::insert_one> result =
        collection.insert_one(doc.view());

    return result ? true : false;

}

/**
 * @brief   Update the first document that matches the filter using the document `doc`.
 * @param   filter: The filter to use to find the document(s) to update.
 * @param   doc: The document from which to get the values to use during the update.
 * @param   db: The database to do the action in.
 * @param   col: The database to do the action in.
 * @retval  True if at least one document matched the filter, false otherwise.
 */
bool DB::UpdateDocument(const bsoncxx::document::value& filter, const bsoncxx::document::value& doc,
                        const std::string& db, const std::string& col)
{
    if (!CLIENT_IS_VALID)
    {
        return false;
    }
    try
    {
        bsoncxx::stdx::optional<mongocxx::result::update> result =
            CLIENT.database(db).collection(col).update_one(filter.view(), doc.view());

        // If the std::optional returned by the database is valid:
        if (result)
        {
            // Return true if at least 1 document matched the filter.
            return (result.value().matched_count() > 0);
        }
        else
        {
            return false;
        }
    }
    catch (const mongocxx::bulk_write_exception & e)
    {
        Logging::System.Error("An error occurred when updating a document: ", e.what());
        return false;
    }
}

/**
 * @brief   Delete the first document that matches the filter in
 *          the collection `col` from the database `db`.
 * @param   filter: The filter to use to find the document to delete.
 * @param   db: The database to do the action in.
 * @param   col: The collection to do the action in.
 * @retval  True if at least 1 document was deleted, false otherwise.
 */
bool DB::DeleteDocument(const bsoncxx::document::value& filter, const std::string& db, const std::string& col)
{
    if (!CLIENT_IS_VALID)
    {
        return false;
    }
    bsoncxx::stdx::optional<mongocxx::result::delete_result> r =
        CLIENT.database(db).collection(col).delete_one(filter.view());

    // If the std::optional returned by the database is valid:
    if (r)
    {
        // Return true if at least 1 document was deleted.
        return (r.value().deleted_count() > 0);
    }
    else
    {
        return false;
    }
}

/**
 * @brief   Check if the current client has write privileges in the database.
 *          This is accomplished by attempting to add a temporary document
 *          in the database then deleting it. If the client has the rights to do that,
 *          the operation will succeed, otherwise, an exception will be thrown.
 * @param   db: The database to verify the client's privileges in.
 * @return  True if the Client has write privileges, false otherwise.
 */
bool DB::HasUserWritePrivileges(const std::string& db)
{
    if (!CLIENT_IS_VALID)
    {
        return false;
    }

    using namespace bsoncxx::builder::stream;
    try
    {
        // Make a dummy document.
        auto builder = document{};
        bsoncxx::document::value doc = builder
            << "field1" << "Value1"
            << finalize;
        // Try to add it to the database then deleting it.
        InsertDocument(doc, DATABASE, "privilegesVerification");
        DeleteDocument(doc, DATABASE, "privilegesVerification");
        return true;
    }
    catch (std::exception)
    {
        // An exception is thrown if the user doesn't have the required permissions.
        return false;
    }
}

/**
 * @brief   Authenticate into the database using credentials different than the default ones.
 * @param   username: The username of the new client.
 * @param   pwd: The password of the user.
 * @param   authDb: The database to use for the login, defaults to "admin".
 * @retval  True if credentials are valid, false otherwise.
 */
bool DB::Login(const std::string& username, const std::string& pwd, const std::string& authDb)
{
    isInit = false;

    // Re-initialize the Client using the new credentials. We don't check the return value
    // because the connection is *always* successful (unless invalid parameters are passed),
    // EVEN IF the credentials are not valid.
    // We must thus use another way to verify the success of the operation.
    Init("mongodb://" + username + ":" + pwd + "@192.168.0.152");
    try
    {
        // To verify if the login was good or not, we send a command to list all the 
        // collections inside of the DATABASE database. Since the mongodb server requires 
        // that all clients be authenticated to do anything, the command will fail
        // with a response of { "ok" : 0 }, even though the listCollections command
        // only requires read permissions.
        bsoncxx::builder::stream::document ping;
        ping << "listCollections" << 1;
        auto db = CLIENT[DATABASE];
        auto result = db.run_command(ping.view());

        if (result.view()["ok"].get_double() != 1)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    catch (const mongocxx::exception & e)
    {
        Logging::System.Error("An error occurred on login: ", e.what());
        return false;
    }
}
