#include "Category.h"
#include "vendor/imgui/imgui.h"
#include "widgets/Logger.h"
#include <vector>

#define CHECK_IS_INIT(...)  if(isInit==false){isInit=true;Init();}
#define IS_INIT     (isInit == true)

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

static std::vector<Category> categories; //!< Cache.
static bool isInit = false;
static bool hasError = false;

/**
 * @brief   Initialize the Category module:
 *              - Clear the cache
 *              - Load all the Categories from the database into the cache.
 *          If the initialization fails (connection issue, authentication error, etc.),
 *          it always returns false and the cache will be empty.
 * @param   None
 * @retval  True if successful, false otherwise.
 */
bool DB::Category::Init()
{
    if (hasError)
    {
        return false;
    }

    // Clear the cache.
    categories.clear();
    // Get all the categories from the database.
    bsoncxx::stdx::optional<mongocxx::cursor> cats = DB::GetAllDocuments(DATABASE, "Categories");

    // `cats` will be `{}` if the query failed.
    if (!cats)
    {
        isInit = false;
        return false;
    }

    // If `if(!cats)` passes (`cats` isn't `{}`) yet doesn't contain any value, `cats.value()` will 
    // throw `mongocxx::query_exception`.
    try
    {
        // For each document returned by the database:
        for (auto cat : cats.value())
        {
            // Create a Category object from that document and add it to the cache.
            categories.emplace_back(CreateObject(cat));
        }
        isInit = true;
        return true;
    }
    catch (const mongocxx::query_exception & e)
    {
        Logging::System.Critical("An error occurred when initializing categories: ", e.what());
        hasError = true;
        return false;
    }
}

/**
 * @brief   Refreshes the cache once every 10 seconds
 * @param   None
 * @retval  None
 */
void Refresh()
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
 * @brief   Adds a new Category to the database and the cache.
 * @param   category The Category to insert into the database.
 * @retval  True if insertion was successful, false otherwise.
 *
 * @note    Category will be added to the cache even if insertion into database
 *          fails. It will however be removed at the next Refresh event.
 */
bool DB::Category::AddCategory(const Category& category)
{
    if (!IS_INIT)
    {
        return false;
    }

    // Add the category to the cache.
    categories.emplace_back(category);

    // Create a mongodb document from the Category object.
    bsoncxx::document::value catDoc = CreateDocument(category);

    // Insert the newly created document in the database.
    return DB::InsertDocument(catDoc, DATABASE, "Categories");
}

/**
 * @brief   Search for a Category object by its name.
 *          We first search in the cache, if nothing is found, we query the database.
 * @param   name The name of the desired Category.
 * @retval  The matching Category.
 *
 * @note    If no Category matching the passed name is found, an empty (non-valid)
 *          Category will be returned.
 */
Category DB::Category::GetCategoryByName(const std::string& name)
{
    // Create a default, non-valid Category to store the output.
    Category c = Category("", "");
    if (!IS_INIT)
    {
        return c;
    }

    // Search in the cache for a Category with a matching name.
    if (FindInCache(c, name) == false)
    {
        // If no matching category was found in the cache, query the database.
        c = CreateObject(DB::GetDocument(DATABASE, "Categories", CreateDocument("name", name)));
        // If a match was found:
        if (c.IsValid() == true)
        {
            // Add it to the cache.
            categories.emplace_back(c);
        }
    }

    return c;
}

/**
 * @brief   Search for a Category object by its prefix.
 *          We first search in the cache, if nothing is found, we query the database.
 * @param   prefix The prefix of the desired Category.
 * @retval  The matching Category.
 *
 * @note    If no Category matching the passed prefix is found, an empty (non-valid)
 *          Category will be returned.
 */
Category DB::Category::GetCategoryByPrefix(const std::string& prefix)
{
    // Create a default, non-valid Category object.
    Category c = Category("", "");
    if (!IS_INIT)
    {
        return c;
    }

    // Search in the cache for a matching Category.
    if (FindInCache(c, prefix) == false)
    {
        // If there was no match, query the database.
        c = CreateObject(DB::GetDocument(DATABASE, "Categories", CreateDocument("prefix", prefix)));
        // If the query succeeded and a Category was returned by the database:
        if (c.IsValid() == true)
        {
            // Add it to the cache.
            categories.emplace_back(c);
        }
    }

    return c;
}

/**
 * @brief   Update a currently existing Category in the cache and the database.
 * @param   oldBom: The Category to edit.
 * @param   newBom: The new value of the Category.
 * @retval  True if successfully edited in the database, false otherwise.
 *
 * @note    Even if the modification in the database fails, the Category will be modified in the cache.
 *          It will remain like that until the next refresh event.
 * @note    The oldBom in the cache isn't technically modified, but rather deleted. The newBom is then
 *          added to the cache.
 */
bool EditCategory(const Category& oldCat, const Category& newCat)
{
    if (!IS_INIT)
    {
        return false;
    }

    // Remove the old category from the cache.
    RemoveFromCache(oldCat);
    // Add the "new" category in the cache.
    categories.emplace_back(newCat);

    // Update the document in the database:
    //  - CreateDocument -> Create a mongodb document containing only the id of the old category to use as a filter.
    //  - CreateDocumentForUpdate -> Create a mongodb document with the new category.
    return (DB::UpdateDocument(CreateDocument("prefix", oldCat.GetPrefix()),
                               CreateDocumentForUpdate(newCat), DATABASE, "Categories"));
}

/**
 * @brief   Delete a Category from the cache and the database.
 * @param   The Category to delete. Only the `m_id` field of the Category object is used to query the database.
 * @retval  True if successfully deleted from the database, false otherwise.
 * @note    Even if the Category isn't deleted from the database, it will still be removed from the cache.
 *          This will remain until the next refresh event.
 */
bool DeleteCategory(const Category& category)
{
    if (!IS_INIT)
    {
        return false;
    }

    // Remove the category from the cache.
    RemoveFromCache(category);

    // Delete the category from the database.
    return (DB::DeleteDocument(CreateDocument(category), DATABASE, "Categories"));
}

/**
 * @brief   Get the entire cache of Categories as is.
 * @param   None
 * @retval  A const reference to the cache.
 */
const std::vector<Category>& GetAll()
{
    return categories;
}

/**
 * @brief   Create a mongodb document from a Category object.
 *          The created document is pretty much just a JSON dump of the object.
 * @param   cat: The Category object to create a document with.
 * @retval  The created document.
 */
bsoncxx::document::value CreateDocument(Category cat)
{
    // Create the document builder.
    auto builder = bsoncxx::builder::stream::document{};

    // Add all the members of the Category to the document.
    bsoncxx::document::value doc = builder
        << "name" << cat.GetName()
        << "prefix" << cat.GetPrefix()
        << "suffix" << cat.GetSuffix()
        << bsoncxx::builder::stream::finalize;

    return doc;
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
bsoncxx::document::value CreateDocumentForUpdate(Category cat)
{
    using bsoncxx::builder::basic::kvp;
    // Create the builder object.
    auto builder = bsoncxx::builder::basic::document{};
    // Create a mongodb document from the Category object and add it to the document.
    builder.append(kvp("$set", CreateDocument(cat)));

    // Make a `bsoncxx::document::value` from the builder.
    return builder.extract();
}

/**
 * @brief   Create a Category instance from a mongodb document.
 * @param   doc The document to use.
 * @retval  A Category instance corresponding to the document.
 */
Category CreateObject(const bsoncxx::document::view& doc)
{
    std::string name = "";
    std::string prefix = "";
    char suffix = '\0';

    // Get the name field of the document.
    bsoncxx::document::element el = doc["name"];
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

    // Get the prefix field of the document.
    el = doc["prefix"];
    // If the field exists:
    if (el.raw() != nullptr)
    {
        // If the field is of the desired type:
        if (el.type() == bsoncxx::type::k_utf8)
        {
            // Get the value.
            prefix = el.get_utf8().value.data();
        }
    }

    // Get the suffix field of the document.
    el = doc["suffix"];
    // If the field exists:
    if (el.raw() != nullptr)
    {
        // If the field is of the desired type:
        if (el.type() == bsoncxx::type::k_int32)
        {
            // Get the value.
            suffix = el.get_int32().value;
        }
    }

    // Instantiate Category with the values we extracted from the document.
    return Category(name, prefix, suffix);
}

/**
 * @brief   Find in the cache a Category that matches the filter.
 * @param   cat [out] A reference to a Category object used to store the category that was found.
 * @param   filter A filter to use to filter the categories
 * @retval  True if a Category matching the filter was found in the cache, false otherwise.
 */
bool FindInCache(Category& cat, const std::string& filter)
{
    // For each categories in the cache:
    for (const Category& c : categories)
    {
        // If the category matches the filter:
        if (c.GetName().find(filter) != std::string::npos ||
            c.GetPrefix().find(filter) != std::string::npos)
        {
            // Write that Category into cat.
            cat = Category(c);
            return true;
        }
    }
    // No match was found.
    return false;
}

/**
 * @brief   Find in the cache a Category that is identical to the one passed.
 *          I have no idea why I wrote this, but my hypothesis is that we can use this function
 *          to check if a Category exists in the cache or not.
 * @param   cat [out] A reference to a Category object used to store the category that was found.
 * @retval  True if a Category was found in the cache, false otherwise.
 */
bool FindInCache(Category& cat)
{
    // For each categories in the cache:
    for (const Category& category : categories)
    {
        // If the category is identical to the passed Category:
        if (category == cat)
        {
            // We found it!
            cat = Category(category);
            return true;
        }
    }
    return false;
}

/**
 * @brief   Remove a category from the cache
 * @param   cat The Category to remove
 * @retval  True if a Category was removed from the cache, false otherwise.
 */
bool RemoveFromCache(const Category& cat)
{
    // For each Category in the cache:
    for (auto c = categories.begin(); c != categories.end(); c++)
    {
        // If it matches the passed Category:
        if (*c == cat)
        {
            // Remove it from the cache.
            categories.erase(c);
            return true;
        }
    }
    // No matching Category was found in the cache.
    return false;
}

}
}
