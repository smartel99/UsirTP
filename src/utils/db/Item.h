#pragma once

#include "utils/db/MongoCore.h"
#include "utils/db/Category.h"

#include <iostream>

namespace DB
{
namespace Item
{
class Item
{
public:
    Item() = default;
    Item(const std::string& id = "XXXX0000",
         const std::string& description = "Default Description",
         const Category::Category category = Category::Category(),
         const std::string& referenceLink = "Default Link",
         const float& price = 0.00f,
         const unsigned int& qty = 0) :
        m_id(id), m_description(description), m_category(category),
        m_referenceLink(referenceLink), m_price(price), m_quantity(qty)
    {
    }
    ~Item() = default;

    inline const std::string& GetId()
    {
        return m_id;
    }

    inline const std::string& GetDescription()
    {
        return m_description;
    }

    inline const DB::Category::Category& GetCategory() const
    {
        return m_category;
    }

    inline const std::string& GetReferenceLink() const
    {
        return m_referenceLink;
    }

    inline const float& GetPrice() const
    {
        return m_price;
    }

    inline const unsigned int& GetQuantity() const
    {
        return m_quantity;
    }

    inline void SetQuantity(const unsigned int& val)
    {
        m_quantity = val;
    }

private:
    std::string     m_id = "XXXX0000";
    std::string     m_description = "Default Description";
    Category::Category m_category = Category::Category();
    std::string     m_referenceLink = "Default Link";
    float           m_price = 0.00f;
    unsigned int    m_quantity = 0;
};



}   // namespace Item.
}   // namespace DB.
