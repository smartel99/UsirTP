﻿#pragma once

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
    Category(const std::string& name, const std::string& prefix, char suffix = '\0') :
        m_name(name), m_idPrefix(prefix), m_suffix(suffix)
    {
    }
    ~Category() = default;

    inline const std::string& GetName() const
    {
        return m_name;
    }

    inline const std::string& GetPrefix() const
    {
        return m_idPrefix;
    }

    inline bool IsValid()
    {
        return (!m_name.empty() && !m_idPrefix.empty());
    }

    bool operator==(const Category& other)
    {
        return (m_name == other.m_name && m_idPrefix == other.m_idPrefix && m_suffix == other.m_suffix);
    }

    inline const char GetSuffix() const
    {
        return m_suffix;
    }
private:
    std::string m_name = "Default Category";
    std::string m_idPrefix = "DFLT";
    char m_suffix = '\0';
};

bool Init(void);
void Refresh();
bool AddCategory(const Category& category);

Category GetCategoryByName(const std::string& name);
Category GetCategoryByPrefix(const std::string& prefix);

bool EditCategory(const Category& oldCat, const Category& newCat);
bool DeleteCategory(const Category& category);

const std::vector<Category>& GetAll();

}   // Namespace Category
}   // namespace DB
