#include "Viewer.h"
#include "utils/db/MongoCore.h"
#include "utils/db/Category.h"
#include "utils/db/Item.h"
#include "utils/Config.h"
#include "utils/StringUtils.h"
#include "vendor/imgui/imgui.h"
#include "widgets/Popup.h"
#include "boost/algorithm/string.hpp"
#include <Windows.h>
#include <vector>
#include <algorithm>
#include <sstream>

#define MAX_INPUT_LENGHT 400

enum class SortBy
{
    id = 0,
    description,
    category,
    referenceLink,
    price,
    quantity,
    unit,
    status
};


static void SortItems(SortBy sortby, std::vector<DB::Item::Item>& items);
static void MakeNewPopup();
static void MakeEditPopup();
static void MakeDeletePopup();
static void SaveNewItem();
static void SaveEditedItem();
static void DeleteItem();
static void CancelAction();

static void HandlePopupIdInput();
static void HandlePopupDescriptionInput();
static void HandlePopupCategoryInput();
static void HandlePopupReferenceLinkInput();
static void HandlePopupPriceInput();
static void HandlePopupQuantityInput();
static void HandlePopupUnitInput();
static void HandlePopupStatusInput();

static int GetCategoryNumber(DB::Category::Category& cat);
static std::string FormId();

static DB::Item::Item tmpItem;
static std::vector<DB::Category::Category> categories;
static bool tmpAutoId = true;
static int tmpId = 0;
static char tmpIdStr[5] = { 0 };
static char tmpDesc[MAX_INPUT_LENGHT] = { 0 };
static int tmpCat = 0;
static char tmpRefLink[MAX_INPUT_LENGHT] = { 0 };
static float tmpPrice = 0.00f;
static float tmpQty = 0.0f;
static char tmpUnit[MAX_INPUT_LENGHT] = { 0 };
static int tmpStatus = 0;

void Viewer::Init()
{
    DB::Init(Config::GetField<std::string>("uri"));
    DB::Category::Init();
    DB::Item::Init();
}

void Viewer::Render()
{
    static bool isEditOpen = false;
    static bool isDeleteOpen = false;

    DB::Item::Refresh();
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Appearing);
    ImGui::Begin("Inventory Manager");

#pragma region Control Buttons
    // Only show buttons if user has admin rights.
    if (DB::IsUserAdmin())
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

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.39f, 0.39f, 0.39f, 0.5859375f));
    ImGui::BeginChildFrame(ImGui::GetID("ViewerChildFrame"), ImVec2());
    ImGui::PopStyleColor();

    static SortBy sortby = SortBy::id;

#pragma region Header
    ImGui::Columns(8);
    if (ImGui::Selectable("ID", sortby == SortBy::id))
    {
        sortby = SortBy::id;
    }
    ImGui::NextColumn();

    if (ImGui::Selectable("Description", sortby == SortBy::description))
    {
        sortby = SortBy::description;
    }
    ImGui::NextColumn();

    if (ImGui::Selectable("Category", sortby == SortBy::category))
    {
        sortby = SortBy::category;
    }
    ImGui::NextColumn();

    if (ImGui::Selectable("Reference Link", sortby == SortBy::referenceLink))
    {
        sortby = SortBy::referenceLink;
    }
    ImGui::NextColumn();

    if (ImGui::Selectable("Price per Unit", sortby == SortBy::price))
    {
        sortby = SortBy::price;
    }
    ImGui::NextColumn();

    if (ImGui::Selectable("Quantity", sortby == SortBy::quantity))
    {
        sortby = SortBy::quantity;
    }
    ImGui::NextColumn();

    if (ImGui::Selectable("Unit", sortby == SortBy::unit))
    {
        sortby = SortBy::unit;
    }
    ImGui::NextColumn();

    if (ImGui::Selectable("Status", sortby == SortBy::status))
    {
        sortby = SortBy::status;
    }
    ImGui::NextColumn();

#pragma endregion

    std::vector<DB::Item::Item> items = DB::Item::GetAll();
    SortItems(sortby, items);
#pragma region Content
    for (auto& item : items)
    {
        ImGui::Separator();
        if (isEditOpen == true)
        {
            std::string l = "Edit##" + item.GetId();
            if (ImGui::SmallButton(l.c_str()))
            {
                isEditOpen = false;
                tmpItem = item;
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
                tmpItem = item;
                MakeDeletePopup();
            }
            ImGui::SameLine();
        }

        ImGui::Text(item.GetId().c_str());
        ImGui::NextColumn();

        ImGui::Text(item.GetDescription().c_str());
        ImGui::NextColumn();

        ImGui::Text(item.GetCategory().GetName().c_str());
        ImGui::NextColumn();

        if (ImGui::Selectable(item.GetReferenceLink().c_str()))
        {
            ShellExecute(0, 0, item.GetReferenceLink().c_str(), 0, 0, SW_SHOW);
        }
        ImGui::NextColumn();

        ImGui::Text("%0.2f $CDN", item.GetPrice());
        ImGui::NextColumn();

        ImGui::Text("%f", item.GetQuantity());
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
    ImGui::End();
}

void SortItems(SortBy sortby, std::vector<DB::Item::Item>& items)
{
    switch (sortby)
    {
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
        default:
            break;
    }
}

void MakeNewPopup()
{
    categories = DB::Category::GetAll();
    Popup::Init("New Item", false);
    Popup::AddCall(HandlePopupIdInput);
    Popup::AddCall(HandlePopupDescriptionInput);
    Popup::AddCall(HandlePopupCategoryInput);
    Popup::AddCall(HandlePopupReferenceLinkInput);
    Popup::AddCall(HandlePopupPriceInput);
    Popup::AddCall(HandlePopupQuantityInput);
    Popup::AddCall(HandlePopupUnitInput);
    Popup::AddCall(HandlePopupStatusInput);
    Popup::AddCall(Popup::Button, "Accept", SaveNewItem);
    Popup::AddCall(Popup::SameLine);
    Popup::AddCall(Popup::Button, "Cancel", CancelAction);
}


static void MakeEditPopup()
{
    categories = DB::Category::GetAll();
    Popup::Init("Edit Item", false);
    tmpId = std::stoi(tmpItem.GetId());
    strcpy_s(tmpDesc, sizeof(tmpDesc), tmpItem.GetDescription().c_str());
    tmpCat = GetCategoryNumber(tmpItem.GetCategory());
    strcpy_s(tmpRefLink, sizeof(tmpRefLink), tmpItem.GetReferenceLink().c_str());
    tmpPrice = tmpItem.GetPrice();
    tmpQty = tmpItem.GetQuantity();
    strcpy_s(tmpUnit, sizeof(tmpUnit), tmpItem.GetUnit().c_str());
    tmpStatus = tmpItem.GetStatus();

    Popup::AddCall(HandlePopupIdInput);
    Popup::AddCall(HandlePopupDescriptionInput);
    Popup::AddCall(HandlePopupCategoryInput);
    Popup::AddCall(HandlePopupReferenceLinkInput);
    Popup::AddCall(HandlePopupPriceInput);
    Popup::AddCall(HandlePopupQuantityInput);
    Popup::AddCall(HandlePopupUnitInput);
    Popup::AddCall(HandlePopupStatusInput);
    Popup::AddCall(Popup::Button, "Accept", SaveEditedItem);
    Popup::AddCall(Popup::SameLine);
    Popup::AddCall(Popup::Button, "Cancel", CancelAction);
}

static void MakeDeletePopup()
{
    categories = DB::Category::GetAll();
    Popup::Init("Delete Item", false);
    Popup::AddCall(Popup::TextStylized, "Are you sure you want to delete this category?"
                   "\nThis action cannot be undone", "Bold", true);
    Popup::AddCall(Popup::Button, "Yes", DeleteItem);
    Popup::AddCall(Popup::SameLine);
    Popup::AddCall(Popup::Button, "No", CancelAction);
}


void SaveNewItem()
{
    std::string id = FormId();
}

void SaveEditedItem()
{

}

void DeleteItem()
{

}

void CancelAction()
{
    tmpId = 0;
    memset(tmpIdStr, 0, sizeof(tmpIdStr));
    memset(tmpDesc, 0, sizeof(tmpDesc));
    tmpCat = 0;
    memset(tmpRefLink, 0, sizeof(tmpRefLink));
    tmpPrice = 0.00f;
    tmpQty = 0.0f;
    memset(tmpUnit, 0, sizeof(tmpUnit));
    tmpStatus = 0;
}

void HandlePopupIdInput()
{
    if (tmpAutoId == true)
    {
        tmpId = DB::Item::GetNewId();
    }
    char idLabel[50] = { 0 };
    _itoa_s(tmpId, tmpIdStr, sizeof(tmpIdStr), 10);
    sprintf_s(idLabel, sizeof(idLabel), "ID Will be: %s", FormId().c_str());
    ImGui::InputText("ID", tmpIdStr, sizeof(tmpIdStr), ImGuiInputTextFlags_CharsDecimal);
    std::string t = tmpIdStr;
    if (!t.empty())
    {
        tmpId = std::stoi(t);
    }
    else
    {
        tmpId = 0;
    }
    ImGui::Columns(2, nullptr, false);
    ImGui::Text(idLabel);
    ImGui::NextColumn();
    ImGui::Checkbox("Auto ID", &tmpAutoId);
    ImGui::NextColumn();
    ImGui::Columns(1);
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

    ImGui::Combo("Category", &tmpCat, cats.data(), cats.size());
}

void HandlePopupReferenceLinkInput()
{
    ImGui::InputText("Reference Link", tmpRefLink, sizeof(tmpRefLink));
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
    ImGui::InputText("Reference Link", tmpRefLink, sizeof(tmpRefLink));
}

void HandlePopupStatusInput()
{
    using namespace DB::Item;
    std::vector<const char*> statuses;
    statuses.emplace_back("Active");
    statuses.emplace_back("Obsolete");
    statuses.emplace_back("Not Recommended For New Design");

    ImGui::Combo("Status", &tmpStatus, statuses.data(), statuses.size());
}

int GetCategoryNumber(DB::Category::Category& cat)
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

std::string FormId()
{
    std::stringstream ss;

    // The number is converted to string with the help of stringstream.
    ss << tmpId;
    std::string ret;
    ss >> ret;
    ret = ret.substr(0, 4); // Only keep 4 chars.
    // Append zero chars.
    int strLen = ret.length();
    for (int i = 0; i < 4 - strLen; i++)
    {
        ret = "0" + ret;
    }
    return categories[tmpCat].GetPrefix() + ret;
}
