#include "CategoryViewer.h"
#include "utils/db/Category.h"
#include "vendor/imgui/imgui.h"
#include "boost/algorithm/string.hpp"
#include "widgets/Logger.h"
#include "widgets/Popup.h"

#include <algorithm>

static void SortByName(std::vector<DB::Category::Category>& categories);
static void SortByPrefix(std::vector<DB::Category::Category>& categories);
static void MakeEditPopup();
static void MakeDeletePopup();
static void HandlePopupNameInput();
static void HandlePopupPrefixInput();
static void HandlePopupSuffixInput();
static void SaveNewCategory();
static void CancelAdd();
static void SaveEditedCategory();
static void DeleteCategory();

static bool isOpen = false;
static char tmpNameBuf[200] = { 0 };
static char tmpPrefixBuf[5] = { 0 };
static char tmpSufixBuf[2] = { 0 };
static DB::Category::Category tmpCat;

void CategoryViewer::Render()
{
    static bool isEditOpen = false;
    static bool isDeleteOpen = false;


    if (isOpen == false)
    {
        return;
    }

    DB::Category::Refresh();

    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Appearing);

    if (!ImGui::Begin("Category Editor", &isOpen))
    {
        ImGui::End();
        return;
    }

#pragma region Control Buttons
    // Only show buttons if user has admin rights.
    if (DB::HasUserWritePrivileges())
    {
        if (ImGui::Button("Add"))
        {
        #pragma region Add Pop up
            Popup::Init("Add New Category", false);
            Popup::AddCall(HandlePopupNameInput);
            Popup::AddCall(HandlePopupPrefixInput);
            Popup::AddCall(HandlePopupSuffixInput);
            Popup::AddCall(Popup::Button, "Accept", SaveNewCategory, true);
            Popup::AddCall(Popup::SameLine);
            Popup::AddCall(Popup::Button, "Cancel", CancelAdd, true);
            Popup::AddCall(ImGui::Spacing);
            Popup::AddCall(ImGui::Separator);
            Popup::AddCall(ImGui::Spacing);
            Popup::AddCall(Popup::Text, "The code are four characters that will be added to the ID of the items in this category");
        #pragma endregion
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

#pragma region Main Renderer
    static bool orderByName = true;
    ImGui::BeginChildFrame(ImGui::GetID("##CategoryChildFrameID"), ImVec2());

    // Header
    ImGui::Columns(3);
    if (ImGui::Selectable("Name", orderByName))
    {
        orderByName = true;
    }
    ImGui::NextColumn();
    if (ImGui::Selectable("Prefix", !orderByName))
    {
        orderByName = false;
    }
    ImGui::NextColumn();
    ImGui::Text("Suffix");
    ImGui::NextColumn();

    // Categories

    auto categories = DB::Category::GetAll();
    if (orderByName == true)
    {
        SortByName(categories);
    }
    else
    {
        SortByPrefix(categories);
    }

    for (auto& category : categories)
    {
        ImGui::Separator();
        if (isEditOpen == true)
        {
            std::string l = "Edit##" + category.GetName();
            if (ImGui::SmallButton(l.c_str()))
            {
                isEditOpen = false;
                tmpCat = category;
                MakeEditPopup();
            }
            ImGui::SameLine();
        }
        else if (isDeleteOpen == true)
        {
            std::string l = "Delete##" + category.GetName();
            if (ImGui::SmallButton(l.c_str()))
            {
                isDeleteOpen = false;
                tmpCat = category;
                MakeDeletePopup();
            }
            ImGui::SameLine();
        }

        ImGui::Text(category.GetName().c_str());
        ImGui::NextColumn();
        ImGui::Text(category.GetPrefix().c_str());
        ImGui::NextColumn();
        if (category.GetSuffix() != '\0')
        {
            ImGui::Text("%c", category.GetSuffix());
        }
        else
        {
            ImGui::Text("None");
        }
        ImGui::NextColumn();
    }

    ImGui::Columns(1);
    ImGui::Separator();

    ImGui::EndChildFrame();
#pragma endregion

    ImGui::End();
}

void CategoryViewer::Open()
{
    isOpen = true;
}

void SortByName(std::vector<DB::Category::Category>& categories)
{
    // Sort all categories by their names.
    std::sort(categories.begin(), categories.end(),
              [](DB::Category::Category& a, DB::Category::Category& b)
              {
                  std::string sa = a.GetName();
                  std::string sb = b.GetName();
                  boost::to_upper(sa);
                  boost::to_upper(sb);
                  return (sa.compare(sb.c_str()) <= 0 ? true : false);
              });
}

void SortByPrefix(std::vector<DB::Category::Category>& categories)
{
    // Sort all categories by their prefixes.
    std::sort(categories.begin(), categories.end(),
              [](DB::Category::Category& a, DB::Category::Category& b)
              {
                  std::string sa = a.GetPrefix();
                  std::string sb = b.GetPrefix();
                  boost::to_upper(sa);
                  boost::to_upper(sb);
                  return (sa.compare(sb.c_str()) <= 0 ? true : false);
              });
}

void MakeEditPopup()
{
    strcpy_s(tmpNameBuf, sizeof(tmpNameBuf), tmpCat.GetName().c_str());
    strcpy_s(tmpPrefixBuf, sizeof(tmpPrefixBuf), tmpCat.GetPrefix().c_str());
    tmpSufixBuf[0] = tmpCat.GetSuffix();
    Popup::Init("Edit Category", false);
    Popup::AddCall(HandlePopupNameInput);
    Popup::AddCall(HandlePopupPrefixInput);
    Popup::AddCall(HandlePopupSuffixInput);
    Popup::AddCall(Popup::Button, "Accept", SaveEditedCategory, true);
    Popup::AddCall(Popup::SameLine);
    Popup::AddCall(Popup::Button, "Cancel", CancelAdd, true);
}

void MakeDeletePopup()
{
    Popup::Init("Delete Category", false);
    Popup::AddCall(Popup::TextStylized, "Are you sure you want to delete this category?"
                   "\nThis action cannot be undone", "Bold", true);
    Popup::AddCall(Popup::Button, "Yes", DeleteCategory, true);
    Popup::AddCall(Popup::SameLine);
    Popup::AddCall(Popup::Button, "No", CancelAdd, true);
}

void HandlePopupNameInput()
{
    ImGui::InputText("Description", tmpNameBuf, sizeof(tmpNameBuf));
}

void HandlePopupPrefixInput()
{
    ImGui::InputText("Code", tmpPrefixBuf, sizeof(tmpPrefixBuf),
                     ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_CharsNoBlank);
}

void HandlePopupSuffixInput()
{
    ImGui::InputText("Revision", tmpSufixBuf, sizeof(tmpSufixBuf),
                     ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_CharsNoBlank);
    ImGui::Text("Leave empty for no revision letter");
}

void SaveNewCategory()
{
    std::string desc = tmpNameBuf;
    std::string prefix = tmpPrefixBuf;
    char suffix = tmpSufixBuf[0];

    memset(tmpNameBuf, 0, sizeof(tmpNameBuf));
    memset(tmpPrefixBuf, 0, sizeof(tmpPrefixBuf));
    memset(tmpSufixBuf, 0, sizeof(tmpSufixBuf));
    if (!desc.empty() && !prefix.empty())
    {
        DB::Category::AddCategory({ desc, prefix, suffix });
    }
}

void CancelAdd()
{
    memset(tmpNameBuf, 0, sizeof(tmpNameBuf));
    memset(tmpPrefixBuf, 0, sizeof(tmpPrefixBuf));
    memset(tmpSufixBuf, 0, sizeof(tmpSufixBuf));
}

void SaveEditedCategory()
{
    std::string desc = tmpNameBuf;
    std::string prefix = tmpPrefixBuf;
    char suffix = tmpSufixBuf[0];

    memset(tmpNameBuf, 0, sizeof(tmpNameBuf));
    memset(tmpPrefixBuf, 0, sizeof(tmpPrefixBuf));
    if (!desc.empty() && !prefix.empty())
    {
        if (DB::Category::EditCategory(tmpCat, { desc, prefix, tmpSufixBuf[0] }) == false)
        {
            Logging::System.Error("Unable to edit category!");
        }

    }
    memset(tmpSufixBuf, 0, sizeof(tmpSufixBuf));
}

void DeleteCategory()
{
    DB::Category::DeleteCategory(tmpCat);
}
