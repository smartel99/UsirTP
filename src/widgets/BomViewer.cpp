#include "BomViewer.h"
#include "utils/db/Bom.h"
#include "vendor/imgui/imgui.h"
#include "boost/algorithm/string.hpp"
#include "widgets/Popup.h"
#include <algorithm>


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

static DB::BOM::BOM tmpBom;
static std::vector<ItemRef> tmpItems;
static DB::BOM::ItemReference tmpOutput;
static int tmpSelectedOut = 0;
static char tmpName[MAX_INPUT_LENGHT];
static bool isAddOpen = false;
static bool isEditOpen = false;
static bool isMakeValid = false;


void BomViewer::Render()
{
    static bool isDeleteOpen = false;

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
            isEditOpen = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Delete"))
        {
            isDeleteOpen = true;
        }
    }
#pragma endregion

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
    ImGui::Text("Output Item");
    ImGui::NextColumn();
#pragma endregion Output

#pragma region Items
    ImGui::Text("Items");
    ImGui::NextColumn();
#pragma endregion Items

#pragma endregion Header

    std::vector<DB::BOM::BOM> boms = DB::BOM::GetAll();
    SortItems(sort, boms);
#pragma region Content
    for (auto& bom : boms)
    {

        ImGui::Separator();
        if (isEditOpen == true)
        {
            std::string l = "Edit##" + bom.GetId();
            if (ImGui::SmallButton(l.c_str()))
            {
                isEditOpen = false;
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

        if (ImGui::Selectable(std::string(bom.GetId() + "##selectable").c_str(), false
        /*,ImGuiSelectableFlags_SpanAllColumns*/))
        {
            tmpBom = bom;
            MakeMakePopup();
        }
        ImGui::NextColumn();

        ImGui::Text(bom.GetName().c_str());
        ImGui::NextColumn();

        ImGui::Text(bom.GetRawOutput().GetId().c_str());
        ImGui::NextColumn();

        std::string l = "Items##" + bom.GetId();
        std::string p = l + "pop up";
        if (ImGui::Selectable(l.c_str()))
        {
            ImGui::OpenPopup(p.c_str());
        }
        if (ImGui::BeginPopup(p.c_str(), ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Dummy(ImVec2(200, 10));
            std::vector<DB::BOM::ItemReference> its = bom.GetRawItems();
            ImGui::Columns(2);
            for (auto& it : its)
            {
                ImGui::Separator();
                ImGui::Text(it.GetId().c_str());
                ImGui::NextColumn();
                ImGui::Text("%0.2f", it.GetQuantity());
                ImGui::NextColumn();
            }
            ImGui::Columns(1);
            ImGui::Separator();

            ImGui::EndPopup();
        }
        ImGui::NextColumn();
    }
#pragma endregion Content

    ImGui::EndChildFrame();
}

void RenderAddWindow()
{
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Appearing);
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
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Appearing);
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
        tmpItems.emplace_back(ItemRef(DB::BOM::ItemReference(i.GetId(), i.GetOid(), i.GetQuantity()), false));
    }
    tmpSelectedOut = 0;
}

static void MakeEditPopup(bool isRetry)
{
    std::vector<DB::Item::Item> items = DB::Item::GetAll();
    for (auto& i : items)
    {
        tmpItems.emplace_back(ItemRef(DB::BOM::ItemReference(i.GetId(), i.GetOid(), i.GetQuantity()), false));
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
        tmpSelectedOut = std::distance(tmpItems.begin(), it);    // Get the index of tmpOutput in tmpItems.
    }
    else
    {
        tmpSelectedOut = 0;
    }
    strcpy_s(tmpName, sizeof(tmpName), tmpBom.GetName().c_str());
}

static void MakeDeletePopup()
{
    Popup::Init("Delete BOM", false);
    Popup::AddCall(Popup::TextStylized, "Are you sure you want to delete this BOM?"
                   "\nThis action cannot be undone", "Bold", true);
    Popup::AddCall(Popup::Button, "Yes", DeleteBom, true);
    Popup::AddCall(Popup::SameLine);
    Popup::AddCall(Popup::Button, "No", CancelAction);
}

static void CancelAction()
{
    tmpBom = DB::BOM::BOM();
    memset(tmpName, 0, sizeof(tmpName));
    tmpItems.clear();
    tmpOutput = DB::BOM::ItemReference();

    isEditOpen = false;
    isAddOpen = false;
}

void MakeMakePopup()
{
    Popup::Init("Make BOM", false);
    Popup::AddCall(HandlePopupMake);
    Popup::AddCall(HandlePopupMakeButton, "Make", CommitMake, true);
    Popup::AddCall(Popup::SameLine);
    Popup::AddCall(HandlePopupMakeButton, "Cancel", CancelAction, true);
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
            items.emplace_back(item.reference);
        }
    }

    tmpOutput = tmpItems.at(tmpSelectedOut).reference;

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
    /*throw gcnew System::NotImplementedException();*/
}

void HandlePopupItemPickerInput()
{
    ImGui::Text("Items to use:");
    ImGui::BeginChildFrame(ImGui::GetID("ItemPickerChildFrame"), ImVec2(0, 200));
    ImGui::Columns(3);
    ImGui::Text("Item ID");
    ImGui::NextColumn();
    ImGui::Text("Available");
    ImGui::NextColumn();
    ImGui::Text("Quantity");
    ImGui::NextColumn();

    for (auto& item : tmpItems)
    {
        ImGui::Separator();
        ImGui::Checkbox(item.reference.GetId().c_str(), &item.isSelected);
        ImGui::NextColumn();
        float available = DB::Item::GetItemByID(item.reference.GetId()).GetQuantity();
        ImGui::Text("%0.2f %s", available, DB::Item::GetItemByID(item.reference.GetId()).GetUnit());
        ImGui::NextColumn();
        ImGui::BeginChildFrame(ImGui::GetID(std::string("##QtyChildFrame" + item.reference.GetId()).c_str()),
                               ImVec2(0, ImGui::GetFrameHeight()));
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

    ImGui::Combo("Created Item", &tmpSelectedOut, ids.data(), ids.size());

}

void HandlePopupMake()
{
    std::string txt = "Make Product?";
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

    for (auto& i : tmpBom.GetRawItems())
    {
        ImGui::Separator();
        float avail = DB::Item::GetItemByID(i.GetId()).GetQuantity();
        float needed = i.GetQuantity();
        ImU32 col = avail < needed ? 0xFF0000FF : ImGui::GetColorU32(ImGuiCol_Text);

        isMakeValid = avail < needed ? false : true;

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
}

bool HandlePopupMakeButton(const std::string& label)
{
    bool needToPop = false;
    bool r = false;
    if (isMakeValid == false && label != "Cancel")
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_TextDisabled));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetColorU32(ImGuiCol_TextDisabled));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetColorU32(ImGuiCol_TextDisabled));
        needToPop = true;
    }
    if (ImGui::Button(label.c_str()) && (label == "Cancel" || isMakeValid == true))
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
