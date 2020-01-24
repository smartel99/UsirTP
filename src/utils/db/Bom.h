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
namespace BOM
{
/*****************************************************************************/
/* Exported defines */


/*****************************************************************************/
/* Exported macro */


/*****************************************************************************/
/* Exported types */
class ItemReference
{
public:
    ItemReference()
        = default;
    ItemReference(const std::string& id, const std::string& objId, float qty) :
        m_id(id), m_objectId(objId), m_qty(qty)
    {
    }

    ItemReference(const std::string& itemId)
    {
        DB::Item::Item item = DB::Item::GetItemByID(itemId);
        m_id = item.GetId();
        m_qty = 1.f;
        m_objectId = item.GetOid();
    }

    inline const std::string& GetId()const
    {
        return m_id;
    }

    inline const std::string& GetObjId() const
    {
        return m_objectId;
    }

    inline const float& GetQuantity() const
    {
        return m_qty;
    }

    inline void SetQuantity(float qty)
    {
        m_qty = qty;
    }

private:
    std::string m_id = "XXXX0000";
    std::string m_objectId = "N/A";
    float       m_qty = 0.00f;
};

class BOM
{
public:
    BOM() = default;
    BOM(const std::string& id, const std::string& name, const std::vector<ItemReference>& items, const ItemReference& outputItemId) :
        m_id(id), m_name(name), m_items(items), m_output(outputItemId)
    {
    }

    inline const std::string& GetId() const
    {
        return m_id;
    }

    inline const std::string& GetName() const
    {
        return m_name;
    }

    inline const std::vector<ItemReference>& GetRawItems() const
    {
        return m_items;
    }

    inline const ItemReference& GetRawOutput() const
    {
        return m_output;
    }
    const DB::Item::Item GetOutput() const;
    const std::vector<DB::Item::Item> GetItems() const;
private:
    std::string m_id = "N/A";
    std::string m_name = "N/A";
    std::vector<ItemReference> m_items = std::vector<ItemReference>();
    ItemReference m_output;
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
