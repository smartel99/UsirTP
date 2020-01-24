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

static std::vector<Item> items;
static bool isInit = false;
static bool hasError = false;


bool DB::Item::Init()
{
    if (hasError)
    {
        return false;
    }

    items.clear();
    bsoncxx::stdx::optional<mongocxx::cursor> its = DB::GetAllDocuments("CEP", "Items");
    if (!its)
    {
        isInit = false;
        return false;
    }

    try
    {
        for (auto it : its.value())
        {
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

void DB::Item::Refresh()
{
    static double elapsedTime = 0;
    static int frameCount = 0;
    const static float deltaTime = ImGui::GetIO().DeltaTime;

    if (frameCount != ImGui::GetFrameCount())
    {
        frameCount = ImGui::GetFrameCount();
        elapsedTime += deltaTime;
        if (elapsedTime >= 10.0f)
        {
            elapsedTime = 0;
            Init();
        }
    }
}

bool DB::Item::AddItem(const Item& it)
{
    if (!IS_INIT)
    {
        return false;
    }

    items.emplace_back(it);

    bsoncxx::document::value itDoc = CreateDocument(it);
    bool r = DB::InsertDocument(itDoc, "CEP", "Items");

    // Refresh Cache.
    Init();

    return r;
}

Item DB::Item::GetItemByName(const std::string& name)
{
    if (!IS_INIT)
    {
        return DB::Item::Item();
    }
    throw std::logic_error("Not Implemented");
}

Item DB::Item::GetItemByID(const std::string& prefix)
{
    if (!IS_INIT)
    {
        return DB::Item::Item();
    }
    return (CreateObject(DB::GetDocument("CEP", "Items", CreateDocument("id", prefix))));
}

std::string DB::Item::GetNewId(DB::Category::Category cat, int id)
{
    if (!IS_INIT)
    {
        return "";
    }

    int max = -1;
    for (auto& item : items)
    {
        if (item.GetCategory() == cat)
        {
            if (StringUtils::StringToNum<int>(item.GetId()) >= max)
            {
                max = StringUtils::StringToNum<int>(item.GetId());
            }
        }
    }

    max = id == -1 ? max + 1 : id;
    std::string ret = cat.GetPrefix() + StringUtils::IntToString(max);
    if (cat.GetSuffix() != '\0')
    {
        ret += cat.GetSuffix();
    }
    boost::to_upper(ret);
    return  ret;
}

bool DB::Item::EditItem(Item& oldItem, const Item& newItem)
{
    if (!IS_INIT)
    {
        return false;
    }

    items.emplace_back(newItem);
    bool r = (DB::UpdateDocument(CreateDocument("id", oldItem.GetId()), CreateDocumentForUpdate(newItem), "CEP", "Items"));
    RemoveFromCache(oldItem);

    // Refresh Cache.
    Init();

    return r;
}

bool DB::Item::DeleteItem(Item& item)
{
    if (!IS_INIT)
    {
        return false;
    }

    RemoveFromCache(item);
    bool r = (DB::DeleteDocument(CreateDocument("id", item.GetId()), "CEP", "Items"));

    // Refresh Cache.
    Init();

    return r;
}

const std::vector<Item>& DB::Item::GetAll()
{
    return items;
}

bsoncxx::document::value CreateDocument(Item it)
{
    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;
    bsoncxx::builder::basic::document builder = bsoncxx::builder::basic::document{};

    builder.append(kvp("id", it.GetId()));
    builder.append(kvp("description", it.GetDescription()));
    builder.append(kvp("category", make_document(
        kvp("name", it.GetCategory().GetName()),
        kvp("prefix", it.GetCategory().GetPrefix()),
        kvp("suffix", it.GetCategory().GetSuffix()))));
    builder.append(kvp("referenceLink", it.GetReferenceLink()));
    builder.append(kvp("location", it.GetLocation()));
    builder.append(kvp("price", it.GetPrice()));
    builder.append(kvp("quantity", it.GetQuantity()));
    builder.append(kvp("unit", it.GetUnit()));
    builder.append(kvp("status", it.GetStatus()));

    if (!it.GetOid().empty() && it.GetOid() != "N/A")
    {
        builder.append(kvp("_id", bsoncxx::oid(it.GetOid())));
    }

    return builder.extract();
}

bsoncxx::document::value CreateDocument(const std::string& field, const std::string& val)
{
    auto builder = bsoncxx::builder::stream::document{};
    bsoncxx::document::value doc = builder
        << field << val << bsoncxx::builder::stream::finalize;

    return doc;
}

bsoncxx::document::value CreateDocumentForUpdate(Item it)
{
    auto builder = bsoncxx::builder::stream::document{};
    bsoncxx::document::value doc = builder
        << "$set" << bsoncxx::builder::stream::open_document
        << "id" << it.GetId()
        << "description" << it.GetDescription()
        << "category.name" << it.GetCategory().GetName()
        << "category.prefix" << it.GetCategory().GetPrefix()
        << "category.suffix" << it.GetCategory().GetSuffix()
        << "referenceLink" << it.GetReferenceLink()
        << "location" << it.GetLocation()
        << "price" << it.GetPrice()
        << "quantity" << it.GetQuantity()
        << "unit" << it.GetUnit()
        << "status" << it.GetStatus()
        << bsoncxx::builder::stream::close_document
        << bsoncxx::builder::stream::finalize;

    return doc;
}

Item CreateObject(const bsoncxx::document::view& doc)
{
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

    bsoncxx::document::element el = doc["_id"];
    if (el.raw() != nullptr)
    {
        if (el.type() == bsoncxx::type::k_oid)
        {
            oid = el.get_oid().value.to_string();
        }
    }

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
            price = el.get_double().value;
        }
    }

    el = doc["quantity"];
    if (el.raw() != nullptr)
    {
        if (el.type() == bsoncxx::type::k_double)
        {
            quantity = el.get_double().value;
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

    return Item(oid, id, description, { catName, catPref, catSuffix }, refLink, location, price, quantity, unit, status);
}

bool FindInCache(Item& it, const std::string& filter)
{
    throw std::logic_error("Implement this");
}

bool FindInCache(Item& it)
{
    for (Item i : items)
    {
        if (i == it)
        {
            it = i;
            return true;
        }
    }
    return false;
}

bool RemoveFromCache(const Item& it)
{
    for (auto i = items.begin(); i != items.end(); i++)
    {
        if (*i == it)
        {
            items.erase(i);
            return true;
        }
    }
    return false;
}

}
}
