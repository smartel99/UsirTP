#include "ItemViewer.h"
#include "boost/algorithm/string.hpp"
#include "utils/db/MongoCore.h"
#include "utils/db/Category.h"
#include "utils/db/Item.h"
#include "utils/Document.h"
#include "utils/Config.h"
#include "utils/FilterUtils.h"
#include "utils/StringUtils.h"
#include "vendor/imgui/imgui.h"
#include "widgets/Popup.h"
#include "widgets/Logger.h"

#include <fstream>
#include <Windows.h>
#include <vector>
#include <algorithm>

#define SORT_ASCEND     "(A to Z)"
#define SORT_DESCEND    "(Z to A)"
#define SORT_TXT(x)     (sortby == SortBy::x ? SORT_ASCEND : (sortby == SortBy::r ## x ? SORT_DESCEND : " "))
#define IS_SORT_ACTIVE(x) (( sortby == SortBy::x ) || ( sortby == SortBy::r ## x ))

enum class SortBy
{
    id = 0,
    rid,
    description,
    rdescription,
    category,
    rcategory,
    referenceLink,
    rreferenceLink,
    location,
    rlocation,
    price,
    rprice,
    quantity,
    rquantity,
    unit,
    runit,
    status,
    rstatus
};


static void SortItems(SortBy sortby, std::vector<DB::Item::Item>& items);
static void RenderFilterBar();
static bool CheckDoesItemMatchFilter(const DB::Item::Item& item);
static void MakeNewPopup();
static void MakeEditPopup(bool isRetry = false);
static void MakeDeletePopup();
static void SaveNewItem();
static void SaveEditedItem();
static void DeleteItem();
static void CancelAction();

static void HandlePopupIdInput();
static void HandlePopupIdInputDisabled();
static void HandlePopupDescriptionInput();
static void HandlePopupCategoryInput();
static void HandlePopupReferenceLinkInput();
static void HandlePopupLocationInput();
static void HandlePopupPriceInput();
static void HandlePopupQuantityInput();
static void HandlePopupUnitInput();
static void HandlePopupStatusInput();

static int GetCategoryNumber(const DB::Category::Category& cat);
static bool VerifyItem(DB::Item::Item& item);
static void ExportItems();
static void OpenOutputFile();

static DB::Item::Item tmpItem;
static std::vector<DB::Category::Category> categories;
static bool tmpAutoId = true;
static int tmpId = 0;
static char tmpIdStr[5] = { 0 };
static char tmpDesc[MAX_INPUT_LENGTH] = { 0 };
static int tmpCat = 0;
static char tmpRefLink[MAX_INPUT_LENGTH] = { 0 };
static char tmpLoc[MAX_INPUT_LENGTH] = { 0 };
static float tmpPrice = 0.00f;
static float tmpQty = 0.0f;
static char tmpUnit[MAX_INPUT_LENGTH] = { 0 };
static int tmpStatus = 0;
static std::wstring tmpOutputFilePath = L"";

const static std::vector<std::string> cats = { "ID",
                                               "Description",
                                               "Category",
                                               "Reference",
                                               "Location",
                                               "Price",
                                               "Quantity",
                                               "Unit",
                                               "Status" };
static FilterUtils::FilterHandler filter(cats);

void ItemViewer::Render()
{
    static bool isEditOpen = false;
    static bool isDeleteOpen = false;

    DB::Item::Refresh();

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.39f, 0.39f, 0.39f, 0.5859375f));
    ImGui::Columns(3, nullptr, false);
    const static float w1 = ImGui::GetColumnWidth(-1) * 0.75f;
    ImGui::SetColumnWidth(-1, w1);

#pragma region Control Buttons
    // Only show buttons if user has admin rights.
    if (DB::HasUserWritePrivileges())
    {
        if (ImGui::Button("Add"))
        {
            MakeNewPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Edit"))
        {
            isEditOpen = !isEditOpen;
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

#pragma region Search bar/filters
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
#pragma endregion Search bar/filters

    ImGui::BeginChildFrame(ImGui::GetID("ViewerChildFrame"), ImVec2());
    ImGui::PopStyleColor();

    static SortBy sortby = SortBy::id;

#pragma region Header
    ImGui::Columns(9);
#pragma region ID
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4());
    ImGui::BeginChildFrame(ImGui::GetID("##IdChildFrame"), ImVec2(0, ImGui::GetFrameHeight()));
    ImGui::PopStyleColor();
    ImGui::Columns(2, nullptr, false);
    if (ImGui::Selectable("ID", IS_SORT_ACTIVE(id), ImGuiSelectableFlags_SpanAllColumns))
    {
        sortby = sortby == SortBy::id ? SortBy::rid : SortBy::id;
    }
    ImGui::NextColumn();
    ImGui::Text(SORT_TXT(id));
    ImGui::NextColumn();
    ImGui::Columns(1);
    ImGui::EndChildFrame();
    ImGui::NextColumn();
#pragma endregion

#pragma region Description
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4());
    ImGui::BeginChildFrame(ImGui::GetID("##DescChildFrame"), ImVec2(0, ImGui::GetFrameHeight()));
    ImGui::PopStyleColor();
    ImGui::Columns(2, nullptr, false);
    if (ImGui::Selectable("Description", IS_SORT_ACTIVE(description), ImGuiSelectableFlags_SpanAllColumns))
    {
        sortby = sortby == SortBy::description ? SortBy::rdescription : SortBy::description;
    }
    ImGui::NextColumn();
    ImGui::Text(SORT_TXT(description));
    ImGui::NextColumn();
    ImGui::Columns(1);
    ImGui::EndChildFrame();
    ImGui::NextColumn();
#pragma endregion

#pragma region Category
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4());
    ImGui::BeginChildFrame(ImGui::GetID("##CatChildFrame"), ImVec2(0, ImGui::GetFrameHeight()));
    ImGui::PopStyleColor();
    ImGui::Columns(2, nullptr, false);
    if (ImGui::Selectable("Category", IS_SORT_ACTIVE(category), ImGuiSelectableFlags_SpanAllColumns))
    {
        sortby = sortby == SortBy::category ? SortBy::rcategory : SortBy::category;
    }
    ImGui::NextColumn();
    ImGui::Text(SORT_TXT(category));
    ImGui::NextColumn();
    ImGui::Columns(1);
    ImGui::EndChildFrame();
    ImGui::NextColumn();
#pragma endregion

#pragma region Reference Link
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4());
    ImGui::BeginChildFrame(ImGui::GetID("##RefChildFrame"), ImVec2(0, ImGui::GetFrameHeight()));
    ImGui::PopStyleColor();
    ImGui::Columns(2, nullptr, false);
    if (ImGui::Selectable("Reference", IS_SORT_ACTIVE(referenceLink), ImGuiSelectableFlags_SpanAllColumns))
    {
        sortby = sortby == SortBy::referenceLink ? SortBy::rreferenceLink : SortBy::referenceLink;
    }
    ImGui::NextColumn();
    ImGui::Text(SORT_TXT(referenceLink));
    ImGui::NextColumn();
    ImGui::Columns(1);
    ImGui::EndChildFrame();
    ImGui::NextColumn();
#pragma endregion

#pragma region Location
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4());
    ImGui::BeginChildFrame(ImGui::GetID("##LocChildFrame"), ImVec2(0, ImGui::GetFrameHeight()));
    ImGui::PopStyleColor();
    ImGui::Columns(2, nullptr, false);
    if (ImGui::Selectable("Location", IS_SORT_ACTIVE(location), ImGuiSelectableFlags_SpanAllColumns))
    {
        sortby = sortby == SortBy::location ? SortBy::rlocation : SortBy::location;
    }
    ImGui::NextColumn();
    ImGui::Text(SORT_TXT(location));
    ImGui::NextColumn();
    ImGui::Columns(1);
    ImGui::EndChildFrame();
    ImGui::NextColumn();
#pragma endregion

#pragma region Price
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4());
    ImGui::BeginChildFrame(ImGui::GetID("##PriChildFrame"), ImVec2(0, ImGui::GetFrameHeight()));
    ImGui::PopStyleColor();
    ImGui::Columns(2, nullptr, false);
    if (ImGui::Selectable("Price/Unit", IS_SORT_ACTIVE(price), ImGuiSelectableFlags_SpanAllColumns))
    {
        sortby = sortby == SortBy::price ? SortBy::rprice : SortBy::price;
    }
    ImGui::NextColumn();
    ImGui::Text(SORT_TXT(price));
    ImGui::NextColumn();
    ImGui::Columns(1);
    ImGui::EndChildFrame();
    ImGui::NextColumn();
#pragma endregion

#pragma region Quantity
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4());
    ImGui::BeginChildFrame(ImGui::GetID("##QtyChildFrame"), ImVec2(0, ImGui::GetFrameHeight()));
    ImGui::PopStyleColor();
    ImGui::Columns(2, nullptr, false);
    if (ImGui::Selectable("Quantity", IS_SORT_ACTIVE(quantity), ImGuiSelectableFlags_SpanAllColumns))
    {
        sortby = sortby == SortBy::quantity ? SortBy::rquantity : SortBy::quantity;
    }
    ImGui::NextColumn();
    ImGui::Text(SORT_TXT(quantity));
    ImGui::NextColumn();
    ImGui::Columns(1);
    ImGui::EndChildFrame();
    ImGui::NextColumn();
#pragma endregion

#pragma region Unit
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4());
    ImGui::BeginChildFrame(ImGui::GetID("##UniChildFrame"), ImVec2(0, ImGui::GetFrameHeight()));
    ImGui::PopStyleColor();
    ImGui::Columns(2, nullptr, false);
    if (ImGui::Selectable("Unit", IS_SORT_ACTIVE(unit), ImGuiSelectableFlags_SpanAllColumns))
    {
        sortby = sortby == SortBy::unit ? SortBy::runit : SortBy::unit;
    }
    ImGui::NextColumn();
    ImGui::Text(SORT_TXT(unit));
    ImGui::NextColumn();
    ImGui::Columns(1);
    ImGui::EndChildFrame();
    ImGui::NextColumn();
#pragma endregion

#pragma region Status
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4());
    ImGui::BeginChildFrame(ImGui::GetID("##StaChildFrame"), ImVec2(0, ImGui::GetFrameHeight()));
    ImGui::PopStyleColor();
    ImGui::Columns(2, nullptr, false);
    if (ImGui::Selectable("Status", IS_SORT_ACTIVE(status), ImGuiSelectableFlags_SpanAllColumns))
    {
        sortby = sortby == SortBy::status ? SortBy::rstatus : SortBy::status;
    }
    ImGui::NextColumn();
    ImGui::Text(SORT_TXT(status));
    ImGui::NextColumn();
    ImGui::Columns(1);
    ImGui::EndChildFrame();
    ImGui::NextColumn();
#pragma endregion

#pragma endregion

    std::vector<DB::Item::Item> items = DB::Item::GetAll();
    SortItems(sortby, items);
#pragma region Content
    for (auto& item : items)
    {
        if (filter.CheckMatch(item) == false)
        {
            continue;
        }
        ImGui::Separator();
        if (isEditOpen == true)
        {
            std::string l = "Edit##" + item.GetId();
            if (ImGui::SmallButton(l.c_str()))
            {
                isEditOpen = false;
                tmpItem = DB::Item::Item(item);
                MakeEditPopup();
            }
            ImGui::SameLine();
        }
        else if (isDeleteOpen == true)
        {
            std::string l = "Delete##" + item.GetId();
            if (ImGui::SmallButton(l.c_str()))
            {
                isDeleteOpen = false;
                tmpItem = DB::Item::Item(item);
                MakeDeletePopup();
            }
            ImGui::SameLine();
        }

        ImGui::Text(item.GetId().c_str());
        ImGui::NextColumn();

        ImGui::Text(item.GetDescription().c_str());
        ImGui::NextColumn();

        DB::Category::Category cat = DB::Category::GetCategoryByName(item.GetCategory().GetName());
        ImGui::Text(cat.GetName().c_str());
        ImGui::NextColumn();

        if (StringUtils::StringIsValidUrl(item.GetReferenceLink()))
        {
            if (ImGui::SmallButton(std::string("Link##" + item.GetId()).c_str()))
            {
                ShellExecute(nullptr, nullptr, item.GetReferenceLink().c_str(), nullptr, nullptr, SW_SHOW);
            }
        }
        else
        {
            ImGui::Text(item.GetReferenceLink().c_str());
        }
        ImGui::NextColumn();

        ImGui::Text(item.GetLocation().c_str());
        ImGui::NextColumn();

        ImGui::Text("%0.3f $CDN", item.GetPrice());
        ImGui::NextColumn();

        {
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4());
            ImGui::BeginChildFrame(ImGui::GetID(std::string("Qty" + item.GetId()).c_str()),
                                   ImVec2(0, ImGui::GetFrameHeight()));
            ImGui::PopStyleColor();
            ImGui::Columns(2, nullptr, false);
            ImGui::Text("%0.2f", item.GetQuantity());
            ImGui::NextColumn();
            if (ImGui::SmallButton("Set"))
            {
                if (DB::HasUserWritePrivileges() == false)
                {
                    Popup::Init("Unauthorized");
                    Popup::AddCall(Popup::TextStylized, "You must be logged in to do this action", "Bold/4278190335", true);
                }
                else
                {
                    ImGui::OpenPopup(std::string("QtySet##" + item.GetId()).c_str());
                }
            }
            ImGui::NextColumn();
            ImGui::Columns(1);

            if (ImGui::BeginPopup(std::string("QtySet##" + item.GetId()).c_str()))
            {
                static float incVal = 1.0f;
                if (ImGui::SmallButton(std::string("+##" + item.GetId()).c_str()))
                {
                    DB::Item::Item tmp = item;
                    tmp.SetQuantity(tmp.GetQuantity() + incVal);
                    DB::Item::EditItem(item, tmp);
                }
                ImGui::SameLine();
                if (ImGui::SmallButton(std::string("-##" + item.GetId()).c_str()))
                {
                    DB::Item::Item tmp = item;
                    tmp.SetQuantity(tmp.GetQuantity() - incVal);
                    DB::Item::EditItem(item, tmp);
                }
                ImGui::InputFloat("Step Size", &incVal, 0.01f, 1.0f, 2);
                ImGui::EndPopup();
            }
            ImGui::EndChildFrame();
        }
        ImGui::NextColumn();

        ImGui::Text(item.GetUnit().c_str());
        ImGui::NextColumn();

        ImGui::Text(item.GetStatusAsString().c_str());
        ImGui::NextColumn();
    }

    ImGui::Columns(1);
    ImGui::Separator();
#pragma endregion

    ImGui::EndChildFrame();
}

void SortItems(SortBy sortby, std::vector<DB::Item::Item>& items)
{
    switch (sortby)
    {
#pragma region Sort Ascend
        case SortBy::id:
            std::sort(items.begin(), items.end(),
                      [](DB::Item::Item& a, DB::Item::Item& b)
                      {
                          std::string sa = a.GetId();
                          std::string sb = b.GetId();
                          boost::to_upper(sa);
                          boost::to_upper(sb);
                          return(sa.compare(sb.c_str()) <= 0 ? true : false);
                      });
            break;
        case SortBy::description:
            std::sort(items.begin(), items.end(),
                      [](DB::Item::Item& a, DB::Item::Item& b)
                      {
                          std::string sa = a.GetDescription();
                          std::string sb = b.GetDescription();
                          boost::to_upper(sa);
                          boost::to_upper(sb);
                          return(sa.compare(sb.c_str()) <= 0 ? true : false);
                      });
            break;
        case SortBy::category:
            std::sort(items.begin(), items.end(),
                      [](DB::Item::Item& a, DB::Item::Item& b)
                      {
                          std::string sa = a.GetCategory().GetName();
                          std::string sb = b.GetCategory().GetName();
                          boost::to_upper(sa);
                          boost::to_upper(sb);
                          return(sa.compare(sb.c_str()) <= 0 ? true : false);
                      });
            break;
        case SortBy::referenceLink:
            std::sort(items.begin(), items.end(),
                      [](DB::Item::Item& a, DB::Item::Item& b)
                      {
                          std::string sa = a.GetReferenceLink();
                          std::string sb = b.GetReferenceLink();
                          boost::to_upper(sa);
                          boost::to_upper(sb);
                          return(sa.compare(sb.c_str()) <= 0 ? true : false);
                      });
            break;
        case SortBy::location:
            std::sort(items.begin(), items.end(),
                      [](DB::Item::Item& a, DB::Item::Item& b)
                      {
                          std::string sa = a.GetLocation();
                          std::string sb = b.GetLocation();
                          boost::to_upper(sa);
                          boost::to_upper(sb);
                          return(sa.compare(sb.c_str()) <= 0 ? true : false);
                      });
            break;
        case SortBy::price:
            std::sort(items.begin(), items.end(),
                      [](DB::Item::Item& a, DB::Item::Item& b)
                      {
                          float sa = a.GetPrice();
                          float sb = b.GetPrice();
                          return(sa <= sb ? true : false);
                      });
            break;
        case SortBy::quantity:
            std::sort(items.begin(), items.end(),
                      [](DB::Item::Item& a, DB::Item::Item& b)
                      {
                          float sa = a.GetQuantity();
                          float sb = b.GetQuantity();
                          return(sa <= sb ? true : false);
                      });
            break;
        case SortBy::unit:
            std::sort(items.begin(), items.end(),
                      [](DB::Item::Item& a, DB::Item::Item& b)
                      {
                          std::string sa = a.GetUnit();
                          std::string sb = b.GetUnit();
                          boost::to_upper(sa);
                          boost::to_upper(sb);
                          return(sa.compare(sb.c_str()) <= 0 ? true : false);
                      });
            break;
        case SortBy::status:
            std::sort(items.begin(), items.end(),
                      [](DB::Item::Item& a, DB::Item::Item& b)
                      {
                          int sa = a.GetStatus();
                          int sb = b.GetStatus();
                          return(sa <= sb ? true : false);
                      });
            break;
#pragma endregion

#pragma region Sort Descend
        case SortBy::rid:
            std::sort(items.begin(), items.end(),
                      [](DB::Item::Item& a, DB::Item::Item& b)
                      {
                          std::string sa = a.GetId();
                          std::string sb = b.GetId();
                          boost::to_upper(sa);
                          boost::to_upper(sb);
                          return(sa.compare(sb.c_str()) > 0 ? true : false);
                      });
            break;
        case SortBy::rdescription:
            std::sort(items.begin(), items.end(),
                      [](DB::Item::Item& a, DB::Item::Item& b)
                      {
                          std::string sa = a.GetDescription();
                          std::string sb = b.GetDescription();
                          boost::to_upper(sa);
                          boost::to_upper(sb);
                          return(sa.compare(sb.c_str()) > 0 ? true : false);
                      });
            break;
        case SortBy::rcategory:
            std::sort(items.begin(), items.end(),
                      [](DB::Item::Item& a, DB::Item::Item& b)
                      {
                          std::string sa = a.GetCategory().GetName();
                          std::string sb = b.GetCategory().GetName();
                          boost::to_upper(sa);
                          boost::to_upper(sb);
                          return(sa.compare(sb.c_str()) > 0 ? true : false);
                      });
            break;
        case SortBy::rreferenceLink:
            std::sort(items.begin(), items.end(),
                      [](DB::Item::Item& a, DB::Item::Item& b)
                      {
                          std::string sa = a.GetReferenceLink();
                          std::string sb = b.GetReferenceLink();
                          boost::to_upper(sa);
                          boost::to_upper(sb);
                          return(sa.compare(sb.c_str()) > 0 ? true : false);
                      });
            break;
        case SortBy::rlocation:
            std::sort(items.begin(), items.end(),
                      [](DB::Item::Item& a, DB::Item::Item& b)
                      {
                          std::string sa = a.GetLocation();
                          std::string sb = b.GetLocation();
                          boost::to_upper(sa);
                          boost::to_upper(sb);
                          return(sa.compare(sb.c_str()) > 0 ? true : false);
                      });
            break;
        case SortBy::rprice:
            std::sort(items.begin(), items.end(),
                      [](DB::Item::Item& a, DB::Item::Item& b)
                      {
                          float sa = a.GetPrice();
                          float sb = b.GetPrice();
                          return(sa > sb ? true : false);
                      });
            break;
        case SortBy::rquantity:
            std::sort(items.begin(), items.end(),
                      [](DB::Item::Item& a, DB::Item::Item& b)
                      {
                          float sa = a.GetQuantity();
                          float sb = b.GetQuantity();
                          return(sa > sb ? true : false);
                      });
            break;
        case SortBy::runit:
            std::sort(items.begin(), items.end(),
                      [](DB::Item::Item& a, DB::Item::Item& b)
                      {
                          std::string sa = a.GetUnit();
                          std::string sb = b.GetUnit();
                          boost::to_upper(sa);
                          boost::to_upper(sb);
                          return(sa.compare(sb.c_str()) > 0 ? true : false);
                      });
            break;
        case SortBy::rstatus:
            std::sort(items.begin(), items.end(),
                      [](DB::Item::Item& a, DB::Item::Item& b)
                      {
                          int sa = a.GetStatus();
                          int sb = b.GetStatus();
                          return(sa > sb ? true : false);
                      });
            break;
#pragma endregion
        default:
            break;
    }
}

void RenderFilterBar()
{

}

bool CheckDoesItemMatchFilter(const DB::Item::Item& item)
{
    return false;
}

void MakeNewPopup()
{
    categories = DB::Category::GetAll();
    tmpAutoId = true;
    Popup::Init("New Item", false);
    Popup::AddCall(HandlePopupIdInput);
    Popup::AddCall(HandlePopupDescriptionInput);
    Popup::AddCall(HandlePopupCategoryInput);
    Popup::AddCall(HandlePopupReferenceLinkInput);
    Popup::AddCall(HandlePopupLocationInput);
    Popup::AddCall(HandlePopupPriceInput);
    Popup::AddCall(HandlePopupQuantityInput);
    Popup::AddCall(HandlePopupUnitInput);
    Popup::AddCall(HandlePopupStatusInput);
    Popup::AddCall(Popup::Button, "Accept", SaveNewItem, true);
    Popup::AddCall(Popup::SameLine);
    Popup::AddCall(Popup::Button, "Cancel", CancelAction, true);
}


static void MakeEditPopup(bool isRetry)
{
    categories = DB::Category::GetAll();
    Popup::Init("Edit Item", false);

    tmpId = StringUtils::StringToNum<int>(tmpItem.GetId());
    tmpAutoId = false;

    strcpy_s(tmpDesc, sizeof(tmpDesc), tmpItem.GetDescription().c_str());
    tmpCat = GetCategoryNumber(tmpItem.GetCategory());
    strcpy_s(tmpRefLink, sizeof(tmpRefLink), tmpItem.GetReferenceLink().c_str());
    strcpy_s(tmpLoc, sizeof(tmpLoc), tmpItem.GetLocation().c_str());
    tmpPrice = tmpItem.GetPrice();
    tmpQty = tmpItem.GetQuantity();
    strcpy_s(tmpUnit, sizeof(tmpUnit), tmpItem.GetUnit().c_str());
    tmpStatus = tmpItem.GetStatus();

    if (isRetry == true)
    {
        std::string style = "Bold/4278190335"; // 4278190335 -> 0xFF0000FF -> Red.
        Popup::AddCall(Popup::TextStylized, "Some fields are invalid, please retry", style, true);
    }
    Popup::AddCall(HandlePopupIdInputDisabled);
    Popup::AddCall(HandlePopupDescriptionInput);
    Popup::AddCall(HandlePopupCategoryInput);
    Popup::AddCall(HandlePopupReferenceLinkInput);
    Popup::AddCall(HandlePopupLocationInput);
    Popup::AddCall(HandlePopupPriceInput);
    Popup::AddCall(HandlePopupQuantityInput);
    Popup::AddCall(HandlePopupUnitInput);
    Popup::AddCall(HandlePopupStatusInput);
    Popup::AddCall(Popup::Button, "Accept", SaveEditedItem, true);
    Popup::AddCall(Popup::SameLine);
    Popup::AddCall(Popup::Button, "Cancel", CancelAction, true);
}

static void MakeDeletePopup()
{
    categories = DB::Category::GetAll();
    Popup::Init("Delete Item", false);
    Popup::AddCall(Popup::TextStylized, "Are you sure you want to delete this category?"
                   "\nThis action cannot be undone", "Bold", true);
    Popup::AddCall(Popup::Button, "Yes", DeleteItem, true);
    Popup::AddCall(Popup::SameLine);
    Popup::AddCall(Popup::Button, "No", CancelAction);
}


void SaveNewItem()
{
    DB::Item::Item newItem;
    if (VerifyItem(newItem) == true)
    {
        if (DB::Item::AddItem(newItem) == false)
        {
            // If unable to add item to DB.
            Popup::Init("Add Item");
            Popup::AddCall(ImGui::Spacing);
            Popup::AddCall(Popup::TextCentered, "Unable to create item!");
        }

        // Clear all temporary fields.
        CancelAction();
    }
    else
    {
        tmpItem = newItem;
        MakeEditPopup(true);
    }
}

void SaveEditedItem()
{
    DB::Item::Item newItem;
    if (VerifyItem(newItem) == true)
    {
        if (DB::Item::EditItem(tmpItem, newItem) == false)
        {
            // If unable to add item to DB.
            Popup::Init("Add Item");
            Popup::AddCall(ImGui::Spacing);
            Popup::AddCall(Popup::TextCentered, "Unable to create item!");
        }

        // Clear all temporary fields.
        CancelAction();
    }
    else
    {
        tmpItem = newItem;
        MakeEditPopup(true);
    }
}

void DeleteItem()
{
    DB::Item::DeleteItem(tmpItem);
    CancelAction();
}

void CancelAction()
{
    tmpId = 0;
    memset(tmpIdStr, 0, sizeof(tmpIdStr));
    memset(tmpDesc, 0, sizeof(tmpDesc));
    tmpCat = 0;
    memset(tmpRefLink, 0, sizeof(tmpRefLink));
    memset(tmpLoc, 0, sizeof(tmpLoc));
    tmpPrice = 0.00f;
    tmpQty = 0.0f;
    memset(tmpUnit, 0, sizeof(tmpUnit));
    tmpStatus = 0;
    tmpItem = DB::Item::Item();
}

void HandlePopupIdInput()
{
    if (tmpAutoId == true)
    {
        tmpId = StringUtils::StringToNum<int>(DB::Item::GetNewId(categories.at(tmpCat)));
    }
    char idLabel[50] = { 0 };
    std::string i = DB::Item::GetNewId(categories.at(tmpCat), tmpId);
    strcpy_s(tmpIdStr, sizeof(tmpIdStr), StringUtils::NumToString(unsigned int(tmpId)).c_str());
    sprintf_s(idLabel, sizeof(idLabel), "ID Will be: %s", i.c_str());
    ImGui::InputText("ID", tmpIdStr, sizeof(tmpIdStr), ImGuiInputTextFlags_CharsDecimal);
    std::string t = tmpIdStr;
    tmpId = StringUtils::StringToNum<int>(t);
    ImGui::Columns(2, nullptr, false);
    ImGui::Text(idLabel);
    ImGui::NextColumn();
    ImGui::Checkbox("Auto ID", &tmpAutoId);
    ImGui::NextColumn();
    ImGui::Columns(1);
}

void HandlePopupIdInputDisabled()
{
    bool disabled = false;
    if (tmpAutoId == true)
    {
        tmpId = StringUtils::StringToNum<int>(DB::Item::GetNewId(categories.at(tmpCat)));
    }
    char idLabel[50] = { 0 };

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));

    //     _itoa_s(tmpId, tmpIdStr, sizeof(tmpIdStr), 10);
    strcpy_s(tmpIdStr, sizeof(tmpIdStr), StringUtils::NumToString(unsigned int(tmpId)).c_str());
    DB::Category::Category cat;
    try
    {
        cat = categories.at(tmpCat);
    }
    catch (const std::exception&)
    {
    }
    sprintf_s(idLabel, sizeof(idLabel), "ID Will be: %s", DB::Item::GetNewId(cat, tmpId).c_str());
    ImGui::InputText("ID", tmpIdStr, sizeof(tmpIdStr), ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_ReadOnly);
    std::string t = tmpIdStr;
    if (!t.empty())
    {
        tmpId = std::stoi(t);
    }
    else
    {
        tmpId = 0;
    }

    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    ImGui::Columns(2, nullptr, false);
    ImGui::TextDisabled(idLabel);
    ImGui::NextColumn();
    ImGui::Checkbox("Auto ID", &disabled);
    ImGui::NextColumn();
    ImGui::Columns(1);

    ImGui::PopStyleColor(5);
}

void HandlePopupDescriptionInput()
{
    ImGui::InputText("Description", tmpDesc, sizeof(tmpDesc));
}

void HandlePopupCategoryInput()
{
    std::vector<const char*> cats;
    for (auto& category : categories)
    {
        cats.emplace_back(category.GetName().c_str());
    }

    ImGui::Combo("Category", &tmpCat, cats.data(), int(cats.size()));
}

void HandlePopupReferenceLinkInput()
{
    ImGui::InputText("Reference Link", tmpRefLink, sizeof(tmpRefLink));
}

void HandlePopupLocationInput()
{
    ImGui::InputText("Location", tmpLoc, sizeof(tmpLoc));
}

void HandlePopupPriceInput()
{
    ImGui::InputFloat("Price ($CDN)", &tmpPrice, 0.0f, 0.0f, 2);
}


void HandlePopupQuantityInput()
{
    ImGui::InputFloat("Quantity", &tmpQty, 0.0f, 0.0f, 2);
}

void HandlePopupUnitInput()
{
    ImGui::InputText("Unit", tmpUnit, sizeof(tmpUnit));
}

void HandlePopupStatusInput()
{
    using namespace DB::Item;
    std::vector<const char*> statuses;
    statuses.emplace_back("Active");
    statuses.emplace_back("Obsolete");
    statuses.emplace_back("Not Recommended For New Design");

    ImGui::Combo("Status", &tmpStatus, statuses.data(), int(statuses.size()));
}

int GetCategoryNumber(const DB::Category::Category& cat)
{
    int i = 0;
    for (auto& c : categories)
    {
        if (c == cat)
        {
            return i;
        }
        i++;
    }
    return i;
}

bool VerifyItem(DB::Item::Item& item)
{
    using namespace DB;
    bool isAllGood = true;
    std::string id = DB::Item::GetNewId(categories.at(tmpCat), tmpId);

    if (id.size() != 8)
    {
        //         isAllGood = false;
    }

    std::string description = tmpDesc;

    Category::Category category;
    try
    {
        category = categories.at(tmpCat);
    }
    catch (std::out_of_range)
    {
        //         isAllGood = false;
    }

    std::string refLink = tmpRefLink;
    std::string location = tmpLoc;

    float price = tmpPrice;
    float qty = tmpQty;

    std::string unit = tmpUnit;
    if (unit.empty())
    {
        //         isAllGood = false;
    }

    auto status = Item::ItemStatus(tmpStatus);

    item = Item::Item("", id, description, category, refLink, location, price, qty, unit, status);

    return isAllGood;
}

void ExportItems()
{
    // Form a list of all desired items.
    std::vector<DB::Item::Item> items = DB::Item::GetAll();
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
    File::SaveFile(tmpOutputFilePath, FileType::INDEX_CSV, L"*.csv");

    std::ofstream outputFile;
    outputFile.open(tmpOutputFilePath);

    if (outputFile.is_open() == false)
    {
        Logging::System.Error("An error occurred when opening the output file.");
        return;
    }

    // Write the headers.
    outputFile << "Id,Description,Category,Reference Link,Location,Price,Quantity,Unit,Status" << std::endl;

    // Write the items.
    for (auto& item : items)
    {
        outputFile << item.GetId() << ","
            << "\"" << item.GetDescription() << "\","
            << item.GetCategory().GetName() << ","
            << item.GetReferenceLink() << ","
            << item.GetLocation() << ","
            << item.GetPrice() << ","
            << item.GetQuantity() << ","
            << item.GetUnit() << ","
            << item.GetStatusAsString() << std::endl;
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
