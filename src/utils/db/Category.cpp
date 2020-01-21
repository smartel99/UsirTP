#include "Category.h"
#include "vendor/imgui/imgui.h"
#include <vector>

#define CHECK_IS_INIT(...)  if(isInit==false){isInit=true;Init();}

namespace DB
{
namespace Category
{

static bsoncxx::document::value CreateDocument(Category cat);
static bsoncxx::document::value CreateDocument(const std::string& field, const std::string& val);
static bsoncxx::document::value CreateDocumentForUpdate(Category cat);
static Category CreateObject(const bsoncxx::document::view& doc);
static bool FindInCache(Category& cat, const std::string& filter);
static bool FindInCache(Category& cat);
static bool RemoveFromCache(const Category& cat);

static std::vector<Category> categories;
static bool isInit = false;

bool DB::Category::Init(void)
{
    categories.clear();
    for (auto cat : DB::GetAllDocuments("CEP", "Categories"))
    {
        categories.emplace_back(CreateObject(cat));
    }
    return true;
}

void Refresh()
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

bool DB::Category::AddCategory(const Category& category)
{
    CHECK_IS_INIT();

    categories.emplace_back(category);

    bsoncxx::document::value catDoc = CreateDocument(category);

    return DB::InsertDocument(catDoc, "CEP", "Categories");
}

Category DB::Category::GetCategoryByName(const std::string& name)
{
    Category c = Category("", "");

    if (FindInCache(c, name) == false)
    {
        c = CreateObject(DB::GetDocument("CEP", "Categories", CreateDocument("name", name)));
        if (c.IsValid() == true)
        {
            categories.emplace_back(c);
        }
    }

    return c;
}

Category DB::Category::GetCategoryByPrefix(const std::string& prefix)
{
    Category c = Category("", "");

    if (FindInCache(c, prefix) == false)
    {
        c = CreateObject(DB::GetDocument("CEP", "Categories", CreateDocument("prefix", prefix)));
        if (c.IsValid() == true)
        {
            categories.emplace_back(c);
        }
    }

    return c;
}

bool EditCategory(const Category& oldCat, const Category& newCat)
{
    RemoveFromCache(oldCat);
    categories.emplace_back(newCat);

    return (DB::UpdateDocument(CreateDocument(oldCat), CreateDocumentForUpdate(newCat), "CEP", "Categories"));
}

bool DeleteCategory(const Category& category)
{
    RemoveFromCache(category);

    return (DB::DeleteDocument(CreateDocument(category), "CEP", "Categories"));
}

const std::vector<Category>& GetAll()
{
    return categories;
}

bsoncxx::document::value CreateDocument(Category cat)
{
    auto builder = bsoncxx::builder::stream::document{};
    bsoncxx::document::value doc = builder
        << "name" << cat.GetName()
        << "prefix" << cat.GetPrefix()
        << bsoncxx::builder::stream::finalize;

    return doc;
}

bsoncxx::document::value CreateDocument(const std::string& field, const std::string& val)
{
    auto builder = bsoncxx::builder::stream::document{};
    bsoncxx::document::value doc = builder
        << field << val
        << bsoncxx::builder::stream::finalize;

    return doc;
}

bsoncxx::document::value CreateDocumentForUpdate(Category cat)
{
    auto builder = bsoncxx::builder::stream::document{};
    bsoncxx::document::value doc = builder
        << "$set" << bsoncxx::builder::stream::open_document
        << "name" << cat.GetName()
        << "prefix" << cat.GetPrefix()
        << bsoncxx::builder::stream::close_document
        << bsoncxx::builder::stream::finalize;

    return doc;
}

Category CreateObject(const bsoncxx::document::view& doc)
{
    std::string name = "";
    std::string prefix = "";

    bsoncxx::document::element el = doc["name"];
    if (el.raw() != nullptr)
    {
        if (el.type() == bsoncxx::type::k_utf8)
        {
            name = el.get_utf8().value.data();
        }
    }

    el = doc["prefix"];
    if (el.raw() != nullptr)
    {
        if (el.type() == bsoncxx::type::k_utf8)
        {
            prefix = el.get_utf8().value.data();
        }
    }

    return Category(name, prefix);
}

bool FindInCache(Category& cat, const std::string& filter)
{
    for (Category c : categories)
    {
        if (c.GetName().find(filter) != std::string::npos ||
            c.GetPrefix().find(filter) != std::string::npos)
        {
            cat = Category(c);
            return true;
        }
    }
    return false;
}

bool FindInCache(Category& cat)
{
    for (Category category : categories)
    {
        if (category == cat)
        {
            cat = Category(category);
            return true;
        }
    }
    return false;
}

bool RemoveFromCache(const Category& cat)
{
    for (auto c = categories.begin(); c != categories.end(); c++)
    {
        if (*c == cat)
        {
            categories.erase(c);
            return true;
        }
    }
    return false;
}

}
}
