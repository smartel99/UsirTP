#include "BomViewer.h"
#include "utils/db/Bom.h"
#include "vendor/imgui/imgui.h"
#include "boost/algorithm/string.hpp"
#include "widgets/Popup.h"
#include "widgets/Logger.h"
#include "utils/Document.h"
#include "utils/FilterUtils.h"
#include "utils/Fonts.h"
#include "utils/StringUtils.h"
#include <algorithm>
#include <iostream>
#include <fstream>


#define MAX_INPUT_LENGHT    400
#define SORT_ASCEND         "(A to Z)"
#define SORT_DESCEND        "(Z to A)"
#define SORT_TXT(x)         (sort == SortBy::x ? SORT_ASCEND : (sort == SortBy::r ## x ? SORT_DESCEND : " "))
#define IS_SORT_ACTIVE(x)   (( sort == SortBy::x ) || ( sort == SortBy::r ## x ))

enum class SortBy
{
    id = 0,
    rid,
    name,
    rname
};

class ItemRef
{
public:
    ItemRef(const DB::BOM::ItemReference& items, bool selected) :
        reference(items), isSelected(selected)
    {
    }
    DB::BOM::ItemReference reference;
    bool isSelected = false;
    float quantity = 0.0f;
};

static void RenderAddWindow();
static void RenderEditWindow();
static void SortItems(SortBy sort, std::vector<DB::BOM::BOM>& boms);
static void MakeNewPopup();
static void MakeEditPopup(bool isRetry = false);
static void MakeDeletePopup();
static void MakeMakePopup();
static void SaveNewBom();
static void SaveEditedBom();
static void DeleteBom();
static void CommitMake();
static void CancelAction();

static void HandlePopupNameInput();
static void HandlePopupItemPickerInput();
static void HandlePopupOutputPickerInput();
static void HandlePopupMake();
static bool HandlePopupMakeButton(const std::string& label);

static void ExportItems();
static void OpenOutputFile();

static DB::BOM::BOM tmpBom;
static std::vector<ItemRef> tmpItems;
static DB::BOM::ItemReference tmpOutput;
static int tmpSelectedOut = 0;
static char tmpName[MAX_INPUT_LENGHT];
static int tmpQuantityMade = 1;
static int tmpQuantityToMake = 1;
static bool isAddOpen = false;
static bool isEditOpen = false;
static bool isMakeValid = false;
static std::wstring tmpOutputFilePath = L"";

static FilterUtils::FilterHandler itemFilter;

const static std::vector<std::string> cats = { "ID", "Description", "Output ID" };
static FilterUtils::FilterHandler filter(cats);

void BomViewer::Render()
{
    static bool isDeleteOpen = false;
    static bool isEditPending = false;

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.39f, 0.39f, 0.39f, 0.5859375f));
    DB::BOM::Refresh();

    if (isAddOpen == true)
    {
        RenderAddWindow();
    }
    if (isEditOpen == true)
    {
        RenderEditWindow();
    }

    ImGui::Columns(3, nullptr, false);
    const static float w1 = ImGui::GetColumnWidth(-1) * 0.75f;
    ImGui::SetColumnWidth(-1, w1);

#pragma region Control Buttons
    // Only show buttons if user has admin rights.
    if (DB::HasUserWritePrivileges())
    {
        if (ImGui::Button("Add"))
        {
            isAddOpen = true;
            MakeNewPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Edit"))
        {
            isEditPending = !isEditPending;
            isDeleteOpen = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Delete"))
        {
            isDeleteOpen = !isDeleteOpen;
            isEditOpen = false;
        }
    }
#pragma endregion

    ImGui::NextColumn();
    const static float w2 = ImGui::GetColumnWidth(-1) * 1.5f;
    ImGui::SetColumnWidth(-1, w2);
    filter.Render();
    ImGui::NextColumn();
    if (ImGui::Button("Export"))
    {
        ExportItems();
    }
    ImGui::NextColumn();
    ImGui::Columns(1);


    ImGui::BeginChildFrame(ImGui::GetID("BomViewerChildFrame"), ImVec2());
    ImGui::PopStyleColor();

    static SortBy sort = SortBy::id;

#pragma region Header
    ImGui::Columns(4);
#pragma region ID
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4());
    ImGui::BeginChildFrame(ImGui::GetID("##BomIdChildFrame"), ImVec2(0, ImGui::GetFrameHeight()));
    ImGui::PopStyleColor();
    ImGui::Columns(2, nullptr, false);
    if (ImGui::Selectable("ID", IS_SORT_ACTIVE(id), ImGuiSelectableFlags_SpanAllColumns))
    {
        sort = sort == SortBy::id ? SortBy::rid : SortBy::id;
    }
    ImGui::NextColumn();
    ImGui::Text(SORT_TXT(id));
    ImGui::NextColumn();
    ImGui::Columns(1);
    ImGui::EndChildFrame();
    ImGui::NextColumn();
#pragma endregion ID

#pragma region Name
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4());
    ImGui::BeginChildFrame(ImGui::GetID("##BomNameChildFrame"), ImVec2(0, ImGui::GetFrameHeight()));
    ImGui::PopStyleColor();
    ImGui::Columns(2, nullptr, false);
    if (ImGui::Selectable("Name", IS_SORT_ACTIVE(name), ImGuiSelectableFlags_SpanAllColumns))
    {
        sort = sort == SortBy::name ? SortBy::rname : SortBy::name;
    }
    ImGui::NextColumn();
    ImGui::Text(SORT_TXT(name));
    ImGui::NextColumn();
    ImGui::Columns(1);
    ImGui::EndChildFrame();
    ImGui::NextColumn();
#pragma endregion Name

#pragma region Output
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4());
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4());
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4());
    ImGui::BeginChildFrame(ImGui::GetID("##BomOutputChildFrame"), ImVec2(0, ImGui::GetFrameHeight()));
    ImGui::Selectable("Output Item");
    ImGui::EndChildFrame();
    ImGui::NextColumn();
#pragma endregion Output

#pragma region Items
    ImGui::BeginChildFrame(ImGui::GetID("##BomItemsChildFrame"), ImVec2(0, ImGui::GetFrameHeight()));
    ImGui::Selectable("Items");
    ImGui::EndChildFrame();
    ImGui::PopStyleColor(3);
    ImGui::NextColumn();
#pragma endregion Items

#pragma endregion Header

    std::vector<DB::BOM::BOM> boms = DB::BOM::GetAll();
    SortItems(sort, boms);

#pragma region Content
    for (auto& bom : boms)
    {
        ImGui::Separator();
        if (isEditPending == true)
        {
            std::string l = "Edit##" + bom.GetId();
            if (ImGui::SmallButton(l.c_str()))
            {
                isEditOpen = true;
                isEditPending = false;
                tmpBom = bom;
                MakeEditPopup();
            }
            ImGui::SameLine();
        }
        else if (isDeleteOpen == true)
        {
            std::string l = "Delete##" + bom.GetId();
            if (ImGui::SmallButton(l.c_str()))
            {
                isDeleteOpen = false;
                tmpBom = bom;
                MakeDeletePopup();
            }
            ImGui::SameLine();
        }

        if (ImGui::Selectable(std::string(bom.GetId() + "##selectable").c_str(), false))
        {
            tmpBom = bom;
            MakeMakePopup();
        }
        ImGui::NextColumn();

        ImGui::Text(bom.GetName().c_str());
        ImGui::NextColumn();

        ImGui::Text(bom.GetRawOutput().GetId().c_str());
        ImGui::NextColumn();

        std::string l = "Click to view##" + bom.GetId();
        std::string p = l + "pop up";
        if (ImGui::Selectable(l.c_str()))
        {
            ImGui::OpenPopup(p.c_str());
        }
        if (ImGui::BeginPopup(p.c_str(), ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Dummy(ImVec2(400.f, 0.1f));
            std::vector<DB::BOM::ItemReference> its = bom.GetRawItems();
            ImGui::Columns(3);
            Fonts::Push("Bold");
            ImGui::Text("Item ID");
            ImGui::NextColumn();
            ImGui::Text("Description");
            ImGui::NextColumn();
            ImGui::Text("Qty Needed");
            Fonts::Pop();
            ImGui::NextColumn();

            for (auto& it : its)
            {
                ImGui::Separator();
                ImGui::Text(it.GetId().c_str());
                ImGui::NextColumn();
                std::string d = it.GetItem().GetDescription().substr(0, 15) + "...";
                ImGui::Text(d.c_str());
                ImGui::NextColumn();
                ImGui::Text("%0.2f", it.GetQuantity());
                ImGui::NextColumn();
            }
            ImGui::Columns(1);
            ImGui::Separator();

            ImGui::Text("Makes %0.2f %s", bom.GetRawOutput().GetQuantity(), bom.GetOutput().GetUnit());

            ImGui::EndPopup();
        }
        ImGui::NextColumn();
    }
#pragma endregion Content

    ImGui::EndChildFrame();
}

void RenderAddWindow()
{
    ImGui::SetNextWindowSize(ImVec2(800, 700), ImGuiCond_Appearing);
    if (ImGui::Begin("Add new BOM"))
    {

        HandlePopupNameInput();
        HandlePopupItemPickerInput();
        HandlePopupOutputPickerInput();

        if (ImGui::Button("Accept"))
        {
            SaveNewBom();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            CancelAction();
        }

        ImGui::End();
    }
}

void RenderEditWindow()
{
    ImGui::SetNextWindowSize(ImVec2(800, 700), ImGuiCond_Appearing);
    if (ImGui::Begin("Edit BOM"))
    {

        HandlePopupNameInput();
        HandlePopupItemPickerInput();
        HandlePopupOutputPickerInput();

        if (ImGui::Button("Accept"))
        {
            SaveEditedBom();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            CancelAction();
        }

        ImGui::End();
    }
}

static void SortItems(SortBy sort, std::vector<DB::BOM::BOM>& boms)
{
    switch (sort)
    {
        case SortBy::id:
            std::sort(boms.begin(), boms.end(), [](DB::BOM::BOM& a, DB::BOM::BOM& b)
                      {
                          std::string sa = a.GetId();
                          std::string sb = b.GetId();
                          boost::to_upper(sa);
                          boost::to_upper(sb);
                          return(sa.compare(sb.c_str()) <= 0 ? true : false);
                      });
            break;
        case SortBy::rid:
            std::sort(boms.begin(), boms.end(), [](DB::BOM::BOM& a, DB::BOM::BOM& b)
                      {
                          std::string sa = a.GetId();
                          std::string sb = b.GetId();
                          boost::to_upper(sa);
                          boost::to_upper(sb);
                          return(sa.compare(sb.c_str()) > 0 ? true : false);
                      });
            break;
        case SortBy::name:
            std::sort(boms.begin(), boms.end(), [](DB::BOM::BOM& a, DB::BOM::BOM& b)
                      {
                          std::string sa = a.GetName();
                          std::string sb = b.GetName();
                          boost::to_upper(sa);
                          boost::to_upper(sb);
                          return(sa.compare(sb.c_str()) <= 0 ? true : false);
                      });
            break;
        case SortBy::rname:
            std::sort(boms.begin(), boms.end(), [](DB::BOM::BOM& a, DB::BOM::BOM& b)
                      {
                          std::string sa = a.GetName();
                          std::string sb = b.GetName();
                          boost::to_upper(sa);
                          boost::to_upper(sb);
                          return(sa.compare(sb.c_str()) > 0 ? true : false);
                      });
            break;
        default:
            break;

    }
}

static void MakeNewPopup()
{
    std::vector<DB::Item::Item> items = DB::Item::GetAll();
    for (auto& i : items)
    {
        tmpItems.emplace_back(ItemRef(DB::BOM::ItemReference(i.GetId(), i.GetOid(), i.GetQuantity(), tmpItems.size()),
                                      false));
    }
    tmpSelectedOut = 0;
    tmpQuantityMade = 0;
    itemFilter.ClearText();
}

static void MakeEditPopup(bool isRetry)
{
    std::vector<DB::Item::Item> items = DB::Item::GetAll();
    for (auto& i : items)
    {
        tmpItems.emplace_back(ItemRef(DB::BOM::ItemReference(i.GetId(), i.GetOid(), i.GetQuantity(), tmpItems.size()),
                                      false));
        auto it = std::find_if(tmpBom.GetRawItems().begin(),
                               tmpBom.GetRawItems().end(),
                               [i](const DB::BOM::ItemReference& obj)
                               {
                                   return obj.GetId() == i.GetId();
                               });

        if (it != tmpBom.GetRawItems().end())
        {
            tmpItems.back().isSelected = true;
            tmpItems.back().quantity = it->GetQuantity();
            tmpItems.back().reference.SetPosition(it->GetPosition());
        }
    }
    tmpOutput = tmpBom.GetRawOutput();
    auto it = std::find_if(tmpItems.begin(), tmpItems.end(),
                           [](ItemRef& i)
                           {
                               return tmpOutput.GetId() == i.reference.GetId();
                           });
    if (it != tmpItems.end())
    {
        tmpSelectedOut = int(std::distance(tmpItems.begin(), it));    // Get the index of tmpOutput in tmpItems.
    }
    else
    {
        tmpSelectedOut = 0;
    }
    strcpy_s(tmpName, sizeof(tmpName), tmpBom.GetName().c_str());
    tmpQuantityMade = int(tmpBom.GetRawOutput().GetQuantity());
    itemFilter.ClearText();
}

static void MakeDeletePopup()
{
    Popup::Init("Delete BOM", false);
    Popup::AddCall(Popup::TextStylized, "Are you sure you want to delete this BOM?"
                   "\nThis action cannot be undone", "Bold", true);
    Popup::AddCall(Popup::Button, "Yes", DeleteBom, true);
    Popup::AddCall(Popup::SameLine);
    Popup::AddCall(Popup::Button, "No", CancelAction, true);
}

static void CancelAction()
{
    tmpBom = DB::BOM::BOM();
    memset(tmpName, 0, sizeof(tmpName));
    tmpItems.clear();
    tmpOutput = DB::BOM::ItemReference();
    tmpQuantityMade = 0;
    tmpQuantityToMake = 0;

    isEditOpen = false;
    isAddOpen = false;
}

void MakeMakePopup()
{
    tmpQuantityToMake = 1;
    Popup::Init("Make BOM", false);
    Popup::AddCall(HandlePopupMake);
    Popup::AddCall(HandlePopupMakeButton, "Make", CommitMake, true);
    Popup::AddCall(Popup::SameLine);
    Popup::AddCall(HandlePopupMakeButton, "Cancel", CancelAction, true);
    Popup::AddCall(Popup::SameLine);
    Popup::AddCall(HandlePopupMakeButton, "Export", ExportItems, true);
}

void SaveNewBom()
{
    std::string id = DB::BOM::GetNewId();
    std::string name = tmpName;
    std::vector<DB::BOM::ItemReference> items;
    for (auto& item : tmpItems)
    {
        if (item.isSelected == true)
        {
            item.reference.SetQuantity(item.quantity);
            items.emplace_back(item.reference);
        }
    }

    tmpItems.at(tmpSelectedOut).reference.SetQuantity(float(tmpQuantityMade));
    tmpOutput = tmpItems.at(tmpSelectedOut).reference;

    DB::BOM::AddBom(DB::BOM::BOM(id, name, items, tmpOutput));
    CancelAction();
}

void SaveEditedBom()
{
    std::string id = DB::BOM::GetNewId();
    std::string name = tmpName;
    std::vector<DB::BOM::ItemReference> items;
    for (auto& item : tmpItems)
    {
        if (item.isSelected == true)
        {
            item.reference.SetQuantity(item.quantity);
            items.emplace_back(item.reference);
        }
    }

    ItemRef newOut = tmpItems.at(tmpSelectedOut);
    newOut.reference.SetQuantity(float(tmpQuantityMade));
    tmpOutput = newOut.reference;

    DB::BOM::EditBom(tmpBom, DB::BOM::BOM(tmpBom.GetId(), name, items, tmpOutput));
    CancelAction();
}

void DeleteBom()
{
    DB::BOM::DeleteBom(tmpBom);
    CancelAction();
}

void CommitMake()
{
    if (DB::HasUserWritePrivileges() == false)
    {
        Popup::Init("Unauthorized");
        Popup::AddCall(Popup::TextStylized, "You must be logged in to do this action", "Bold/4278190335", true);

        CancelAction();
        return;
    }
    DB::Item::Item newIt = tmpBom.GetOutput();
    newIt.IncQuantity();
    DB::Item::EditItem(tmpBom.GetOutput(), newIt);

    for (auto& item : tmpBom.GetRawItems())
    {
        newIt = DB::Item::GetItemByID(item.GetId());
        newIt.DecQuantity(item.GetQuantity());
        DB::Item::EditItem(DB::Item::GetItemByID(item.GetId()), newIt);
    }

    CancelAction();
}

void HandlePopupItemPickerInput()
{
    static bool shouldOnlyShowSelected = false;
    static std::string itemClicked = "";
    ImGui::Text("Items to use:");
    itemFilter.Render();
    ImGui::Checkbox("Only show selected items", &shouldOnlyShowSelected);

    ImVec2 size = ImVec2(0, ImGui::GetWindowHeight() - 250);
    ImGui::BeginChildFrame(ImGui::GetID("ItemPickerChildFrame"), size);
    float availWidth = ImGui::GetWindowContentRegionWidth();

    std::sort(tmpItems.begin(), tmpItems.end(), [](ItemRef& a, ItemRef& b)
              {
                  return a.reference.GetPosition() < b.reference.GetPosition();
              });

    ImGui::Columns(3);
    ImGui::Text("Item ID");
    ImGui::NextColumn();
    ImGui::Text("Available");
    ImGui::NextColumn();
    ImGui::Text("Quantity");
    ImGui::NextColumn();
    for (auto& item : tmpItems)
    {
        // Only display the item if it matches the filter or 
        // if the "Only show selected items" checkbox is active (shouldOnlyShowSelected == true),
        // only display the item if it is included in the BOM.
        if ((itemFilter.CheckMatch(DB::Item::GetItemByID(item.reference.GetId())) == false &&
             itemFilter.CheckMatch(item.reference.GetItem(), 1) == false) ||
             (shouldOnlyShowSelected == true && item.isSelected == false))
        {
            continue;
        }
        ImGui::Separator();
        ImVec2 checkboxPos1 = ImGui::GetCursorScreenPos();
        ImVec2 checkboxPos2 = ImVec2(checkboxPos1.x + ImGui::GetFrameHeight(), checkboxPos1.y + ImGui::GetFrameHeight());

        ImGui::Checkbox(item.reference.GetId().c_str(), &item.isSelected);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) &&
            ImGui::IsMouseHoveringRect(checkboxPos1, checkboxPos2) == false)
        {
            ImGui::OpenPopup(std::string("##ItemDescriptionPopup" + item.reference.GetId()).c_str());
        }

        if (ImGui::BeginPopup(std::string("##ItemDescriptionPopup" + item.reference.GetId()).c_str()))
        {
            ImVec2 popupPos = ImGui::GetMousePosOnOpeningCurrentPopup();
            ImVec2 cursorPos = ImGui::GetMousePos();
            if (cursorPos.x > popupPos.x + 100 || cursorPos.x < popupPos.x - 100 ||
                cursorPos.y > popupPos.y + 20 || cursorPos.y < popupPos.y - 20 ||
                ImGui::IsMouseHoveringRect(checkboxPos1, checkboxPos2, false) == true)
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::Text(item.reference.GetItem().GetDescription().c_str());

            ImGui::EndPopup();
        }

        // Check if current row is clicked.
        // Set highlighting box to start at top-left point of item frame.
        checkboxPos1.x -= ImGui::GetStyle().ItemSpacing.x;
        checkboxPos1.y -= ImGui::GetStyle().FramePadding.y;

        // Set highlighting box to end at bottom-right of item frame.
        ImVec2 endOfFrame = ImVec2(availWidth + checkboxPos1.x + ImGui::GetStyle().ItemSpacing.x,
                                   checkboxPos1.y + ImGui::GetFrameHeightWithSpacing() +
                                   ImGui::GetStyle().FramePadding.y + ImGui::GetStyle().ItemSpacing.y + 5);
        if ((ImGui::IsMouseHoveringRect(checkboxPos1, endOfFrame, false) && itemClicked.empty()) &&
            ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            itemClicked = item.reference.GetId();
        }

        // If this item is the one we clicked on:
        if (item.reference.GetId() == itemClicked)
        {
            // If the left mouse button has been released:
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left) == false)
            {
                // Drop the item.
                itemClicked = "";
            }
            else
            {
                // Check if mouse is above the item.
                if (ImGui::GetMousePos().y < checkboxPos1.y)
                {
                    item.reference.SetPosition(item.reference.GetPosition() - 1);
                }
                else if (ImGui::GetMousePos().y > endOfFrame.y)
                {
                    item.reference.SetPosition(item.reference.GetPosition() + 1);
                }
                ImGui::GetForegroundDrawList()->AddRectFilled(checkboxPos1, endOfFrame,
                                                              ImGui::GetColorU32(ImGuiCol_ButtonActive, 0.3f));
            }
        }

        ImGui::NextColumn();
        float available = DB::Item::GetItemByID(item.reference.GetId()).GetQuantity();
        ImGui::Text("%0.2f %s", available, DB::Item::GetItemByID(item.reference.GetId()).GetUnit());
        ImGui::NextColumn();
        ImGui::BeginChildFrame(ImGui::GetID(std::string("##QtyChildFrame" + item.reference.GetId()).c_str()),
                               ImVec2(0, ImGui::GetFrameHeightWithSpacing() + 5));
        ImU32 col = ImGui::GetColorU32(ImGuiCol_Text);
        if (available < item.quantity)
        {
            col = 0xFF0000FF;   // Red.
        }
        ImGui::PushStyleColor(ImGuiCol_Text, col);
        ImGui::InputFloat(std::string("##QtyInputFloat" + item.reference.GetId()).c_str(), &item.quantity, 1.0f, 10.0f);
        ImGui::PopStyleColor();
        ImGui::EndChildFrame();
        ImGui::NextColumn();
    }
    ImGui::EndChildFrame();

}

void HandlePopupOutputPickerInput()
{
    std::vector<const char*> ids;
    for (auto& item : tmpItems)
    {
        ids.emplace_back(item.reference.GetId().c_str());
    }

    ImGui::Combo("Created Item", &tmpSelectedOut, ids.data(), int(ids.size()));
    ImGui::InputInt("Quantity of items made", &tmpQuantityMade);
    if (tmpQuantityMade < 1)
    {
        tmpQuantityMade = 1;
    }
}

void HandlePopupMake()
{
    std::string txt = "Make " + StringUtils::NumToString(tmpBom.GetRawOutput().GetQuantity()) + " " +
        tmpBom.GetName().substr(0, 10) + "?";
    std::string bold = "Bold";
    Popup::TextStylized(txt, bold, true);
    ImGui::Text("Items needed to make product:");
    ImGui::Columns(3);
    ImGui::Text("Item ID");
    ImGui::NextColumn();
    ImGui::Text("Available");
    ImGui::NextColumn();
    ImGui::Text("Needed");
    ImGui::NextColumn();

    // Reset isMakeValid for this frame.
    isMakeValid = true;
    for (auto& i : tmpBom.GetRawItems())
    {
        ImGui::Separator();
        float avail = DB::Item::GetItemByID(i.GetId()).GetQuantity();
        float needed = i.GetQuantity() * tmpQuantityToMake;
        ImU32 col = avail < needed ? 0xFF0000FF : ImGui::GetColorU32(ImGuiCol_Text);

        // Set isMakeValid to false if we need more than available, else leave it as it is.
        isMakeValid = (avail < needed) || isMakeValid == false ? false : true;

        ImGui::Text(i.GetId().c_str());
        ImGui::NextColumn();
        ImGui::Text("%0.2f %s", avail, DB::Item::GetItemByID(i.GetId()).GetUnit().c_str());
        ImGui::NextColumn();
        ImGui::BeginChildFrame(ImGui::GetID(std::string("##NeededField" + i.GetId()).c_str()),
                               ImVec2(0, ImGui::GetFrameHeight()));
        ImGui::PushStyleColor(ImGuiCol_Text, col);
        ImGui::Text("%0.2f %s", needed, DB::Item::GetItemByID(i.GetId()).GetUnit().c_str());
        ImGui::PopStyleColor();
        ImGui::EndChildFrame();
        ImGui::NextColumn();
    }
    ImGui::Columns(1);

    ImGui::InputInt("Quantity to make", &tmpQuantityToMake, 1, 10);
    tmpQuantityToMake = tmpQuantityToMake <= 0 ? 1 : tmpQuantityToMake;
}

bool HandlePopupMakeButton(const std::string& label)
{
    std::string l = label;
    bool needToPop = false;
    bool r = false;
    if (isMakeValid == false && l.find("Make") != std::string::npos)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_TextDisabled));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetColorU32(ImGuiCol_TextDisabled));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetColorU32(ImGuiCol_TextDisabled));
        needToPop = true;
    }
    if (l == "Make")
    {
        l += " (" + StringUtils::NumToString(tmpQuantityToMake * int(tmpBom.GetRawOutput().GetQuantity()), false) + ")";
    }
    if (ImGui::Button(l.c_str()) && (l.find("Make") == std::string::npos || isMakeValid == true))
    {
        r = true;
    }

    if (needToPop)
    {
        ImGui::PopStyleColor(3);
    }

    return r;
}

static void HandlePopupNameInput()
{
    ImGui::InputText("Description", tmpName, sizeof(tmpName));
}

void ExportItems()
{
    // Form a list of all desired items.
    std::vector<DB::BOM::BOM> items = DB::BOM::GetAll();
    for (auto item = items.begin(); item != items.end();)
    {
        // We don't `item++` in the loop because it is virtually done by `items.erase`.
        if (filter.CheckMatch(*item) == false)
        {
            items.erase(item);
        }
        else
        {
            // We want to increment here because the item wasn't removed from the list.
            item++;
        }
    }

    // Make the user select the desired output file.
    tmpOutputFilePath = L"";
    File::SaveFile(tmpOutputFilePath, INDEX_CSV, L"*.csv");

    std::ofstream outputFile;
    outputFile.open(tmpOutputFilePath);

    if (outputFile.is_open() == false)
    {
        Logging::System.Error("An error occurred when opening the output file.");
        return;
    }

    // Write the headers.
    outputFile << "Id,Name,Output Item Id, Items" << std::endl;

    // Write the items.
    for (auto& item : items)
    {
        outputFile << item.GetId() << ","
            << "\"" << item.GetName() << "\","
            << item.GetRawOutput().GetId() << ",\"";
        for (auto& i : item.GetRawItems())
        {
            outputFile << i.GetId() << ":\t" << i.GetQuantity() * tmpQuantityToMake << " " << DB::Item::GetItemByID(i.GetId()).GetUnit() << std::endl;
        }

        outputFile << "\"" << std::endl;
    }

    outputFile.close();

    Popup::Init("File Generated");
    Popup::AddCall(Popup::TextCentered, "Do you want to open it?");
    Popup::AddCall(Popup::Button, "Yes", OpenOutputFile, true);
    Popup::AddCall(Popup::SameLine);
    Popup::AddCall(Popup::Button, "No", []()
                   {}, true);
}

void OpenOutputFile()
{
    ShellExecute(nullptr, nullptr, StringUtils::LongStringToString(tmpOutputFilePath).c_str(), nullptr, nullptr, SW_SHOW);
}
