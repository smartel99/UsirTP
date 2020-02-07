/**
 ******************************************************************************
 * @addtogroup Bom
 * @{
 * @file    Bom
 * @author  Samuel Martel
 * @brief   Header for the Bom module.
 *
 * @date 1/23/2020 8:54:07 AM
 *
 ******************************************************************************
 */
#ifndef _Bom
#define _Bom

/*****************************************************************************/
/* Includes */
#include "utils/db/MongoCore.h"
#include "utils/db/Item.h"
#include <iostream>
#include <vector>

namespace DB
{
/**
 * @namespace BOM
 * @brief   The Bill of Material Namespace.
 */
namespace BOM
{
/*****************************************************************************/
/* Exported defines */


/*****************************************************************************/
/* Exported macro */


/*****************************************************************************/
/* Exported types */

/**
 * @class   ItemReference Bom.h Bom
 * @brief   A class containing minimal information about a DB::Item::Item object
 */
class ItemReference
{
public:
    /**
     * @brief   Default constructor.
     */
    ItemReference() = default;

    /**
     * @brief   Construct an object with an item id, its mongodb's ObjectId and the quantity.
     */
    ItemReference(const std::string& id, const std::string& objId, float qty, int position) :
        m_id(id), m_objectId(objId), m_qty(qty), m_position(position)
    {
    }

    /**
     * @brief   Construct an object with only the item's id, automatically fetching the ObjectId.
     */
    ItemReference(const std::string& itemId)
    {
        DB::Item::Item item = DB::Item::GetItemByID(itemId);
        m_id = item.GetId();
        m_qty = 1.f;
        m_objectId = item.GetOid();
    }

    /**
     * @brief   Get the id.
     * @param   None
     * @retval  The id.
     */
    inline const std::string& GetId()const
    {
        return m_id;
    }

    /**
     * @brief   Get the object id.
     * @param   None
     * @retval  The object id.
     */
    inline const std::string& GetObjId() const
    {
        return m_objectId;
    }

    inline const int GetPosition() const
    {
        return m_position;
    }

    inline void SetPosition(int position)
    {
        m_position = position;
    }

    /**
     * @brief   Get the quantity.
     * @param   None
     * @retval  The quantity.
     */
    inline const float& GetQuantity() const
    {
        return m_qty;
    }

    /**
     * @brief   Set the quantity.
     * @param   qty: the new quantity.
     * @retval  None
     */
    inline void SetQuantity(float qty)
    {
        m_qty = qty;
    }

    /**
     * @brief   Return a copy of the DB::Item::Item represented by `this`.
     * @param   None
     * @retval  The instance of DB::Item::Item
     */
    inline DB::Item::Item GetItem() const
    {
        return DB::Item::GetItemByID(m_id);
    }

    /**
     * @brief   Overload the == operator so it compares the members of each objects.
     * @param   other: The other object to compare against.
     * @retval  True if they are identical, false otherwise.
     */
    bool operator==(const ItemReference& other) const
    {
        return (m_id == other.m_id &&
                m_objectId == other.m_objectId &&
                m_qty == other.m_qty);
    }

    /**
     * @brief   Overload the != operator so it compares the members of each objects.
     * @param   other: The other object to compare against.
     * @retval  True if they are not identical, false otherwise.
     */
    bool operator!=(const ItemReference& other) const
    {
        return !(*this == other);
    }

private:

    std::string m_id = "XXXX0000";  /**< The CEP id of the item */
    std::string m_objectId = "N/A"; /**< The mongodb ObjectId of the item */
    float       m_qty = 0.00f;      /**< The quantity needed by the BOM */
    int         m_position = 0;     /**< The item's position in the list */
};

/**
 * @class   BOM Bom.h Bom
 * @brief   An object representing a Bill of Material document in the database.
 */
class BOM
{
public:
    /**
     * @brief   The default constructor.
     */
    BOM() = default;

    /**
     * @brief   The complete constructor.
     * @param   id: The CEP id to assign to the BOM object.
     * @param   name: The name/description of the new BOM.
     * @param   items: A list of all the items used by the BOM.
     * @param   outputItem: The item the BOM will use as an output.
     */
    BOM(const std::string& id, const std::string& name, const std::vector<ItemReference>& items, const ItemReference& outputItem) :
        m_id(id), m_name(name), m_items(items), m_output(outputItem)
    {
    }

    /**
     * @brief   Get the CEP id of the BOM
     * @param   None
     * @retval  The CEP id of the BOM
     */
    inline const std::string& GetId() const
    {
        return m_id;
    }

    /**
     * @brief   Get the name of the BOM
     * @param   None
     * @retval  The name of the BOM
     */
    inline const std::string& GetName() const
    {
        return m_name;
    }

    /**
     * @brief   Get a list of all the items in the BOM
     * @param   None
     * @retval  A list of ItemReference objects
     */
    inline const std::vector<ItemReference>& GetRawItems() const
    {
        return m_items;
    }

    /**
     * @brief   Get the output object of the BOM
     * @param   None
     * @retval  The output object as a ItemReference instance
     */
    inline const ItemReference& GetRawOutput() const
    {
        return m_output;
    }

    /**
     * @brief   Overload the == operator to compare against the m_id and m_name fields
     *          of the two object.
     * @param   other: the object to compare against.
     * @retval  True if the two m_ids and m_names are identical, false otherwise.
     */
    bool operator==(const BOM& other)
    {
        return (m_id == other.m_id && m_name == other.m_name);
    }

    /**
     * @brief   Get the output item as a DB::Item::Item instance.
     * @param   None
     * @retval  The DB::Item::Item object corresponding to the output item.
     */
    const DB::Item::Item GetOutput() const;


    /**
     * @brief   Get the list of items as DB::Item::Item instances.
     * @param   None
     * @retval  A std::vector of DB::Item::Item objects corresponding to the list of items.
     */
    const std::vector<DB::Item::Item> GetItems() const;
private:
    std::string m_id = "N/A";   //!< The CEP id of the BOM.
    std::string m_name = "N/A"; //!< The name of the BOM.
    std::vector<ItemReference> m_items = std::vector<ItemReference>(); //!< A list of the items needed by the BOM.
    ItemReference m_output;     //!< The item used as an output by the BOM.
};

/*****************************************************************************/
/* Exported functions */


bool Init();
void Refresh();

bool AddBom(const BOM& bom);
bool EditBom(const BOM& oldBom, const BOM& newBom);
bool DeleteBom(const BOM& bom);

const std::vector<BOM>& GetAll();
std::string GetNewId();
}
}
/* Have a wonderful day :) */
#endif /* _Bom */
/**
 * @}
 */
/****** END OF FILE ******/
