#include "MongoCore.h"
#include "vendor/json/json.hpp"
#include "widgets/Logger.h"
#include <vector>

#define CLIENT client->GetClient()
static mongocxx::instance instance;

class Client
{
public:
    Client(const std::string& host, const mongocxx::options::client& options) :
        m_client(mongocxx::uri(host), options)
    {
    }
    ~Client()
    {
        m_client.~client();
    }

    inline mongocxx::client& GetClient(void)
    {
        return m_client;
    }

private:
    mongocxx::client m_client;
};

static Client* client;
static bool isInit = false;

bool DB::Init(const std::string& host, const mongocxx::options::client& options)
{
//     client.~Client();
    Logging::System.Debug("Using host: ", host);
    client = new Client(host, options);
    if (isInit == true)
    {
        return false;
    }

    isInit = true;

    return true;
}

std::string DB::GetCurrentClientHostName(void)
{
    return CLIENT.uri().to_string();
}

bsoncxx::document::view DB::GetDocument(const std::string& db,
                                        const std::string& col,
                                        const bsoncxx::document::value& filter)
{
    bsoncxx::stdx::optional<bsoncxx::document::value> result =
        CLIENT.database(db).collection(col).find_one(filter.view());
    if (result)     // If the document was found:
    {
        // It won't be nothing, like literally nothing.
        return result.value().view();
    }
    else
    {
        return bsoncxx::document::view({});
    }
}

mongocxx::cursor DB::GetAllDocuments(std::string db,
                                     std::string col,
                                     const bsoncxx::document::value& filter)
{
    mongocxx::cursor cursor = CLIENT.database(db).collection(col).find(filter.view());
    return cursor;
}

bool DB::InsertDocument(const bsoncxx::document::value& doc, const std::string& db, const std::string& col)
{
    auto collection = CLIENT[db][col];
    bsoncxx::stdx::optional<mongocxx::result::insert_one> result =
        collection.insert_one(doc.view());

    return result ? true : false;

}

bool DB::UpdateDocument(const bsoncxx::document::value& filter, const bsoncxx::document::value& doc,
                        const std::string& db, const std::string& col)
{
    bsoncxx::stdx::optional<mongocxx::result::update> result =
        CLIENT.database(db).collection(col).update_one(filter.view(), doc.view());

    if (result)
    {
        return (result.value().matched_count() > 0);
    }
    else
    {
        return false;
    }
}

bool DB::DeleteDocument(const bsoncxx::document::value& filter, const std::string& db, const std::string& col)
{
    bsoncxx::stdx::optional<mongocxx::result::delete_result> r =
        CLIENT.database(db).collection(col).delete_one(filter.view());

    if (r)
    {
        return (r.value().deleted_count() > 0);
    }
    else
    {
        return false;
    }
}

bool DB::IsUserAdmin(const std::string& db)
{
    using namespace bsoncxx::builder::basic;
    try
    {
        auto serverStatus = CLIENT.database(db).run_command(make_document(kvp("serverStatus", 1)));

        // if we are admin, the "ok" field of the reply will be one, otherwise it will be 0.
        if (serverStatus.view()["ok"].get_double() != double(1))
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    catch (const std::exception & e)
    {
        // An exception is thrown if the user doesn't have the required permissions.
        return false;
    }
}

bool DB::Login(const std::string& username, const std::string& pwd, const std::string& authDb)
{
    isInit = false;
    return Init("mongodb://" + username + ":" + pwd + "@192.168.0.152");
}
