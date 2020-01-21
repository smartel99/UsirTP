#include "Item.h"
#include "vendor/imgui/imgui.h"
#include <vector>
#include <stdexcept>

#define CHECK_IS_INIT(...) if(isInit==false){isInit=true;Init();}

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


bool DB::Item::Init(void)
{
    items.clear();
    for (auto it : DB::GetAllDocuments("CEP", "Items"))
    {
        items.emplace_back(CreateObject(it));
    }
    return true;
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
    CHECK_IS_INIT();

    items.emplace_back(it);

    bsoncxx::document::value itDoc = CreateDocument(it);

    return DB::InsertDocument(itDoc, "CEP", "Items");
}

Item DB::Item::GetItemByName(const std::string& name)
{
    throw std::logic_error("Not Implemented");
}

Item DB::Item::GetItemByID(const std::string& prefix)
{
    throw std::logic_error("Not Implemented");
}

int GetNewId()
{
    int max = 0;
    for (auto& item : items)
    {
        if (std::stoi(item.GetId()) >= max)
        {
            max = std::stoi(item.GetId());
        }
    }

    return max + 1;
}

bool DB::Item::EditItem(const Item& oldItem, const Item& newItem)
{
    RemoveFromCache(oldItem);
    items.emplace_back(newItem);

    return (DB::UpdateDocument(CreateDocument(oldItem), CreateDocumentForUpdate(newItem), "CEP", "Items"));
}

bool DB::Item::DeleteItem(const Item& item)
{
    RemoveFromCache(item);

    return (DB::DeleteDocument(CreateDocument(item), "CEP", "Items"));
}

const std::vector<Item>& DB::Item::GetAll()
{
    return items;
}

bsoncxx::document::value CreateDocument(Item it)
{
    auto builder = bsoncxx::builder::stream::document{};
    bsoncxx::document::value doc = builder
        << "id" << it.GetId()
        << "description" << it.GetDescription()
        << "category" << bsoncxx::builder::stream::open_document
        << "name" << it.GetCategory().GetName()
        << "prefix" << it.GetCategory().GetPrefix()
        << bsoncxx::builder::stream::close_document
        << "referenceLink" << it.GetReferenceLink()
        << "price" << float(it.GetPrice())
        << "quantity" << it.GetQuantity()
        << "unit" << it.GetUnit()
        << "status" << it.GetStatus()
        << bsoncxx::builder::stream::finalize;

    return doc;
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
        << "category" << bsoncxx::builder::stream::open_document
        << "name" << it.GetCategory().GetName()
        << "prefix" << it.GetCategory().GetPrefix()
        << bsoncxx::builder::stream::close_document
        << "referenceLink" << it.GetReferenceLink()
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
    std::string id = "N/A";
    std::string description = "N/A";
    std::string catName = "N/A";
    std::string catPref = "N/A";
    std::string refLink = "N/A";
    float price = 0.00f;
    float quantity = 0.f;
    std::string unit = "N/A";
    ItemStatus status = ItemStatus::active;

    bsoncxx::document::element el = doc["id"];
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
    }

    el = doc["referenceLink"];
    if (el.raw() != nullptr)
    {
        if (el.type() == bsoncxx::type::k_utf8)
        {
            refLink = el.get_utf8().value.data();
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

    return Item(id, description, { catName, catPref }, refLink, price, quantity, unit, status);
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
