#pragma once

#include "utils/db/MongoCore.h"
#include <iostream>

namespace DB
{
namespace Category
{
class Category
{
public:
    Category() = default;
    Category(const std::string& name, const std::string& prefix) :
        m_name(name), m_idPrefix(prefix)
    {
    }
    ~Category() = default;

    inline const std::string& GetName()
    {
        return m_name;
    }

    inline const std::string& GetPrefix()
    {
        return m_idPrefix;
    }

    inline bool IsValid()
    {
        return (!m_name.empty() && !m_idPrefix.empty());
    }

    bool operator==(const Category& other)
    {
        return (m_name == other.m_name && m_idPrefix == other.m_idPrefix);
    }

private:
    std::string m_name = "Default Name";
    std::string m_idPrefix = "DFLT";
};

bool Init(void);
bool AddCategory(const Category& category);

Category GetCategoryByName(const std::string& name);
Category GetCategoryByPrefix(const std::string& prefix);

bool EditCategory(const Category& oldCat, const Category& newCat);
bool DeleteCategory(const Category& category);

}   // Namespace Category
}   // namespace DB
