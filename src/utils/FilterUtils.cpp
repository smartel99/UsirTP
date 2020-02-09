#include "FilterUtils.h"
#include "utils/db/Item.h"
#include "utils/db/Bom.h"
#include "utils/StringUtils.h"
#include "vendor/imgui/imgui.h"
#include "boost/algorithm/string.hpp"

namespace FilterUtils
{
FilterUtils::FilterHandler::FilterHandler(const std::vector<std::string>& categories)
{
    m_categories = categories;
    memset(m_filterText, 0, sizeof(m_filterText));
    m_selectedCategory = 0;
}

/**
 * @brief   Check if the passed `DB::Item::Item`'s `category` matches the filter.
 * @param   item: The Item to verify.
 * @param   category [optional]: The Item's category to verify.
 *                               If not specified, the currently selected category will be used instead.
 * @retval  `true` if the Item matches the filter, `false` otherwise.
 */
template<> bool FilterHandler::CheckMatch<DB::Item::Item>(const DB::Item::Item& item, int category)
{
    // Create a string from the filter input buffer.
    std::string filterText = m_filterText;
    // Transforms the input to all caps.
    boost::to_upper(filterText);

    // If no `category` was provided, used the selected one.
    // Otherwise use the one provided.
    switch (category == -1 ? m_selectedCategory : category)
    {
        case 0:     // ID.
        {
            std::string id = item.GetId();
            boost::to_upper(id);
            return (id.find(filterText) != std::string::npos);
        }
        case 1:     // Description.
        {
            std::string description = item.GetDescription();
            boost::to_upper(description);
            return (description.find(filterText) != std::string::npos);
        }
        case 2:     // Category.
        {
            std::string category = item.GetCategory().GetName();
            boost::to_upper(category);
            return (category.find(filterText) != std::string::npos);
        }
        case 3:     // Reference Link.
        {
            std::string refLink = item.GetReferenceLink();
            boost::to_upper(refLink);
            return (refLink.find(filterText) != std::string::npos);
        }
        case 4:     // Location.
        {
            std::string location = item.GetLocation();
            boost::to_upper(location);
            return (location.find(filterText) != std::string::npos);
        }
        case 5:     // Price.
        {
            // Price is a float, transform it into a string.
            std::string price = StringUtils::NumToString(item.GetPrice());
            boost::to_upper(price);
            return (price.find(filterText) != std::string::npos);
        }
        case 6:     // Quantity.
        {
            // Quantity is a float, transform it into a string.
            std::string quantity = StringUtils::NumToString(item.GetQuantity());
            boost::to_upper(quantity);
            return (quantity.find(filterText) != std::string::npos);
        }
        case 7:     // Unit.
        {
            std::string unit = item.GetUnit();
            boost::to_upper(unit);
            return (unit.find(filterText) != std::string::npos);
        }
        case 8:     // Status.
        {
            std::string status = DB::Item::GetStatusString(DB::Item::ItemStatus(item.GetStatus()));
            boost::to_upper(status);
            return (status.find(filterText) != std::string::npos);
        }
        default:
            return false;
    }
}

/**
 * @brief   Check if the passed `DB::BOM::BOM`'s `category` matches the filter.
 * @param   item: The BOM to verify.
 * @param   category [optional]: The BOM's category to verify.
 *                               If not specified, the currently selected category will be used instead.
 * @retval  `true` if the Item matches the filter, `false` otherwise.
 */
template<> bool FilterHandler::CheckMatch<DB::BOM::BOM>(const DB::BOM::BOM& item, int category)
{
    // Create a string from the filter input text's buffer.
    std::string filterText = m_filterText;
    // Transform the string into all caps to make the filter case insensitive.
    boost::to_upper(filterText);

    // If no `category` was provided, used the selected one.
    // Otherwise use the one provided.
    switch (category == -1 ? m_selectedCategory : category)
    {
        case 0:     // ID.
        {
            std::string id = item.GetId();
            boost::to_upper(id);
            return (id.find(filterText) != std::string::npos);
        }
        case 1:     // Description.
        {
            std::string description = item.GetName();
            boost::to_upper(description);
            return (description.find(filterText) != std::string::npos);
        }
        case 2:     // Output Item ID.
        {
            std::string id = item.GetRawOutput().GetId();
            boost::to_upper(id);
            return (id.find(filterText) != std::string::npos);
        }
        default:
            return false;
    }
}

void FilterUtils::FilterHandler::Render()
{
    // Create a child frame in the current frame with a width equal to the parent frame's width
    // and a height equals to the height of a regular frame's height (height of text) with 
    // spacing plus a 5 pixels padding.
    ImGui::BeginChildFrame(ImGui::GetID("FilterHandlerChildFrame"), ImVec2(0, ImGui::GetFrameHeightWithSpacing() + 5));

    // Create two columns in the frame, no ID, no visible division between each columns.
    ImGui::Columns(2, nullptr, false);

    // Input box of text for user to write the filter in, bound to the FilterUtils' input buffer.
    ImGui::InputText("Filter", m_filterText, sizeof(m_filterText));

    // Move to the next column.
    ImGui::NextColumn();

    // If categories were provided upon instantiation of the FilterUtils:
    if (!m_categories.empty())
    {
        // If the user as opened the combo box:
        if (ImGui::BeginCombo("Filter By", m_categories.at(m_selectedCategory).c_str()))
        {
            int i = 0;
            // For each category in the FilterUtils object:
            for (const std::string& category : m_categories)
            {
                bool selected = false;
                // If the user has clicked on the category:
                if (ImGui::Selectable(category.c_str(), &selected))
                {
                    // Update the selected category to be this one.
                    m_selectedCategory = i;
                }
                i++;
            }
            // End the combo box item.
            ImGui::EndCombo();
        }
    }
    // Move to the next column, a new row here.
    ImGui::NextColumn();
    // Create a single column so that everything lines up well.
    ImGui::Columns(1);
    // End the child frame we create earlier.
    ImGui::EndChildFrame();
}

}
