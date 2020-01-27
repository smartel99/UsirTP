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

    for (auto& i : m_items)
    {
        items.emplace_back(DB::Item::GetItemByID(i.GetId()));
    }

    return items;
}

bool DB::BOM::Init()
{
    if (hasError)
    {
        return false;
    }

    boms.clear();
    bsoncxx::stdx::optional<mongocxx::cursor> bs = DB::GetAllDocuments("CEP", "BOMs");
    if (!bs)
    {
        isInit = false;
        return false;
    }

    try
    {
        for (auto b : bs.value())
        {
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

void DB::BOM::Refresh()
{
    static double elapsedTime = 0;
    static int frameCount = 0;
    const static float deltaTime = ImGui::GetIO().DeltaTime;

    if (frameCount != ImGui::GetFrameCount())
    {
        frameCount = ImGui::GetFrameCount();
        elapsedTime += deltaTime;
        if (elapsedTime >= 10.f)
        {
            elapsedTime = 0;
            Init();
        }
    }
}

bool DB::BOM::AddBom(const BOM& bom)
{
    if (isInit == false)
    {
        return false;
    }

    boms.emplace_back(bom);
    bsoncxx::document::value doc = CreateDocument(bom);
    Logging::Audit.Info("Created BOM \"" + bom.GetId(), "\"", true);

    return DB::InsertDocument(doc, "CEP", "BOMs");
}

bool DB::BOM::EditBom(const BOM& oldBom, const BOM& newBom)
{
    if (isInit == false)
    {
        return false;
    }

    boms.emplace_back(newBom);
    RemoveFromCache(oldBom);
    Logging::Audit.Info("Edited BOM ", oldBom.GetId() + FindDiffs(oldBom, newBom), true);

    return DB::UpdateDocument(CreateDocument("id", oldBom.GetId()), CreateDocumentForUpdate(newBom), "CEP", "BOMs");
}

bool DB::BOM::DeleteBom(const BOM& bom)
{
    if (isInit == false)
    {
        return false;
    }

    RemoveFromCache(bom);
    Logging::Audit.Info("Deleted BOM \"" + bom.GetId(), "\"", true);

    return (DB::DeleteDocument(CreateDocument("id", bom.GetId()), "CEP", "BOMs"));
}

const std::vector<BOM>& DB::BOM::GetAll()
{
    return boms;
}


std::string DB::BOM::GetNewId()
{
    int max = 0;
    for (DB::BOM::BOM& b : boms)
    {
        if (max < StringUtils::StringToNum<int>(b.GetId()))
        {
            max = StringUtils::StringToNum<int>(b.GetId());
        }
    }

    std::string ret = "BOM" + StringUtils::NumToString(max + 1);
    boost::to_upper(ret);
    return ret;
}

bsoncxx::document::value CreateDocument(BOM bom)
{
    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;
    bsoncxx::builder::basic::document builder = bsoncxx::builder::basic::document{};
    builder.append(kvp("id", bom.GetId()));
    builder.append(kvp("name", bom.GetName()));

    auto array_builder = bsoncxx::builder::basic::array{};
    for (const auto& item : bom.GetRawItems())
    {
        array_builder.append(make_document(
            kvp("_id", bsoncxx::oid(item.GetObjId())),
            kvp("id", item.GetId()),
            kvp("quantity", item.GetQuantity())));
    }

    builder.append(kvp("items", array_builder));
    builder.append(kvp("output", make_document(
        kvp("_id", bsoncxx::oid(bom.GetRawOutput().GetObjId())),
        kvp("id", bom.GetRawOutput().GetId()),
        kvp("quantity", bom.GetRawOutput().GetQuantity()))));

    return builder.extract();
}

bsoncxx::document::value CreateDocument(const std::string& field, const std::string& val)
{
    auto builder = bsoncxx::builder::stream::document{};
    bsoncxx::document::value doc = builder
        << field << val << bsoncxx::builder::stream::finalize;

    return doc;
}

bsoncxx::document::value CreateDocumentForUpdate(BOM bom)
{
    using bsoncxx::builder::basic::kvp;
    auto builder = bsoncxx::builder::basic::document{};
    builder.append(kvp("$set", CreateDocument(bom)));

    return builder.extract();
}

BOM CreateObject(const bsoncxx::document::view& doc)
{
    std::vector<ItemReference> items;
    std::string id = "N/A";
    std::string name = "N/A";
    ItemReference output;

    bsoncxx::document::element el = doc["id"];
    if (el.raw() != nullptr)
    {
        if (el.type() == bsoncxx::type::k_utf8)
        {
            id = el.get_utf8().value.data();
        }
    }

    el = doc["name"];
    if (el.raw() != nullptr)
    {
        if (el.type() == bsoncxx::type::k_utf8)
        {
            name = el.get_utf8().value.data();
        }
    }

    el = doc["output"];
    if (el.raw() != nullptr)
    {
        if (el.type() == bsoncxx::type::k_document)
        {
            output = CreateItemReference(el.get_document());
        }
    }

    el = doc["items"];
    if (el.raw() != nullptr)
    {
        if (el.type() == bsoncxx::type::k_array)
        {
            for (auto& i : el.get_array().value)
            {
                if (i.type() == bsoncxx::type::k_document)
                {
                    items.emplace_back(CreateItemReference(i.get_document()));
                }
            }
        }
    }

    return BOM(id, name, items, output);
}

ItemReference CreateItemReference(const bsoncxx::document::view& doc)
{
    std::string id = "N/A";
    std::string oid = "N/A";
    float qty = 0.f;

    bsoncxx::document::element el = doc["id"];
    if (el.raw() != nullptr)
    {
        if (el.type() == bsoncxx::type::k_utf8)
        {
            id = el.get_utf8().value.data();
        }
    }

    el = doc["_id"];
    if (el.raw() != nullptr)
    {
        if (el.type() == bsoncxx::type::k_oid)
        {
            oid = el.get_oid().value.to_string();
        }
    }

    el = doc["quantity"];
    if (el.raw() != nullptr)
    {
        if (el.type() == bsoncxx::type::k_double)
        {
            qty = el.get_double().value;
        }
    }

    return ItemReference(id, oid, qty);

}

bool RemoveFromCache(const BOM& bom)
{
    for (auto i = boms.begin(); i != boms.end(); i++)
    {
        if (*i == bom)
        {
            boms.erase(i);
            return true;
        }
    }
    return false;
}

std::string FindDiffs(const BOM& from, const BOM& to)
{
    std::string difs = "";

    if (from.GetId() != to.GetId())
    {
        difs += "\n\r\tID:\n\r\t\tFrom: " + from.GetId() + "\n\r\t\tTo: " + to.GetId();
    }
    if (from.GetName() != to.GetName())
    {
        difs += "\n\r\tName:\n\r\t\tFrom: " + from.GetName() + "\n\r\t\tTo: " + to.GetName();
    }
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
    if (from.GetRawItems().size() != to.GetRawItems().size())
    {
        int dif = from.GetRawItems().size() - to.GetRawItems().size();
        difs += "\n\r\tItems: \n\r\t\t";
        difs += dif < 0 ?
            "Removed " + StringUtils::NumToString(-dif) + " items" :
            "Added " + StringUtils::NumToString(dif) + " items";
    }

    return difs;
}
