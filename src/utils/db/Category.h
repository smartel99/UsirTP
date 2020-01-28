#pragma once

#include "utils/db/MongoCore.h"
#include <iostream>

namespace DB
{
/**
 * @namespace   Category
 * @brief       The Category Namespace
 */
namespace Category
{
/**
 * @class   Category
 * @brief   A class representing a Category document in the database
 */
class Category
{
public:
    Category() = default;
    Category(const std::string& name, const std::string& prefix, char suffix = '\0') :
        m_name(name), m_idPrefix(prefix), m_suffix(suffix)
    {
    }
    ~Category() = default;

    /**
     * Get the name of the category
     */
    inline const std::string& GetName() const
    {
        return m_name;
    }

    /**
     * Get the prefix of the category.
     * The prefix is 4 characters that are used for the generation of item ids.
     */
    inline const std::string& GetPrefix() const
    {
        return m_idPrefix;
    }

    /**
     * Check if the category is valid, that is, if it has a name and a prefix.
     */
    inline bool IsValid()
    {
        return (!m_name.empty() && !m_idPrefix.empty());
    }

    /**
     * Get the suffix of the category.
     * The suffix is a single character that is used for the generation of item ids.
     */
    inline const char GetSuffix() const
    {
        return m_suffix;
    }

    bool operator==(const Category& other) const
    {
        return (m_name == other.m_name && m_idPrefix == other.m_idPrefix && m_suffix == other.m_suffix);
    }

    bool operator!=(const Category& other) const
    {
        return !(*this == other);
    }

private:
    std::string m_name = "";                    /**< The name of the category */
    std::string m_idPrefix = "";                /**< The prefix that is added to an item's id */
    char m_suffix = '\0';                       /**< The suffix that is added to an item's id */
};

bool Init();
void Refresh();
bool AddCategory(const Category& category);

Category GetCategoryByName(const std::string& name);
Category GetCategoryByPrefix(const std::string& prefix);

bool EditCategory(const Category& oldCat, const Category& newCat);
bool DeleteCategory(const Category& category);

const std::vector<Category>& GetAll();

}   // Namespace Category
}   // namespace DB
