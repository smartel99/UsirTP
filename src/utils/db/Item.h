#pragma once

#include "utils/db/MongoCore.h"
#include "utils/db/Category.h"

#include <iostream>

namespace DB
{
/**
 *  @namespace  Item Item.h Item
 *  @brief      The Item namespace
 */
namespace Item  // Item namespace
{

/**
 * @enum    ItemStatus
 * @brief   The production status of the item.
 */
enum class ItemStatus
{
    active = 0, /*!< The item is currently used to make new products. */
    obsolete,   /*!< The item is not used to make new products nor to repair older out-of-production products. */
    /** Not Recommended For New Designs. The item is not used to make new products,
        but is used to service older out-of-production products. */
        nrfnd,
};

/**
 * @brief   Get a string representation of the item status.
 * @param   i The ItemStatus to convert into a string
 * @retval  The string representation of `i`
 */
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
        default:
            return "Invalid";
            break;
    }
}

/**
 * @class   Item
 * @brief   A class representing an Item document in the database.
 */
class Item
{
public:
    /**
     * @brief   The constructor of the class.
     * @param   oid: The mongodb ObjectId assigned to the Item. This is handled by the database.
     * @param   id: The CEP id of the item.
     * @param   description: The description of the item.
     * @param   category: The category of the item.
     * @param   referenceLink: A link, file path or any text to use as a reference for the item.
     * @param   location: The physical location of the item.
     * @param   price: The per-unit price of the item, in $CDN.
     * @param   qty: The available quantity of the item.
     * @param   unit: The unit to use when representing quantities of this item.
     * @param   status: The current production status of the item.
     */
    Item(const std::string& oid = "N/A",
         const std::string& id = "XXXX0000",
         const std::string& description = "Default Description",
         const Category::Category category = Category::Category(),
         const std::string& referenceLink = "Default Link",
         const std::string& location = "Default Location",
         const float& price = 0.00f,
         const float& qty = 0,
         const std::string& unit = "Default Unit",
         const ItemStatus& status = ItemStatus::active) :
        m_oid(oid), m_id(id), m_description(description), m_category(category),
        m_referenceLink(referenceLink), m_location(location),
        m_price(price), m_quantity(qty),
        m_unit(unit), m_status(status)
    {
    }
    ~Item() = default;

    /**
     * Get the ObjectId of the Item
     */
    inline const std::string& GetOid() const
    {
        return m_oid;
    }

    /**
     * Get the CEP id of the Item
     */
    inline const std::string& GetId() const
    {
        return m_id;
    }

    /**
     * Get the description of the item
     */
    inline const std::string& GetDescription() const
    {
        return m_description;
    }

    /**
     * Get the Category object of the item.
     */
    inline const DB::Category::Category& GetCategory() const
    {
        return m_category;
    }

    /**
     * Get the reference link (or whatever is stored there) of the item.
     */
    inline const std::string& GetReferenceLink() const
    {
        return m_referenceLink;
    }

    /**
     * Get the per-unit price (in $CDN) of the item.
     */
    inline const float& GetPrice() const
    {
        return m_price;
    }

    /**
     * Get the currently available quantity of the item.
     */
    inline const float& GetQuantity() const
    {
        return m_quantity;
    }

    /**
     * Set the currently available quantity of the item.
     * @param   val: The new available quantity
     */
    inline void SetQuantity(const float& val)
    {
        m_quantity = val;
    }

    /**
     * Increment the available quantity by `count`.
     * This is a shortcut to `Item::SetQuantity(Item::GetQuantity() + count)`
     * @param   count: The quantity to add to the available quantity.
     */
    inline void IncQuantity(float count = 1)
    {
        m_quantity += count;
    }

    /**
     * Decrement the available quantity by `count`.
     * This is a shortcut to `Item::SetQuantity(Item::GetQuantity() - count)`
     * @param   count: The quantity to remove from the available quantity.
     */
    inline void DecQuantity(float count = 1)
    {
        m_quantity -= count;
    }

    /**
     * Get the unit used by the item.
     */
    inline const std::string& GetUnit() const
    {
        return m_unit;
    }

    /**
     * Get the item's current production status as an integer.
     */
    inline int GetStatus() const
    {
        return int(m_status);
    }

    /**
     * Get the item's current production status as a string.
     */
    inline const std::string GetStatusAsString() const
    {
        return GetStatusString(m_status);
    }

    /**
     * Update the production status of the item.
     * @param   status: The item's new production status.
     */
    inline void SetStatus(const ItemStatus& status)
    {
        m_status = status;
    }

    /**
     * Get the location of the item.
     */
    inline const std::string& GetLocation() const
    {
        return m_location;
    }

    bool operator==(const Item& other)
    {
        return (m_id == other.m_id &&
                m_description == other.m_description &&
                m_category == other.m_category &&
                m_referenceLink == other.m_referenceLink &&
                GetLocation() == other.GetLocation() &&
                m_price == other.m_price &&
                m_quantity == other.m_quantity &&
                m_unit == other.m_unit &&
                m_status == other.m_status);
    }

private:
    /* The mongodb ObjectId */
    std::string             m_oid = "N/A";
    /* The CEP id */
    std::string             m_id = "XXXX0000";
    /* The description */
    std::string             m_description = "Default Description";
    /* The category */
    Category::Category      m_category = Category::Category();
    /* A link, file path or any text used as a reference */
    std::string             m_referenceLink = "Default Link";
    /* The physical location */
    std::string             m_location = "Default Location";
    /* The price per unit, in $CDN */
    float                   m_price = 0.00f;
    /* The currently available quantity */
    float                   m_quantity = 0;
    /* The unit used when representing quantities */
    std::string             m_unit = "Default Unit";
    /* The current production status */
    ItemStatus              m_status = ItemStatus::active;
};

bool Init();
void Refresh();
bool AddItem(const Item& it);

Item GetItemByName(const std::string& name);
Item GetItemByID(const std::string& prefix);

std::string GetNewId(const DB::Category::Category& cat, int id = -1);

bool EditItem(const Item& oldItem, const Item& newItem);
bool DeleteItem(Item& item);

const std::vector<Item>& GetAll();


}   // namespace Item.
}   // namespace DB.
