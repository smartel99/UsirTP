#pragma once

#include "utils/db/MongoCore.h"
#include "utils/db/Category.h"

#include <iostream>

namespace DB
{
namespace Item
{
enum class ItemStatus
{
    active = 0,
    obsolete,
    nrfnd,   // Not Recommended For New Designs.
    end
};

inline const std::string GetStatusString(ItemStatus i)
{
    switch (i)
    {
        case DB::Item::ItemStatus::active:
            return "Active";
            break;
        case DB::Item::ItemStatus::obsolete:
            return "Obsolete";
            break;
        case DB::Item::ItemStatus::nrfnd:
            return "Not Recommended for New Designs";
            break;
        case DB::Item::ItemStatus::end:
        default:
            return "Invalid";
            break;
    }
}

class Item
{
public:
    Item(const std::string& id = "XXXX0000",
         const std::string& description = "Default Description",
         const Category::Category category = Category::Category(),
         const std::string& referenceLink = "Default Link",
         const float& price = 0.00f,
         const float& qty = 0,
         const std::string& unit = "Default Unit",
         const ItemStatus& status = ItemStatus::active) :
        m_id(id), m_description(description), m_category(category),
        m_referenceLink(referenceLink), m_price(price), m_quantity(qty),
        m_unit(unit), m_status(status)
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

    inline DB::Category::Category& GetCategory()
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

    inline const float& GetQuantity() const
    {
        return m_quantity;
    }

    inline void SetQuantity(const unsigned int& val)
    {
        m_quantity = val;
    }

    inline const std::string& GetUnit() const
    {
        return m_unit;
    }

    inline const int& GetStatus() const
    {
        return int(m_status);
    }

    inline const std::string GetStatusAsString() const
    {
        return GetStatusString(m_status);
    }

    inline void SetStatus(const ItemStatus& status)
    {
        m_status = status;
    }

    bool operator==(const Item& other)
    {
        return (m_id == other.m_id &&
                m_description == other.m_description &&
                m_category == other.m_category &&
                m_referenceLink == other.m_referenceLink &&
                m_price == other.m_price &&
                m_quantity == other.m_quantity &&
                m_unit == other.m_unit &&
                m_status == other.m_status);
    }

private:
    std::string             m_id = "XXXX0000";
    std::string             m_description = "Default Description";
    Category::Category      m_category = Category::Category();
    std::string             m_referenceLink = "Default Link";
    float                   m_price = 0.00f;
    float                   m_quantity = 0;
    std::string             m_unit = "Default Unit";
    ItemStatus              m_status = ItemStatus::active;
};

bool Init(void);
void Refresh();
bool AddItem(const Item& category);

Item GetItemByName(const std::string& name);
Item GetItemByID(const std::string& prefix);

int GetNewId();

bool EditItem(const Item& oldItem, const Item& newItem);
bool DeleteItem(const Item& item);

const std::vector<Item>& GetAll();


}   // namespace Item.
}   // namespace DB.
