#include "MongoCore.h"
#include "vendor/json/json.hpp"
#include "widgets/Logger.h"
#include <vector>

#define CLIENT client->GetClient()
#define CLIENT_IS_VALID (client != nullptr)
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
    try
    {
        client = new Client(host, options);
    }
    catch (const mongocxx::logic_error & e)
    {
        Logging::System.Error("An Error Occurred When Logging In: ", e.what());
    }
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
    if (!CLIENT_IS_VALID)
    {
        return bsoncxx::document::value({});
    }

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

bsoncxx::stdx::optional<mongocxx::cursor> DB::GetAllDocuments(std::string db,
                                                              std::string col,
                                                              const bsoncxx::document::value& filter)
{
    if (CLIENT_IS_VALID)
    {
        mongocxx::cursor cursor = CLIENT.database(db).collection(col).find(filter.view());
        return cursor;
    }
    else
    {
        return {};
    }
}

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

bool DB::UpdateDocument(const bsoncxx::document::value& filter, const bsoncxx::document::value& doc,
                        const std::string& db, const std::string& col)
{
    if (!CLIENT_IS_VALID)
    {
        return false;
    }
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
    if (!CLIENT_IS_VALID)
    {
        return false;
    }
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

bool DB::HasUserWritePrivileges(const std::string& db)
{
    if (!CLIENT_IS_VALID)
    {
        return false;
    }

    using namespace bsoncxx::builder::stream;
    try
    {
        auto builder = document{};
        bsoncxx::document::value doc = builder
            << "field1" << "Value1"
            << finalize;
        InsertDocument(doc, "CEP", "privilegesVerification");
        DeleteDocument(doc, "CEP", "privilegesVerification");
    }
    catch (const std::exception & e)
    {
        // An exception is thrown if the user doesn't have the required permissions.
        return false;
    }
}

bool DB::Login(const std::string& username, const std::string& pwd, const std::string& authDb)
{
    if (!CLIENT_IS_VALID)
    {
        return false;
    }
    isInit = false;
    return Init("mongodb://" + username + ":" + pwd + "@192.168.0.152");
}
