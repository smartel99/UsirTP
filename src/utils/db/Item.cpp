#include "Item.h"
#include "boost/algorithm/string.hpp"
#include "utils/StringUtils.h"
#include "vendor/imgui/imgui.h"
#include "widgets/Logger.h"
#include <vector>
#include <stdexcept>

#define IS_INIT     (isInit==true)

namespace DB
{
namespace Item
{

static bsoncxx::document::value CreateDocument(Item it);
static bsoncxx::document::value CreateDocument(const std::string& field, const std::string& val);
static bsoncxx::document::value CreateDocumentForUpdate(Item it);
static Item CreateObject(const bsoncxx::document::view& doc);
static bool FindInCache(Item& it, const std::string& filter);
static bool FindInCache(Item& it);
static bool RemoveFromCache(const Item& it);
static std::string FindDiffs(const Item& from, const Item& to);

static std::vector<Item> items; /**< Cache */
static bool isInit = false;
static bool hasError = false;

/**
 * @brief   Initialize the Item module:
 *              - Clear the cache
 *              - Load all the Items from the database into the cache.
 *          If the initialization fails (connection issue, authentication error, etc.),
 *          it always returns false and the cache will be empty.
 * @param   None
 * @retval  True if successful, false otherwise.
 */
bool DB::Item::Init()
{
    if (hasError)
    {
        return false;
    }

    // Clear the cache.
    items.clear();
    // Get all the items from the database.
    bsoncxx::stdx::optional<mongocxx::cursor> its = DB::GetAllDocuments("CEP", "Items");
    // `its` will be `{}` if the query failed.
    if (!its)
    {
        isInit = false;
        return false;
    }

    // If `if(!its)` passes (`its` isn't `{}`) yet doesn't contain any value, `its.value()` will 
    // throw `mongocxx::query_exception`.
    try
    {
        // For each document returned by the database:
        for (auto it : its.value())
        {
            // Extract an Item object from that document and add it in the cache.
            items.emplace_back(CreateObject(it));
        }

        isInit = true;
        return true;
    }
    catch (const mongocxx::query_exception & e)
    {
        Logging::System.Critical("An error occurred when initializing Items: ", e.what());
        hasError = true;
        return false;
    }
}

/**
 * @brief   Refreshes the cache once every 10 seconds
 * @param   None
 * @retval  None
 */
void DB::Item::Refresh()
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
 * @brief   Adds a new Item to the database and the cache.
 * @param   it The Item to insert into the database.
 * @retval  True if insertion was successful, false otherwise.
 *
 * @note    Item will be added to the cache even if insertion into database
 *          fails. It will however be removed at the next Refresh event.
 */
bool DB::Item::AddItem(const Item& it)
{
    if (!IS_INIT)
    {
        return false;
    }

    // Add the new Item to the cache.
    items.emplace_back(it);

    // Create a mongodb document from the Item.
    bsoncxx::document::value itDoc = CreateDocument(it);
    // Insert the new Item in the database.
    bool r = DB::InsertDocument(itDoc, "CEP", "Items");
    // Log the event.
    Logging::Audit.Info("Created Item \"" + it.GetId(), "\"", true);
    // Refresh Cache.
    Init();

    return r;
}

/**
 * @brief   Created if needed but not implemented.
 *          Intended to be identical in principle to DB::Category::GetCategoryByName.
 *
 * @throws  std::logic_error
 */
Item DB::Item::GetItemByName(const std::string& name)
{
    if (!IS_INIT)
    {
        return DB::Item::Item();
    }
    throw std::logic_error("Not Implemented");
}

/**
 * @brief   Get an item from the database that matches the id.
 * @param   id: The id to query for.
 * @retval  The Item that was found or an empty object if none were found.
 */
Item DB::Item::GetItemByID(const std::string& id)
{
    if (!IS_INIT)
    {
        return DB::Item::Item();
    }
    // Create an Item object from the result of the query.
    return (CreateObject(DB::GetDocument("CEP", "Items", CreateDocument("id", id))));
}

/**
 * @brief   Generates a new Item id for the appropriate category.
 *          Optionally, force the id to be a certain value by using the id parameter.
 *          Example: `cat.GetPrefix()` is "MFOM" and id is -1
 *              Cache contains:
 *                  - MFOM0001
 *                  - MBOX0003
 *                  - AFOM0002
 *                  - MFOM0002
 *              DB::Item::GetNewId will return "MFOM0003"
 * @param   cat: The category to create the id for.
 * @param   id (optional): If specified, used as the id's number.
 *                         If not specified, automatically increment the id.
 * @retval  The generated id.
 */
std::string DB::Item::GetNewId(const DB::Category::Category& cat, int id)
{
    if (!IS_INIT)
    {
        return "";
    }

    int max = 0;
    // For each items in the cache:
    for (auto& item : items)
    {
        // If the item's category is the same we want:
        if (item.GetCategory() == cat)
        {
            // If this item's id is the biggest we've seen so far:
            if (StringUtils::StringToNum<int>(item.GetId()) >= max)
            {
                // Save it.
                max = StringUtils::StringToNum<int>(item.GetId());
            }
        }
    }

    // First ID will be 0001, not 0, this is desired.
    // If `id` is -1, use the maximal id we observed.
    // Otherwise, use the specified `id`
    max = id == -1 ? max + 1 : id;
    // Form the id using the category's prefix and the value of `max`
    std::string ret = cat.GetPrefix() + StringUtils::NumToString(max);
    // Does the Category have a suffix?
    if (cat.GetSuffix() != '\0')
    {
        // If yes, add it to the id.
        ret += cat.GetSuffix();
    }
    // Force the generated id to be all capitalized.
    boost::to_upper(ret);
    return  ret;
}

/**
 * @brief   Update a currently existing Item in the cache and the database.
 * @param   oldItem: The Item to edit.
 * @param   newItem: The new value of the Item.
 * @retval  True if successfully edited in the database, false otherwise.
 *
 * @note    Even if the modification in the database fails, the Item will be modified in the cache.
 *          It will remain like that until the next refresh event.
 * @note    The oldBom in the cache isn't technically modified, but rather deleted. The newBom is then
 *          added to the cache.
 */
bool DB::Item::EditItem(const Item& oldItem, const Item& newItem)
{
    if (!IS_INIT)
    {
        return false;
    }

    // Remove the old Item from the cache.
    RemoveFromCache(oldItem);
    // Add the "new" Item to the cache.
    items.emplace_back(newItem);
    // Update the Item in the database.
    bool r = (DB::UpdateDocument(CreateDocument("id", oldItem.GetId()),
                                 CreateDocumentForUpdate(newItem), "CEP", "Items"));
    // Log the event.
    Logging::Audit.Info("Edited Item ", oldItem.GetId() + FindDiffs(oldItem, newItem), true);

    // Refresh Cache.
    Init();

    return r;
}

/**
 * @brief   Delete an Item from the cache and the database.
 * @param   The Item to delete. Only the `m_id` field of the Item object is used to query the database.
 * @retval  True if successfully deleted from the database, false otherwise.
 * @note    Even if the Item isn't deleted from the database, it will still be removed from the cache.
 *          This will remain until the next refresh event.
 */
bool DB::Item::DeleteItem(Item& item)
{
    if (!IS_INIT)
    {
        return false;
    }

    // Remove the Item from the cache.
    RemoveFromCache(item);
    // Delete the Item from the database.
    bool r = (DB::DeleteDocument(CreateDocument("id", item.GetId()), "CEP", "Items"));
    // Log the event.
    Logging::Audit.Info("Deleted Item \"" + item.GetId(), "\"", true);

    // Refresh Cache.
    Init();

    return r;
}

/**
 * @brief   Get all items contained in the Cache.
 * @param   None
 * @retval  A list of all the Items.
 */
const std::vector<Item>& DB::Item::GetAll()
{
    return items;
}

/**
 * @brief   Create a mongodb document out of the Item object.
 * @param   it: The Item to use.
 * @retval  The document created from the Item.
 */
bsoncxx::document::value CreateDocument(Item it)
{
    // kvp -> Key-Value Pair.
    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;
    // Create the document builder.
    bsoncxx::builder::basic::document builder = bsoncxx::builder::basic::document{};

    // Add the id kvp.
    builder.append(kvp("id", it.GetId()));
    // Add the description kvp.
    builder.append(kvp("description", it.GetDescription()));
    // Add the category sub-document.
    builder.append(kvp("category", make_document(
        kvp("name", it.GetCategory().GetName()),
        kvp("prefix", it.GetCategory().GetPrefix()),
        kvp("suffix", it.GetCategory().GetSuffix()))));
    // Add the referenceLink kvp.
    builder.append(kvp("referenceLink", it.GetReferenceLink()));
    // Add the location kvp.
    builder.append(kvp("location", it.GetLocation()));
    // Add the price kvp.
    builder.append(kvp("price", it.GetPrice()));
    // Add the quantity kvp.
    builder.append(kvp("quantity", it.GetQuantity()));
    // Add the unit kvp.
    builder.append(kvp("unit", it.GetUnit()));
    // Add the status kvp.
    builder.append(kvp("status", it.GetStatus()));

    // If the Item has a valid ObjectId:
    if (!it.GetOid().empty() && it.GetOid() != "N/A")
    {
        // Add the ObjectId kvp.
        builder.append(kvp("_id", bsoncxx::oid(it.GetOid())));
    }

    // Extract a bsoncxx::document::value from the builder.
    return builder.extract();
}

/**
 * @brief   Create a mongoDB document from the passed parameters. You can use this function
 *          to create filters for a query.
 * @param   field: The name of the field
 * @param   val: The value to assign to that field.
 * @retval  The created document.
 */
bsoncxx::document::value CreateDocument(const std::string& field, const std::string& val)
{
    // Create the document builder.
    auto builder = bsoncxx::builder::stream::document{};
    // Add the key-value pair to the document.
    bsoncxx::document::value doc = builder
        << field << val
        << bsoncxx::builder::stream::finalize;

    return doc;
}

/**
 * @brief   Create a mongoDB document to use in an update query from a Category object.
 * @param   cat The Category instance to use to create the document.
 * @retval  The created document.
 */
bsoncxx::document::value CreateDocumentForUpdate(Item it)
{
    using bsoncxx::builder::basic::kvp;
    // Create the builder object.
    auto builder = bsoncxx::builder::basic::document{};
    // Create a mongodb document from the Item object and add it to the document.
    builder.append(kvp("$set", CreateDocument(it)));

    // Make a `bsoncxx::document::value` from the builder.
    return builder.extract();
}

/**
 * @brief   Create an instance of Item using a mongodb document.
 * @param   doc: The document to use.
 * @retval  The Item extracted from the document.
 */
Item CreateObject(const bsoncxx::document::view& doc)
{
    // Default values for Item:
    std::string oid = "N/A";
    std::string id = "N/A";
    std::string description = "N/A";
    std::string catName = "N/A";
    std::string catPref = "N/A";
    char catSuffix = '\0';
    std::string refLink = "N/A";
    std::string location = "N/A";
    float price = 0.00f;
    float quantity = 0.f;
    std::string unit = "N/A";
    ItemStatus status = ItemStatus::active;
    // End of default values.

    /** -------------------------------------------------------------------------------------------------
     * All of these sections bellow are identical in principle, so only the first one will be commented.
     * ------------------------------------------------------------------------------------------------*/

    /** BEGINNING OF A BLOCK */
    // Get the "_id" field from the document.
    bsoncxx::document::element el = doc["_id"];
    // If the field exists:
    if (el.raw() != nullptr)
    {
        // If the field's value is of the desired type:
        if (el.type() == bsoncxx::type::k_oid)
        {
            // Get the value.
            oid = el.get_oid().value.to_string();
        }
    }
    /** END OF A BLOCK */

    el = doc["id"];
    if (el.raw() != nullptr)
    {
        if (el.type() == bsoncxx::type::k_utf8)
        {
            id = el.get_utf8().value.data();
        }
    }

    el = doc["description"];
    if (el.raw() != nullptr)
    {
        if (el.type() == bsoncxx::type::k_utf8)
        {
            description = el.get_utf8().value.data();
        }
    }

    el = doc["category"];
    if (el.raw() != nullptr)
    {
        bsoncxx::document::element subEl = el["name"];
        if (subEl.raw() != nullptr)
        {
            if (subEl.type() == bsoncxx::type::k_utf8)
            {
                catName = subEl.get_utf8().value.data();
            }
        }

        subEl = el["prefix"];
        if (subEl.raw() != nullptr)
        {
            if (subEl.type() == bsoncxx::type::k_utf8)
            {
                catPref = subEl.get_utf8().value.data();
            }
        }

        subEl = el["suffix"];
        if (subEl.raw() != nullptr)
        {
            if (subEl.type() == bsoncxx::type::k_int32)
            {
                catSuffix = subEl.get_int32().value;
            }
        }
    }

    el = doc["referenceLink"];
    if (el.raw() != nullptr)
    {
        if (el.type() == bsoncxx::type::k_utf8)
        {
            refLink = el.get_utf8().value.data();
        }
    }

    el = doc["location"];
    if (el.raw() != nullptr)
    {
        if (el.type() == bsoncxx::type::k_utf8)
        {
            location = el.get_utf8().value.data();
        }
    }

    el = doc["price"];
    if (el.raw() != nullptr)
    {
        if (el.type() == bsoncxx::type::k_double)
        {
            price = float(el.get_double().value);
        }
    }

    el = doc["quantity"];
    if (el.raw() != nullptr)
    {
        if (el.type() == bsoncxx::type::k_double)
        {
            quantity = float(el.get_double().value);
        }
    }

    el = doc["unit"];
    if (el.raw() != nullptr)
    {
        if (el.type() == bsoncxx::type::k_utf8)
        {
            unit = el.get_utf8().value.data();
        }
    }

    el = doc["status"];
    if (el.raw() != nullptr)
    {
        if (el.type() == bsoncxx::type::k_int32)
        {
            status = ItemStatus(el.get_int32().value);
        }
    }

    // Instantiate an Item with the values extracted from the document.
    return Item(oid, id, description, { catName, catPref, catSuffix }, refLink, location, price, quantity, unit, status);
}

/**
 * @brief   Search the cache to check if the Item is present in it.
 * @param   it: The Item to look for
 * @retval  True if the Item is found, false otherwise.
 */
bool FindInCache(Item& it)
{
    // For each Item in the cache:
    for (Item i : items)
    {
        // If the Item matches `it`:
        if (i == it)
        {
            // We found it!
            it = i;
            return true;
        }
    }
    // No matching Item was found in the cache.
    return false;
}

/**
 * @brief   Remove an Item from the cache.
 * @param   it: The Item to remove.
 * @retval  True if an Item was removed from the cache, false otherwise.
 */
bool RemoveFromCache(const Item& it)
{
    // For each Item in the cache:
    for (auto i = items.begin(); i != items.end(); i++)
    {
        // If the Item matches `it`:
        if (*i == it)
        {
            // Remove it from the cache.
            items.erase(i);
            return true;
        }
    }
    // No matching Item was found.
    return false;
}

/**
 * @brief   Create a string containing a list of the differences between two Item objects.
 * @param   from: The original Item object to use.
 * @param   to: The possibly different Item object to use.
 * @retval  A string of all the differences between the two objects. If they are identical,
 *          the string will be empty.
 */
std::string FindDiffs(const Item& from, const Item& to)
{
    std::string difs = "";

    if (from.GetId() != to.GetId())
    {
        difs += "\n\r\tID:\n\r\t\tFrom: " + from.GetId() + "\n\r\t\tTo: " + to.GetId();
    }
    if (from.GetDescription() != to.GetDescription())
    {
        difs += "\n\r\tDescription:\n\r\t\tFrom: " + from.GetDescription() + "\n\r\t\tTo: " + to.GetDescription();
    }
    if (from.GetCategory() != to.GetCategory())
    {
        difs += "\n\r\tCategory:\n\r\t\tFrom: " + from.GetCategory().GetName() +
            "\n\r\t\tTo: " + to.GetCategory().GetName();
    }
    if (from.GetReferenceLink() != to.GetReferenceLink())
    {
        difs += "\n\r\tReference Link:\n\r\t\tFrom: " + from.GetReferenceLink() + "\n\r\t\tTo: " + to.GetReferenceLink();
    }
    if (from.GetLocation() != to.GetLocation())
    {
        difs += "\n\r\tLocation:\n\r\t\tFrom: " + from.GetLocation() + "\n\r\t\tTo: " + to.GetLocation();
    }
    if (from.GetPrice() != to.GetPrice())
    {
        difs += "\n\r\tPrice:\n\r\t\tFrom: " + StringUtils::NumToString(from.GetPrice()) +
            "\n\r\t\tTo: " + StringUtils::NumToString(to.GetPrice());
    }
    if (from.GetQuantity() != to.GetQuantity())
    {
        difs += "\n\r\tQuantity:\n\r\t\tFrom: " + StringUtils::NumToString(from.GetQuantity()) +
            "\n\r\t\tTo: " + StringUtils::NumToString(to.GetQuantity());
    }
    if (from.GetUnit() != to.GetUnit())
    {
        difs += "\n\r\tUnit:\n\r\t\tFrom: " + from.GetUnit() + "\n\r\t\tTo: " + to.GetUnit();
    }
    if (from.GetStatus() != from.GetStatus())
    {
        difs += "\n\r\tStatus:\n\r\t\tFrom: " + from.GetStatusAsString() + "\n\r\t\tTo: " + to.GetStatusAsString();
    }

    return difs;
}

}
}
