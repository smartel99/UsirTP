#include "Bom.h"
#include "vendor/imgui/imgui.h"
#include "utils/StringUtils.h"
#include "boost/algorithm/string.hpp"
#include "widgets/Logger.h"

using namespace DB::BOM;

static bsoncxx::document::value CreateDocument(BOM bom);
static bsoncxx::document::value CreateDocument(const std::string& field, const std::string& val);
static bsoncxx::document::value CreateDocumentForUpdate(BOM bom);
static BOM CreateObject(const bsoncxx::document::view& doc);
static ItemReference CreateItemReference(const bsoncxx::document::view& doc);
static bool RemoveFromCache(const BOM& bom);
static std::string FindDiffs(const BOM& a, const BOM& b);

static std::vector<BOM> boms;
static bool isInit = false;
static bool hasError = false;


const DB::Item::Item DB::BOM::BOM::GetOutput() const
{
    return DB::Item::GetItemByID(m_output.GetId());
}

const std::vector<DB::Item::Item> DB::BOM::BOM::GetItems() const
{
    std::vector<DB::Item::Item> items;

    // For every items in the BOM:
    for (auto& i : m_items)
    {
        // Fetch the DB::Item::Item instance and adds it to the list.
        items.emplace_back(DB::Item::GetItemByID(i.GetId()));
    }

    return items;
}

/**
 * @brief   Initializes the Bill of Material module.
 *              - Clears the cache
 *              - Load all the BOMs in the database into the cache.
 *          If the initialization fails (connection issue, authentication error, etc.),
 *          it always returns false and the cache will be empty.
 * @param   None
 * @retval  True if init is successful.
 */
bool DB::BOM::Init()
{
    if (hasError)
    {
        return false;
    }

    // Clear the cache.
    boms.clear();

    // Get all the BOMs from the database.
    bsoncxx::stdx::optional<mongocxx::cursor> bs = DB::GetAllDocuments("CEP", "BOMs");

    // `bs` will be `{}` if the query failed.
    if (!bs)
    {
        isInit = false;
        return false;
    }

    // If `if(!bs)` passes (`bs` isn't `{}`) yet doesn't contain any value, `bs.value()` will 
    // throw `mongocxx::query_exception`.
    try
    {
        // For each document returned by the database:
        for (auto b : bs.value())
        {
            // Create a BOM instance and add it to the cache.
            boms.emplace_back(CreateObject(b));
        }

        isInit = true;
        return true;
    }
    catch (const mongocxx::query_exception & e)
    {
        Logging::System.Critical("An error occurred when initializing BOMs: ", e.what());
        hasError = true;
        return false;
    }
}

/**
 * @brief   Refreshes the cache once every 10 seconds
 * @param   None
 * @retval  None
 */
void DB::BOM::Refresh()
{
    static double elapsedTime = 0;
    static int frameCount = 0;
    // deltaTime is the time between two frames (e.g. deltaTime @ 60fps is ~16.667ms).
    const static float deltaTime = ImGui::GetIO().DeltaTime;

    // If we're at a new frame:
    if (frameCount != ImGui::GetFrameCount())
    {
        // Add the deltaTime to elapsed time.
        frameCount = ImGui::GetFrameCount();
        elapsedTime += deltaTime;
        // If it has been more than 10 seconds since the last refresh:
        if (elapsedTime >= 10.f)
        {
            // Do the refresh. This is done by just re-initializing the module.
            elapsedTime = 0;
            Init();
        }
    }
}

/**
 * @brief   Adds a new BOM to the database and the cache.
 * @param   bom The BOM to insert into the database.
 * @retval  True if insertion was successful, false otherwise.
 *
 * @note    BOM will be added to the cache even if insertion into database
 *          fails. It will however be removed at the next Refresh event.
 */
bool DB::BOM::AddBom(const BOM& bom /**< [in] The BOM to insert into the database */)
{
    if (isInit == false)
    {
        return false;
    }

    // Add the BOM to the cache.
    boms.emplace_back(bom);

    // Create a mongodb document from the BOM.
    bsoncxx::document::value doc = CreateDocument(bom);

    // Log the event.
    Logging::Audit.Info("Created BOM \"" + bom.GetId(), "\"", true);

    // Insert the BOM in the database.
    return DB::InsertDocument(doc, "CEP", "BOMs");
}

/**
 * @brief   Update a currently existing BOM in the cache and the database.
 * @param   oldBom: The BOM to edit.
 * @param   newBom: The new value of the BOM.
 * @retval  True if successfully edited in the database, false otherwise.
 *
 * @note    Even if the modification in the database fails, the BOM will be modified in the cache.
 *          It will remain like that until the next refresh event.
 * @note    The oldBom in the cache isn't technically modified, but rather deleted. The newBom is then
 *          added to the cache.
 */
bool DB::BOM::EditBom(const BOM& oldBom     /**< [in] The BOM object to edit  */
                      , const BOM& newBom   /**< [in] The new BOM object */)
{
    if (isInit == false)
    {
        return false;
    }

    // Remove the old bom from the cache.
    RemoveFromCache(oldBom);
    // Add the new bom to the cache.
    boms.emplace_back(newBom);
    // Log the event.
    Logging::Audit.Info("Edited BOM ", oldBom.GetId() + FindDiffs(oldBom, newBom), true);

    // Update the document in the database:
    //  - CreateDocument -> Create a mongodb document containing only the id of the old bom to use as a filter.
    //  - CreateDocumentForUpdate -> Create a mongodb document with the new bom.
    return DB::UpdateDocument(CreateDocument("id", oldBom.GetId()), CreateDocumentForUpdate(newBom), "CEP", "BOMs");
}

/**
 * @brief   Delete a BOM from the cache and the database.
 * @param   The BOM to delete. Only the `m_id` field of the bom object is used to query the database.
 * @retval  True if successfully deleted from the database, false otherwise.
 * @note    Even if the bom isn't deleted from the database, it will still be removed from the cache.
 *          This will remain until the next refresh event.
 */
bool DB::BOM::DeleteBom(const BOM& bom /**< [in] The BOM object to delete */)
{
    if (isInit == false)
    {
        return false;
    }

    // Remove the bom from the cache.
    RemoveFromCache(bom);
    // Log the event.
    Logging::Audit.Info("Deleted BOM \"" + bom.GetId(), "\"", true);

    // Delete the BOM from the database.
    return (DB::DeleteDocument(CreateDocument("id", bom.GetId()), "CEP", "BOMs"));
}

/**
 * @brief   Get the entire cache of BOMs as is.
 * @param   None
 * @retval  A const reference to the cache.
 */
const std::vector<BOM>& DB::BOM::GetAll()
{
    return boms;
}

/**
 * @brief   Form a new ID for a BOM item by adding 1 to the highest ID found in the cache.
 *          Example:
 *              Cache contains:
 *                  - BOM0001
 *                  - BOM0002
 *                  - BOM0006
 *              DB::BOM::GetNewId will return `BOM0007`
 * @param   None
 * @retval  A string representation of the new ID.
 */
std::string DB::BOM::GetNewId()
{
    int max = 0;
    // For each BOM in the cache:
    for (DB::BOM::BOM& b : boms)
    {
        // If this BOM's id is the biggest we've seen so far:
        if (max < StringUtils::StringToNum<int>(b.GetId()))
        {
            // Remember it.
            max = StringUtils::StringToNum<int>(b.GetId());
        }
    }

    // Form the new ID by incrementing the biggest seen ID by 1.
    std::string ret = "BOM" + StringUtils::NumToString(max + 1);
    return ret;
}

/**
 * @brief   Create a mongodb document from a BOM object.
 *          The created document is pretty much just a JSON dump of the object.
 * @param   bom: The BOM to create a document with.
 * @retval  The created document.
 */
bsoncxx::document::value CreateDocument(BOM bom /**< [in] The BOM object to create the document from */)
{
    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;

    // Create the document builder.
    bsoncxx::builder::basic::document builder = bsoncxx::builder::basic::document{};
    // Add the ID field.
    builder.append(kvp("id", bom.GetId()));
    // Add the name field.
    builder.append(kvp("name", bom.GetName()));

    // Create an array builder.
    auto array_builder = bsoncxx::builder::basic::array{};
    // For each items in the bom's item list:
    for (const auto& item : bom.GetRawItems())
    {
        // Add a document to the array containing all the fields of the item.
        array_builder.append(make_document(
            kvp("_id", bsoncxx::oid(item.GetObjId())),  // ObjectID: Reference to mongodb document in the DB.
            kvp("id", item.GetId()),                    // id:       CEP Id of the item.
            kvp("quantity", item.GetQuantity())));      // quantity: Quantity needed for that item.
    }

    // Add the array we just created.
    builder.append(kvp("items", array_builder));
    // Add the output object.
    builder.append(kvp("output", make_document(
        kvp("_id", bsoncxx::oid(bom.GetRawOutput().GetObjId())),
        kvp("id", bom.GetRawOutput().GetId()),
        kvp("quantity", bom.GetRawOutput().GetQuantity()))));

    // Return the bsoncxx::document::value created by the builder.
    return builder.extract();
}

/**
 * @brief   Create a mongoDB document from the passed parameters. You can use this function
 *          to create filters for a query.
 * @param   field: The name of the field
 * @param   val: The value to assign to that field.
 * @retval  The created document.
 */
bsoncxx::document::value CreateDocument(const std::string& field    /**< [in] The name of the field */
                                        , const std::string& val    /**< [in] The value to assign to that field */)
{
    // Create the document builder.
    auto builder = bsoncxx::builder::stream::document{};
    // Create the document from the builder and the field-value pair.
    bsoncxx::document::value doc = builder
        << field << val << bsoncxx::builder::stream::finalize;

    return doc;
}

/**
 * @brief   Create a mongoDB document to use in an update query from a BOM object.
 * @param   bom The BOM instance to use to create the document.
 * @retval  The created document.
 */
bsoncxx::document::value CreateDocumentForUpdate(BOM bom    /**< [in] The BOM object to use to create the document */)
{
    using bsoncxx::builder::basic::kvp;
    // Create the builder object.
    auto builder = bsoncxx::builder::basic::document{};
    // Create a mongodb document from the bom object and add it to the document.
    builder.append(kvp("$set", CreateDocument(bom)));

    // Make a `bsoncxx::document::value` from the builder.
    return builder.extract();
}

/**
 * @brief   Create a BOM instance from a mongodb document.
 * @param   doc The document to use.
 * @retval  A BOM instance corresponding to the document.
 */
BOM CreateObject(const bsoncxx::document::view& doc /**< [in] The document to create a BOM object from */)
{
    std::vector<ItemReference> items;
    std::string id = "N/A";
    std::string name = "N/A";
    ItemReference output;

    // Get the id field from the document.
    bsoncxx::document::element el = doc["id"];
    // If the field exists:
    if (el.raw() != nullptr)
    {
        // If the field's value is of the desired type:
        if (el.type() == bsoncxx::type::k_utf8)
        {
            // Get the value.
            id = el.get_utf8().value.data();
        }
    }

    // Get the name field from the document.
    el = doc["name"];
    // If the field exists:
    if (el.raw() != nullptr)
    {
        // If the field's value is of the desired type:
        if (el.type() == bsoncxx::type::k_utf8)
        {
            // Get the value.
            name = el.get_utf8().value.data();
        }
    }

    // And do the same for the rest of the members of the BOM class...

    el = doc["output"];
    if (el.raw() != nullptr)
    {
        if (el.type() == bsoncxx::type::k_document)
        {
            // Build an ItemReference object from the "output" field.
            output = CreateItemReference(el.get_document());
        }
    }

    el = doc["items"];
    if (el.raw() != nullptr)
    {
        if (el.type() == bsoncxx::type::k_array)
        {
            // Build an ItemReference object for each elements of the array.
            for (auto& i : el.get_array().value)
            {
                if (i.type() == bsoncxx::type::k_document)
                {
                    items.emplace_back(CreateItemReference(i.get_document()));
                }
            }
        }
    }

    // Instantiate BOM with the values we extracted from the document.
    return BOM(id, name, items, output);
}

/**
 * @brief   Create an ItemReference object from a mongodb document.
 * @param   doc: The document to use to create the object.
 * @retval  An ItemReference instance corresponding to the document.
 */
ItemReference CreateItemReference(const bsoncxx::document::view& doc)
{
    std::string id = "N/A";
    std::string oid = "N/A";
    float qty = 0.f;

    // Get the id field of the document.
    bsoncxx::document::element el = doc["id"];
    // If the field exists:
    if (el.raw() != nullptr)
    {
        // If the field's value is of the desired type:
        if (el.type() == bsoncxx::type::k_utf8)
        {
            // Get the value.
            id = el.get_utf8().value.data();
        }
    }

    // Get the _id field of the document (internal mongodb ObjectId).
    el = doc["_id"];
    // If the field exists:
    if (el.raw() != nullptr)
    {
        // If the field's value is of the required type:
        if (el.type() == bsoncxx::type::k_oid)
        {
            // Get the value.
            oid = el.get_oid().value.to_string();
        }
    }

    // Get the quantity field of the document.
    el = doc["quantity"];
    // If the field exists:
    if (el.raw() != nullptr)
    {
        // If the field's value is of the desired type:
        if (el.type() == bsoncxx::type::k_double)
        {
            // Get the value.
            qty = float(el.get_double().value);
        }
    }

    // Instantiate ItemReference with the values we extracted from the document.
    return ItemReference(id, oid, qty);

}

/**
 * @brief   Remove the BOM object that matches the BOM reference passed as parameter from the cache.
 * @param   bom: The BOM to remove from the cache.
 * @retval  True if an object was removed from the cache, false otherwise.
 */
bool RemoveFromCache(const BOM& bom)
{
    // For each BOM in the cache:
    for (auto i = boms.begin(); i != boms.end(); i++)
    {
        // If the BOM is identical to the one passed as parameter:
        if (*i == bom)
        {
            // Remove it from the cache.
            boms.erase(i);
            return true;
        }
    }
    // No matching BOM was found in the cache.
    return false;
}

/**
 * @brief   Create a string containing a list of the differences between two BOM objects.
 * @param   const BOM& from: The original BOM object to use.
 * @param   to: The possibly different BOM object to use.
 * @retval  A string of all the differences between the two objects. If they are identical,
 *          the string will be empty.
 */
std::string FindDiffs(const BOM& from, const BOM& to)
{
    std::string difs = "";

    // Compare the ids.
    if (from.GetId() != to.GetId())
    {
        difs += "\n\r\tID:\n\r\t\tFrom: " + from.GetId() + "\n\r\t\tTo: " + to.GetId();
    }
    // Compare the names.
    if (from.GetName() != to.GetName())
    {
        difs += "\n\r\tName:\n\r\t\tFrom: " + from.GetName() + "\n\r\t\tTo: " + to.GetName();
    }
    // Compare the output items.
    if (from.GetRawOutput() != to.GetRawOutput())
    {
        difs += "\n\r\tOutput Item:\n\r\t\tFrom: ";
        difs += "\n\r\t\t\tId: " + from.GetRawOutput().GetId();
        difs += "\n\r\t\t\tOId: " + from.GetRawOutput().GetObjId();
        difs += "\n\r\t\t\tQuantity: " + StringUtils::NumToString(from.GetRawOutput().GetQuantity());

        difs += "\n\r\t\tTo: ";
        difs += "\n\r\t\t\tId: " + from.GetRawOutput().GetId();
        difs += "\n\r\t\t\tOId: " + from.GetRawOutput().GetObjId();
        difs += "\n\r\t\t\tQuantity: " + StringUtils::NumToString(from.GetRawOutput().GetQuantity());
    }
    // Compare the lists of items.
    // We just check the difference in sizes to avoid bloating the output string and thus the audit log too much.
    // (And also because it would be a pain in the ass to do so)
    if (from.GetRawItems().size() != to.GetRawItems().size())
    {
        int dif = int(from.GetRawItems().size() - to.GetRawItems().size());
        difs += "\n\r\tItems: \n\r\t\t";
        difs += dif < 0 ?
            "Removed " + StringUtils::NumToString(-dif) + " items" :
            "Added " + StringUtils::NumToString(dif) + " items";
    }

    return difs;
}
