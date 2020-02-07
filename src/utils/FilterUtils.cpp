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

template<> bool FilterHandler::CheckMatch<DB::Item::Item>(const DB::Item::Item& item, int category)
{
    std::string filterText = m_filterText;
    boost::to_upper(filterText);
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
            std::string price = StringUtils::NumToString(item.GetPrice());
            boost::to_upper(price);
            return (price.find(filterText) != std::string::npos);
        }
        case 6:     // Quantity.
        {
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

template<> bool FilterHandler::CheckMatch<DB::BOM::BOM>(const DB::BOM::BOM& item, int category)
{
    std::string filterText = m_filterText;
    boost::to_upper(filterText);
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
    ImGui::BeginChildFrame(ImGui::GetID("FilterHandlerChildFrame"), ImVec2(0, ImGui::GetFrameHeightWithSpacing() + 5));
    ImGui::Columns(2, nullptr, false);

    ImGui::InputText("Filter", m_filterText, sizeof(m_filterText));
    ImGui::NextColumn();
    if (!m_categories.empty())
    {
        if (ImGui::BeginCombo("Filter By", m_categories.at(m_selectedCategory).c_str()))
        {
            int i = 0;
            for (const std::string& category : m_categories)
            {
                bool selected = false;
                if (ImGui::Selectable(category.c_str(), &selected))
                {
                    m_selectedCategory = i;
                }
                i++;
            }
            ImGui::EndCombo();
        }
    }
    ImGui::NextColumn();
    ImGui::Columns(1);
    ImGui::EndChildFrame();
}

}
