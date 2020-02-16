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

/**
 * @def     MAX_INPUT_LENGHT
 * @brief   Maximum length of user input text.
 */
#define MAX_INPUT_LENGTH    400

/**
 * @def    SORT_ASCEND
 * @brief  Text to display when sorting in ascending order.
 */
#define SORT_ASCEND         "(A to Z)"

/**
 * @def    SORT_DESCEND
 * @brief  Text to display when sorting in descending order.
 */
#define SORT_DESCEND        "(Z to A)"

/**
 * @def    SORT_TXT
 * @brief  Determine if the text that should be displayed should be `SORT_ASCEND` or `SORT_DESCEND`
 */
#define SORT_TXT(x)         (sort == SortBy::x ? SORT_ASCEND : (sort == SortBy::r ## x ? SORT_DESCEND : " "))

/**
 * @def    IS_SORT_ACTIVE
 * @brief  Check if `x` is the current method of sorting.
 */
#define IS_SORT_ACTIVE(x)   (( sort == SortBy::x ) || ( sort == SortBy::r ## x ))

/**
 * @enum    SortBy
 * @brief   The possible ways to sort by.
 */
enum class SortBy
{
    id = 0, /**< Sort by ID in ascending order */
    rid,    /**< Sort by ID in descending order */
    name,   /**< Sort by name in ascending order */
    rname   /**< Sort by name in descending order */
};

/**
 * @class ItemRef
 * @brief A class representing an active, more complete reference of a `DB::Item::Item`
 */
class ItemRef
{
public:
    /**
     * @brief   Default (and only) constructor of the class.
     * @param   items: A reference to a `DB::BOM::ItemReference` object that is to be bound with the object.
     * @param   selected: A bool that indicates if the `ItemRef` is part of the BOM or not.
     * @retval  The new instance of ItemRef.
     */
    ItemRef(const DB::BOM::ItemReference& items, bool selected) :
        reference(items), isSelected(selected)
    {
    }
    DB::BOM::ItemReference reference;   /**< The Item that `this` represents */
    bool isSelected = false;            /**< Is that item part of the BOM */
    float quantity = 0.0f;              /**< The quantity `reference` needed by the BOM to make it */
};

static void RenderAddWindow();
static void RenderEditWindow();
static void RenderItemPopup(std::string& p, DB::BOM::BOM& bom);
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

//! The BOM currently being worked with.
static DB::BOM::BOM tmpBom;

//! The list of Items of the BOM currently being worked with.
static std::vector<ItemRef> tmpItems;
static std::vector<ItemRef> tmpItemsForOutput;

//! The output Item of the BOM currently being worked with.
static DB::BOM::ItemReference tmpOutput;

//! The index of the currently selected `tmpOutput`.
static int tmpSelectedOut = 0;

//! The input buffer for the name of the BOM currently being worked with.
static char tmpName[MAX_INPUT_LENGTH];

//! The quantity of BOM items to make for the BOM currently being worked with.
static int tmpQuantityMade = 1;

//! The quantity of Items that are produced by the BOM currently being worked with.
static int tmpQuantityToMake = 1;

//! Flag that indicates if the `Add BOM` window should be rendered.
static bool isAddOpen = false;

//! Flag that indicates if the `edit` window is open.
static bool isEditOpen = false;

//! Flag that indicates if the BOM currently being worked with can be made.
static bool isMakeValid = false;

//! The path of the file in which to export the data.
static std::wstring tmpOutputFilePath = L"";

//! Object that handles all filtering functionalities for the items.
static FilterUtils::FilterHandler itemFilter;

//! All the BOM categories that can be used to filter BOMs with.
const static std::vector<std::string> cats = { "ID", "Description", "Output ID" };

//! Object that handles all filtering functionalities for the BOMs.
static FilterUtils::FilterHandler filter(cats);

/**
 * @brief   Main rendering task of the module.
 *          It takes care of:
 *              - Displaying the filter input menu
 *              - Displaying every BOMs in the cache
 *              - Refreshing the BOM cache
 *              - Adding, Editing and Deleting BOMs
 *              - Rendering the Adding, the Editing and the Deleting windows/pop ups.
 * @param   None
 * @retval  None
 */
void BomViewer::Render()
{
    static bool isDeleteOpen = false;   /**< Should the `delete` button be drawn? */
    static bool isEditPending = false;  /**< Should the `edit` button be drawn? */

    // Change the frame's background to be of a semi light gray color.
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.39f, 0.39f, 0.39f, 0.5859375f));

    // Call the handler in charge of refreshing the BOMs's cache
    DB::BOM::Refresh();

    // If the `Add BOM` window should be rendered:
    if (isAddOpen == true)
    {
        // Render it.
        RenderAddWindow();
    }
    // If the `Edit BOM` window should be rendered:
    else if (isEditOpen == true)
    {
        // Render it.
        RenderEditWindow();
    }

    // Create 3 columns with no ImGui ID and no border lines.
    ImGui::Columns(3, nullptr, false);

    // Set the width of the 1st column (the left one) to be 75% of its original size.
    const static float w1 = ImGui::GetColumnWidth(-1) * 0.75f;
    ImGui::SetColumnWidth(-1, w1);

/**
 * This region handles the `Add`/`Edit`/`Delete` buttons, the filter bar and the `Export` button.
 */
#pragma region Controls
    // Only show buttons if user has readWrite rights.
    if (DB::HasUserWritePrivileges())
    {
        // If the `Add` button has been clicked by the user:
        if (ImGui::Button("Add"))
        {
            // Set the flag so that the `Add BOM` window is rendered.
            isAddOpen = true;
            // Set everything up so that the `Add BOM` window is read for the next frame.
            MakeNewPopup();
        }
        // On the same line as the `Add` button.
        ImGui::SameLine();
        // If the `Edit` button has been clicked by the user:
        if (ImGui::Button("Edit"))
        {
            // Toggle the state of the flag.
            isEditPending = !isEditPending;
            // Set the `Delete` flag to false to overwrite its state.
            isDeleteOpen = false;
        }
        // On the same line as the `Edit` button.
        ImGui::SameLine();
        // If the `Delete` button has been clicked by the user:
        if (ImGui::Button("Delete"))
        {
            // Toggle the state of the flag.
            isDeleteOpen = !isDeleteOpen;
            // Set the `Edit` flag to false to overwrite its state.
            isEditOpen = false;
        }
    }

    // Move to the next column.
    ImGui::NextColumn();
    // Set the width of the second column (the center one) to be 150% of its original size.
    const static float w2 = ImGui::GetColumnWidth(-1) * 1.5f;
    ImGui::SetColumnWidth(-1, w2);

    // Render the filter.
    filter.Render();
    // Move to the next column.
    ImGui::NextColumn();
    // If the `Export` button has been clicked by the user:
    if (ImGui::Button("Export"))
    {
        // Export the currently displayed BOMs.
        ExportItems();
    }
    // Move to the next column.
    ImGui::NextColumn();
    // Creates a single column to align everything up.
    ImGui::Columns(1);
#pragma endregion

    // Create a child frame that spans all the remaining available space in the parent frame.
    ImGui::BeginChildFrame(ImGui::GetID("BomViewerChildFrame"),
                           ImVec2(0, (ImGui::GetWindowHeight() - ImGui::GetCursorPosY() - 25)));
    // Pop the gray color we've pushed on the ImGui stack earlier.
    ImGui::PopStyleColor();

    // Flag that stores how we should sort the list of BOMs.
    static SortBy sort = SortBy::id;

/**
 * This region handles the headers for the tab and the sorting of the BOMs as well.
 */
#pragma region Header
    // Create 4 columns, no ImGui ID, with borders.
    ImGui::Columns(4);
//! This region handles the ID header.
#pragma region ID
    // Overwrite the frame background color to be 0x00000000 (transparent).
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4());
    // Create a child frame that spans all the available width of the column and is as tall as the font is.
    ImGui::BeginChildFrame(ImGui::GetID("##BomIdChildFrame"), ImVec2(0, ImGui::GetFrameHeight()));
    // Return the frame background color to its original state.
    ImGui::PopStyleColor();

    // Create 2 columns, no ImGui ID, no borders.
    ImGui::Columns(2, nullptr, false);

    // If the `ID` cell has been clicked on by the user:
    if (ImGui::Selectable("ID", IS_SORT_ACTIVE(id), ImGuiSelectableFlags_SpanAllColumns))
    {
        // Toggle the sort by ID (ascend or descend)
        sort = sort == SortBy::id ? SortBy::rid : SortBy::id;
    }
    // Move to the next column.
    ImGui::NextColumn();
    // If we're sorting by ID, display `SORT_ASCEND` or `SORT_DESCEND`, depending on the direction.
    ImGui::Text(SORT_TXT(id));
    // Move to the next column.
    ImGui::NextColumn();
    // Create one empty column for the alignment.
    ImGui::Columns(1);
    // End the ID child frame.
    ImGui::EndChildFrame();
    // Move to the next column.
    ImGui::NextColumn();
#pragma endregion ID

//! This region handles the Name header.
#pragma region Name
    // Overwrite the frame background color to be transparent.
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4());
    // Create a child frame that spans the entire width of the column and is as tall as the font is.
    ImGui::BeginChildFrame(ImGui::GetID("##BomNameChildFrame"), ImVec2(0, ImGui::GetFrameHeight()));
    // Reset the frame background color to its original state.
    ImGui::PopStyleColor();

    // Create 2 columns, no ImGui ID, no borders.
    ImGui::Columns(2, nullptr, false);

    // If the Name cell has been clicked on by the user:
    if (ImGui::Selectable("Name", IS_SORT_ACTIVE(name), ImGuiSelectableFlags_SpanAllColumns))
    {
        // Toggle the sort by name (ascend or descend).
        sort = sort == SortBy::name ? SortBy::rname : SortBy::name;
    }

    // Move to the next column.
    ImGui::NextColumn();
    // If we're sorting by Name, display `SORT_ASCEND` or `SORT_DESCEND`, depending on the direction.
    ImGui::Text(SORT_TXT(name));
    // Move to the next column.
    ImGui::NextColumn();
    // Create an empty column for alignment.
    ImGui::Columns(1);
    // End the Name child frame.
    ImGui::EndChildFrame();
    // Move to the next column.
    ImGui::NextColumn();
#pragma endregion Name

//! This region handles the Output header.
#pragma region Output
    // Overwrite the frame background, the header hovered and the header active colors to be all transparent.
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4());
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4());
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4());
    // Create a child frame that spans all the available width and is as high as the font is.
    ImGui::BeginChildFrame(ImGui::GetID("##BomOutputChildFrame"), ImVec2(0, ImGui::GetFrameHeight()));
    // Display the label text.
    // An `ImBui::Selectable` is used here in place of `ImGui::Text` 
    // to be visually consistent with `ID` and `Name`.
    ImGui::Selectable("Output Item");
    // End the Output child frame.
    ImGui::EndChildFrame();
    // Move to the next column.
    ImGui::NextColumn();
    // We don't restore the colors we've changed yet because we still want to use them for the next header.
#pragma endregion Output

//! This region handles the Items header.
#pragma region Items
    // Create a child frame that spans all the available width and is as high as the font is.
    ImGui::BeginChildFrame(ImGui::GetID("##BomItemsChildFrame"), ImVec2(0, ImGui::GetFrameHeight()));
    // Display the label text.
    // An `ImBui::Selectable` is used here in place of `ImGui::Text` 
    // to be visually consistent with `ID` and `Name`.
    ImGui::Selectable("Items");
    // End the Items child frame.
    ImGui::EndChildFrame();
    // Restore the frame background, header hovered and header active colors to their original ones.
    ImGui::PopStyleColor(3);
    // Move to the next column.
    ImGui::NextColumn();
#pragma endregion Items

#pragma endregion Header

    // Fetches all the BOMs from the cache.
    std::vector<DB::BOM::BOM> boms = DB::BOM::GetAll();
    // Sort the BOMs with the appropriate sorting.
    SortItems(sort, boms);

//! This region handles the rendering of the BOMs
#pragma region Content
    // For each BOM in the list:
    for (auto& bom : boms)
    {
        // Draw an horizontal line.
        ImGui::Separator();
        // If we're in edit mode:
        if (isEditPending == true)
        {
            // Add a small button next to the BOM's ID.
            std::string l = "Edit##" + bom.GetId();
            // If that small button has been clicked on by the user:
            if (ImGui::SmallButton(l.c_str()))
            {
                // Render the edit menu on the next frame.
                isEditOpen = true;
                // Exit edit mode.
                isEditPending = false;
                // Set the temporary BOM to the current one.
                tmpBom = bom;
                // Set everything up for the next frame.
                MakeEditPopup();
            }
            ImGui::SameLine();
        }
        // If we're in delete mode:
        else if (isDeleteOpen == true)
        {
            // Add a small button next to the BOM's ID.
            std::string l = "Delete##" + bom.GetId();
            // If that small button has been clicked on by the user:
            if (ImGui::SmallButton(l.c_str()))
            {
                // Exit delete mode.
                isDeleteOpen = false;
                // Set the temporary BOM to the current one.
                tmpBom = bom;
                // Set everything up for the next frame.
                MakeDeletePopup();
            }
            ImGui::SameLine();
        }

        // If the user has clicked on the BOM's ID:
        // Clicking on a BOM's ID opens a cost preview window.
        if (ImGui::Selectable(std::string(bom.GetId() + "##selectable").c_str(), false))
        {
            // Set the BOM to the current one.
            tmpBom = bom;
            // Set everything up for the next frame.
            MakeMakePopup();
        }

        // Move to the next column.
        ImGui::NextColumn();

        // Display the name of the BOM.
        ImGui::Text(bom.GetName().c_str());
        // Move to the next column.
        ImGui::NextColumn();

        // Display the ID of the BOM's output Item.
        ImGui::Text(bom.GetRawOutput().GetId().c_str());
        // Move to the next column.
        ImGui::NextColumn();

        // Create a unique label ID for this BOM.
        std::string l = "Click to view##" + bom.GetId();
        // Create a unique pop up ID from the above label.
        std::string p = l + "pop up";
        // If the user has clicked on the "Click to view" field:
        if (ImGui::Selectable(l.c_str()))
        {
            // Open the pop up.
            ImGui::OpenPopup(p.c_str());
        }

        // If needed, render the pop up.
        RenderItemPopup(p, bom);

        // Move on to the next line.
        ImGui::NextColumn();
    }
#pragma endregion Content

    ImGui::EndChildFrame();
}

/**
 * @brief   Handles the rendering of the `Add BOM` window.
 * @param   None
 * @retval  None
 */
void RenderAddWindow()
{
    // Set the size of the next window to 800x700 when it first appears.
    ImGui::SetNextWindowSize(ImVec2(800, 700), ImGuiCond_Appearing);
    // Create the `Add BOM` window.
    if (ImGui::Begin("Add new BOM"))
    {
        // Handle the user's input for the BOM's name/description.
        HandlePopupNameInput();
        // Handle the user's input for the BOM's Items (the ingredients of the BOM).
        HandlePopupItemPickerInput();
        // Handle the user's input for the BOM's output Item.
        HandlePopupOutputPickerInput();

        // If the `Accept` button has been clicked on by the user:
        if (ImGui::Button("Accept"))
        {
            // Save the newly created BOM.
            SaveNewBom();
        }
        ImGui::SameLine();
        // If the `Cancel` button has been clicked on by the user:
        if (ImGui::Button("Cancel"))
        {
            // Clear all temporary variables.
            CancelAction();
        }

        // End the `Add BOM` window.
        ImGui::End();
    }
}

/**
 * @brief   Handles the rendering of the `Edit BOM` window.
 * @param   None
 * @retval  None
 */
void RenderEditWindow()
{
    // Set the size of the next window to 800x700 when it first appears.
    ImGui::SetNextWindowSize(ImVec2(800, 700), ImGuiCond_Appearing);
    // Create the `Edit BOM` window.
    if (ImGui::Begin("Edit BOM"))
    {
        // Handle the user's input for the BOM's name/description.
        HandlePopupNameInput();
        // Handle the user's input for the BOM's Items (the ingredients of the BOM).
        HandlePopupItemPickerInput();
        // Handle the user's input for the BOM's output Item.
        HandlePopupOutputPickerInput();

        // If the `Accept` button has been clicked on by the user:
        if (ImGui::Button("Accept"))
        {
            // Save the edited BOM.
            SaveEditedBom();
        }
        ImGui::SameLine();
        // If the `Cancel` button has been clicked on by the user:
        if (ImGui::Button("Cancel"))
        {
            // Clear all temporary variables.
            CancelAction();
        }

        // End of the `Edit BOM` window.
        ImGui::End();
    }
}

/**
 * @brief   Render a pop up that contains a list of all the Items needed by a BOM, if that pop up is open.
 * @param   p: The name and ID of the pop up unique to this `bom`.
 * @param   bom: The BOM object to display the Items from.
 * @retval  None
 */
void RenderItemPopup(std::string& p, DB::BOM::BOM& bom)
{
    // If the pop up should be drawn:
    if (ImGui::BeginPopup(p.c_str(), ImGuiWindowFlags_AlwaysAutoResize))
    {
        // Forces the pop up to be 400 pixels wide with a dummy object.
        ImGui::Dummy(ImVec2(400.f, 0.1f));
        // Get a list of all the items in the BOM.
        std::vector<DB::BOM::ItemReference> its = bom.GetRawItems();
        // Create 3 columns, no ImGui ID, with borders.
        ImGui::Columns(3);

        // Overwrite the default text font with a bold one.
        Fonts::Push("Bold");
        // Render the headers.
        ImGui::Text("Item ID");
        ImGui::NextColumn();
        ImGui::Text("Description");
        ImGui::NextColumn();
        ImGui::Text("Qty Needed");
        // Restore the text font to its default state.
        Fonts::Pop();

        // Move to the next line.
        ImGui::NextColumn();

        // For each Item in the BOM:
        for (auto& it : its)
        {
            // Draw an horizontal line.
            ImGui::Separator();

            ImVec2 currentPos = ImGui::GetCursorScreenPos();
            ImVec2 bottomRight = ImVec2(currentPos.x + 400.f, currentPos.y + ImGui::GetFrameHeight());
            bool isMouseHoveringItem = ImGui::IsMouseHoveringRect(currentPos, bottomRight, false);
            ImVec2 descSize = ImVec2(0, ImGui::GetFrameHeight());
            if (isMouseHoveringItem == true)
            {
                // Calculate the number of lines the full description of the Item takes.
                descSize.y = ((it.GetItem().GetDescription().size() / 15) + 1) * ImGui::GetFrameHeight();
            }
            // Display the Item's ID.
            ImGui::Text(it.GetId().c_str());
            // Move on to the next column.
            ImGui::NextColumn();

            // Display the Item's description.
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4());
            ImGui::BeginChildFrame(ImGui::GetID(std::string("##BomItemDescChildFrame" + it.GetId()).c_str()),
                                   descSize);
            std::string d = isMouseHoveringItem == true ? it.GetItem().GetDescription() :
                it.GetItem().GetDescription().substr(0, 15) + "...";

            if (isMouseHoveringItem == true)
            {
                ImGui::TextWrapped(d.c_str());
            }
            else
            {
                ImGui::Text(d.c_str());
            }
            ImGui::EndChildFrame();
            ImGui::PopStyleColor();
            // Move on to the next column.
            ImGui::NextColumn();
            // Display the quantity needed.
            ImGui::Text("%0.2f", it.GetQuantity());
            // Move on to the next line.
            ImGui::NextColumn();
        }
        // Create an empty column for alignment.
        ImGui::Columns(1);
        // Add an horizontal line.
        ImGui::Separator();

        // Display how many items the BOM creates when made.
        ImGui::Text("Makes %0.2f %s", bom.GetRawOutput().GetQuantity(), bom.GetOutput().GetUnit());

        // End the pop up.
        ImGui::EndPopup();
    }
}

/**
 * @brief   Sort a list of BOMs with a specific sorting term.
 * @param   sort: By what to sort the list.
 * @param   boms: The list of BOMs to sort.
 * @retval  None
 *
 * @note    The list of BOMs is sorted in place, not in a copy.
 */
static void SortItems(SortBy sort, std::vector<DB::BOM::BOM>& boms)
{
    switch (sort)
    {
        // If they should be sorted by their IDs in ascending order:
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
        // If they should be sorted by their IDs in descending order:
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
        // If they should be sorted by their names in ascending order:
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
        // If they should be sorted by their IDs in descending order:
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

/**
 * @brief   Initialize all the temporary variables in order to let the user make a new BOM.
 * @param   None
 * @retval  None
 */
static void MakeNewPopup()
{
    // Get all the existing Items in the cache.
    std::vector<DB::Item::Item> items = DB::Item::GetAll();
    // For each Item in the cache:
    for (auto& i : items)
    {
        // Create an ItemRef out of the Item and add it to the temporary list.
        tmpItems.emplace_back(ItemRef(DB::BOM::ItemReference(i.GetId(), i.GetOid(),
                                                             i.GetQuantity(), int(tmpItems.size())),
                                      false));
        tmpItemsForOutput.emplace_back(ItemRef(DB::BOM::ItemReference(i.GetId(), i.GetOid(),
                                                                      i.GetQuantity(), int(tmpItems.size())),
                                               false));
    }
    // The user hasn't selected any output Item.
    tmpSelectedOut = 0;
    // The user hasn't set an output quantity. 
    tmpQuantityMade = 0;
    // The user hasn't entered any text in the filter box.
    itemFilter.ClearText();
}

/**
 * @brief   Initialize all the temporary variables in order to let the user edit an existing BOM.
 *          If `isRetry` is set, "Unable to save BOM" will be shown in red.
 * @param   isRetry: Set this flag if a previous attempt at creating/editing a BOM failed.
 * @retval  None
 *
 * @todo    isRetry is not used in this function, update it to remove isRetry.
 * #refactor
 */
static void MakeEditPopup(bool isRetry)
{
    // Get all of the Items that are in the cache.
    std::vector<DB::Item::Item> items = DB::Item::GetAll();
    // For each Item in the cache:
    for (auto& i : items)
    {
        // Create an ItemRef object from that Item and add it to the temporary list.
        tmpItems.emplace_back(ItemRef(DB::BOM::ItemReference(i.GetId(), i.GetOid(),
                                                             i.GetQuantity(), int(tmpItems.size() + items.size())),
                                      false));
        tmpItemsForOutput.emplace_back(ItemRef(DB::BOM::ItemReference(i.GetId(), i.GetOid(),
                                                                      i.GetQuantity(),
                                                                      int(tmpItems.size() + items.size())),
                                               false));
        // Check if that Item is one that is already part of the BOM's Item list.
        auto it = std::find_if(tmpBom.GetRawItems().begin(),
                               tmpBom.GetRawItems().end(),
                               [i](const DB::BOM::ItemReference& obj)
                               {
                                   return obj.GetId() == i.GetId();
                               });

        // If it is:
        if (it != tmpBom.GetRawItems().end())
        {
            // Update the ItemRef to take the values of the Item in the BOM's Item list.
            tmpItems.back().isSelected = true;
            tmpItems.back().quantity = it->GetQuantity();
            tmpItems.back().reference.SetPosition(it->GetPosition());
        }
    }

    // Fetch the output Item from the BOM.
    tmpOutput = tmpBom.GetRawOutput();

    // Make sure that the Item still exists.
    auto it = std::find_if(tmpItems.begin(), tmpItems.end(),
                           [](ItemRef& i)
                           {
                               return tmpOutput.GetId() == i.reference.GetId();
                           });
    // If it does still exist:
    if (it != tmpItems.end())
    {
        // Use its index.
        tmpSelectedOut = int(std::distance(tmpItems.begin(), it));    // Get the index of tmpOutput in tmpItems.
    }
    else
    {
        tmpSelectedOut = 0;
    }

    // Copy the current name of the BOM into the user input box.
    strcpy_s(tmpName, sizeof(tmpName), tmpBom.GetName().c_str());
    tmpQuantityMade = int(tmpBom.GetRawOutput().GetQuantity());

    // The user hasn't entered any text in the filter box.
    itemFilter.ClearText();
}

/**
 * @brief   Handle the creation of a confirmation box for the deletion of a BOM.
 * @param   None
 * @retval  None
 */
static void MakeDeletePopup()
{
    // Create a new pop up on the Popup stack.
    Popup::Init("Delete BOM", false);
    // Add stylized text to the pop up.
    Popup::AddCall(Popup::TextStylized, "Are you sure you want to delete this BOM?"
                   "\nThis action cannot be undone", "Bold", true);
    // Add a button to the pop up and bind it to the `DeleteBom` function.
    Popup::AddCall(Popup::Button, "Yes", DeleteBom, true);
    Popup::AddCall(Popup::SameLine);
    // Add a button to the pop up and bind it to the `CancelAction` function.
    Popup::AddCall(Popup::Button, "No", CancelAction, true);
}

/**
 * @brief   Cancel any ongoing action. This clears all temporary variables.
 * @param   None
 * @retval  None
 */
static void CancelAction()
{
    // Reset the temporary BOM object to the default, non-valid BOM.
    tmpBom = DB::BOM::BOM();
    // Clear the name input buffer.
    memset(tmpName, 0, sizeof(tmpName));
    // Clear the list of all the existing Items.
    tmpItems.clear();
    tmpItemsForOutput.clear();
    // Reset the temporary BOM's output object to the default ItemReference.
    tmpOutput = DB::BOM::ItemReference();
    // Reset the other values to their default states.
    tmpQuantityMade = 0;
    tmpQuantityToMake = 0;

    isEditOpen = false;
    isAddOpen = false;
}

/**
 * @brief   Handle the creation of the pop up for using a BOM.
 *          This binds all the necessary functions to the pop up.
 * @param   None
 * @retval  None
 */
void MakeMakePopup()
{
    // Don't let the user try to make less than 1 Item from the BOM.
    tmpQuantityToMake = 1;
    // Create a new pop up on the Popup stack.
    Popup::Init("Make BOM", false);
    // Add the `HandlePopupMake` function to the pop up.
    Popup::AddCall(HandlePopupMake);
    // Add a button with the label "Make" bond to the `CommitMake` function 
    // and that closes the pop up when the button is clicked on by the user.
    Popup::AddCall(HandlePopupMakeButton, "Make", CommitMake, true);
    Popup::AddCall(Popup::SameLine);
    // Add a button with the label "Cancel" bond to the `CancelAction` function
    // and that closes the pop up when the button is clicked on by the user.
    Popup::AddCall(HandlePopupMakeButton, "Cancel", CancelAction, true);
    Popup::AddCall(Popup::SameLine);
    // Add a button with the label "Export" bond to the `ExportItems` function
    // and that closes the pop up when the button is clicked on by the user.
    Popup::AddCall(HandlePopupMakeButton, "Export", ExportItems, true);
}

/**
 * @brief   Uses the information that the user inputted to create a new BOM and adds it to the database.
 * @param   None
 * @retval  None
 */
void SaveNewBom()
{
    // Create an ID for the new BOM.
    std::string id = DB::BOM::GetNewId();
    // Create a string from the name input buffer.
    std::string name = tmpName;
    std::vector<DB::BOM::ItemReference> items;
    // For each Item in the temporary Item list:
    for (auto& item : tmpItems)
    {
        // If the Item's check box is checked:
        if (item.isSelected == true)
        {
            // Update the Item's quantity to be the one specified by the user, 
            // aka the quantity of that Item needed to make the BOM.
            item.reference.SetQuantity(item.quantity);
            // Add this Item to the BOM's Item list.
            items.emplace_back(item.reference);
        }
    }

    // Update the Output Item's quantity to be the one specified by the user,
    // aka the quantity of Items made by that BOM.
    tmpItemsForOutput.at(tmpSelectedOut).reference.SetQuantity(float(tmpQuantityMade));
    // Fetch the actual Item selected by the user.
    tmpOutput = tmpItemsForOutput.at(tmpSelectedOut).reference;

    // Add the newly created BOM object to the database.
    DB::BOM::AddBom(DB::BOM::BOM(id, name, items, tmpOutput));

    // Clear all input buffers.
    CancelAction();
}

/**
 * @brief   Modifies a BOM object that already exists in the database
 *          using the information inputted by the user.
 * @param   None
 * @retval  None
 */
void SaveEditedBom()
{
    // Get the string ID of the BOM.
    const std::string& id = tmpBom.GetId();
    // Create a string from the name input buffer.
    std::string name = tmpName;
    std::vector<DB::BOM::ItemReference> items;
    // For each Item in the temporary Item list:
    for (auto& item : tmpItems)
    {
        // If the Item's check box has been checked by the user:
        if (item.isSelected == true)
        {
            // Update the Item's quantity to be the one specified by the user, 
            // aka the quantity of that Item needed to make the BOM.
            item.reference.SetQuantity(item.quantity);
            // Add this Item to the BOM's Item list.
            items.emplace_back(item.reference);
        }
    }

    // Update the output Item's quantity to be the one specified by the user,
    // AKA the quantity of Items that are produced by this BOM.
    ItemRef newOut = tmpItemsForOutput.at(tmpSelectedOut);
    newOut.reference.SetQuantity(float(tmpQuantityMade));
    tmpOutput = newOut.reference;

    // Update the BOM in the database.
    DB::BOM::EditBom(tmpBom, DB::BOM::BOM(tmpBom.GetId(), name, items, tmpOutput));

    // Clear all temporary input variables.
    CancelAction();
}

/**
 * @brief   Delete the temporary BOM selected by the user.
 * @param   None
 * @retval  None
 */
void DeleteBom()
{
    // Delete the BOM from the database.
    DB::BOM::DeleteBom(tmpBom);

    // Clear all the temporary variables.
    CancelAction();
}

/**
 * @brief   Uses the BOM's recipe to make the designed Item.
 *          It automatically subtracts all the needed Items from the inventory
 *          as well as incrementing the Output Item's quantity by the number of Items made.
 * @param   None
 * @retval  None
 *
 * @note    The user must be logged in to complete the action,
 *          otherwise it will fail with a pop up appearing on the screen.
 */
void CommitMake()
{
    // If the user doesn't have write privileges in the database:
    if (DB::HasUserWritePrivileges() == false)
    {
        // Warn the user.
        Popup::Init("Unauthorized");
        Popup::AddCall(Popup::TextStylized,
                       "You must be logged in to do this action",
                       "Bold/4278190335",   // Display the text in bold and in red. (4278190335 -> 0xFF0000FF)
                       true);

        // Clear the temporary input variables.
        CancelAction();
        return;
    }

    // Get a reference to the output Item in the database.
    DB::Item::Item newIt = tmpBom.GetOutput();
    // #Todo Check if this does what it should (Increment the number of Items by the number of Items made).
    // Increment the Output Item's quantity.
    newIt.IncQuantity();
    // Update the output Item in the database.
    DB::Item::EditItem(tmpBom.GetOutput(), newIt);

    // For each Item in the BOM's list:
    for (auto& item : tmpBom.GetRawItems())
    {
        // Get the Item object from that Item Reference.
        newIt = item.GetItem();
        // #Todo Check if this does what it should (Decrement the number of Items by the number of Items made).
        // Decrement the Item's quantity by the quantity specified in the BOM.
        newIt.DecQuantity(item.GetQuantity());
        // Update the Item in the database.
        DB::Item::EditItem(item.GetItem(), newIt);
    }

    // Clear the temporary output variables.
    CancelAction();
}

/**
 * @brief   Handle everything related to the selection
 *          of Items to add to a BOM being created or edited.
 *          This includes:
 *              - Filtering/Searching for specific Items
 *              - Selecting/De-selecting Items
 *              - Selection of the quantity of Items needed
 *              - Re-arranging the order of the Items.
 * @param   None
 * @retval  None
 */
void HandlePopupItemPickerInput()
{
    // Flag that indicates if we should only draw Items that has their check box checked.
    static bool shouldOnlyShowSelected = false;
    // ID of the Item that is being clicked on by the user and that has not been released yet.
    static std::string itemClicked = "";
    // The time (in ms since the beginning of execution) that the Item has been clicked by the user.
    static double itemClickedTime = 0;
    // Flag that indicates if `itemClicked` is movable (Has been clicked on for more than 150ms.
    static bool isMoving = false;

    ImGui::Text("Items to use:");
    // Render and handle the filter bar.
    itemFilter.Render();
    ImGui::Checkbox("Only show selected items", &shouldOnlyShowSelected);

    ImVec2 size = ImVec2(0, ImGui::GetWindowHeight() - 250);
    ImGui::BeginChildFrame(ImGui::GetID("ItemPickerChildFrame"), size);
    float availWidth = ImGui::GetWindowContentRegionWidth();

    std::sort(tmpItems.begin(), tmpItems.end(),
              [](ItemRef& a, ItemRef& b)
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
    for (auto item = tmpItems.begin(); item != tmpItems.end(); item++)
    {
        // Only display the item if it matches the filter or 
        // if the "Only show selected items" checkbox is active (shouldOnlyShowSelected == true),
        // only display the item if it is included in the BOM.
        if ((itemFilter.CheckMatch(DB::Item::GetItemByID(item->reference.GetId())) == false &&
             itemFilter.CheckMatch(item->reference.GetItem(), 1) == false) ||
             (shouldOnlyShowSelected == true && item->isSelected == false))
        {
            continue;
        }
        ImGui::Separator();
        ImVec2 checkboxPos1 = ImGui::GetCursorScreenPos();
        ImVec2 checkboxPos2 = ImVec2(checkboxPos1.x + ImGui::GetFrameHeight(),
                                     checkboxPos1.y + ImGui::GetFrameHeight());

        if (ImGui::Checkbox(item->reference.GetId().c_str(), &item->isSelected) == true)
        {
            if (item->isSelected == true)
            {
                if (item->reference.GetPosition() >= tmpItems.size())
                {
                    item->reference.SetPosition(item->reference.GetPosition() - tmpItems.size());
                }
            }
            else
            {
                if (item->reference.GetPosition() < tmpItems.size())
                {
                    item->reference.SetPosition(item->reference.GetPosition() + tmpItems.size());
                }
            }
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) &&
            ImGui::IsMouseHoveringRect(checkboxPos1, checkboxPos2) == false)
        {
            ImGui::OpenPopup(std::string("##ItemDescriptionPopup" + item->reference.GetId()).c_str());
        }

        if (ImGui::BeginPopup(std::string("##ItemDescriptionPopup" + item->reference.GetId()).c_str()))
        {
            ImVec2 popupPos = ImGui::GetMousePosOnOpeningCurrentPopup();
            ImVec2 cursorPos = ImGui::GetMousePos();
            if (cursorPos.x > popupPos.x + 100 || cursorPos.x < popupPos.x - 100 ||
                cursorPos.y > popupPos.y + 20 || cursorPos.y < popupPos.y - 20 ||
                ImGui::IsMouseHoveringRect(checkboxPos1, checkboxPos2, false) == true)
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::Text(item->reference.GetItem().GetDescription().c_str());

            ImGui::EndPopup();
        }

        // Check if current row is clicked and has been clicked for more than 300ms.
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
            itemClicked = item->reference.GetId();
            itemClickedTime = ImGui::GetTime();
        }

        // If this item is the one we clicked on:
        if (item->reference.GetId() == itemClicked)
        {
            // If the left mouse button has been released:
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left) == false)
            {
                // Drop the item.
                itemClicked = "";
                itemClickedTime = 0;
                isMoving = false;
            }
            else
            {
                if ((ImGui::GetTime() >= itemClickedTime + 0.150) && isMoving == false)
                {
                    isMoving = true;
                }
                if (isMoving == true)
                {
                    // Check if mouse is above the item.
                    if (ImGui::GetMousePos().y < checkboxPos1.y)
                    {
                        if (item != tmpItems.begin())
                        {
                            // Don't let the user drag an unselected item above a selected item.
                            if ((item->isSelected == false &&
                                (item - 1)->isSelected == false) ||
                                 (item->isSelected == true))
                            {
                                int position = item->reference.GetPosition();
                                item->reference.SetPosition((item - 1)->reference.GetPosition());
                                (item - 1)->reference.SetPosition(position);

                                // If the two Items have the same IDs, We need to make them move somehow.
                                if (item->reference.GetPosition() == (item - 1)->reference.GetPosition())
                                {
                                    item->reference.SetPosition(position - 1);
                                }
                                Logging::System.Info("");
                                Logging::System.Info("Item " + item->reference.GetId() + "position is now ",
                                                     item->reference.GetPosition(), false);
                                Logging::System.Info("Item " + (item - 1)->reference.GetId() + "position is now ",
                                    (item - 1)->reference.GetPosition(), false);
                            }
                        }
                    }
                    else if (ImGui::GetMousePos().y > endOfFrame.y)
                    {
                        if ((item + 1) != tmpItems.end())
                        {
                        // Don't let the user drag a selected item bellow the last selected item.
                            if ((item->isSelected == true &&
                                (item + 1)->isSelected == true) ||
                                 (item->isSelected == false))
                            {
                                int position = item->reference.GetPosition();
                                item->reference.SetPosition((item + 1)->reference.GetPosition());
                                (item + 1)->reference.SetPosition(position);

                                // If the two Items have the same IDs, We need to make them move somehow.
                                if (item->reference.GetPosition() == (item + 1)->reference.GetPosition())
                                {
                                    item->reference.SetPosition((item + 1)->reference.GetPosition() + 1);
                                }

                                // If the Item moved out of visible area:
                                if (ImGui::IsItemVisible() == false)
                                {
                                    // Scroll down till it is visible.
                                    while (ImGui::IsItemVisible() == false);
                                }
                                Logging::System.Info("");
                                Logging::System.Info("Item " + item->reference.GetId() + "position is now ",
                                                     item->reference.GetPosition(), false);
                                Logging::System.Info("Item " + (item + 1)->reference.GetId() + "position is now ",
                                    (item + 1)->reference.GetPosition(), false);
                            }
                        }
                    }
                    ImGui::GetForegroundDrawList()->AddRectFilled(checkboxPos1, endOfFrame,
                                                                  ImGui::GetColorU32(ImGuiCol_ButtonActive, 0.3f));
                }
            }
        }

        ImGui::NextColumn();
        float available = DB::Item::GetItemByID(item->reference.GetId()).GetQuantity();
        ImGui::Text("%0.2f %s", available, DB::Item::GetItemByID(item->reference.GetId()).GetUnit());
        ImGui::NextColumn();
        ImGui::BeginChildFrame(ImGui::GetID(std::string("##QtyChildFrame" + item->reference.GetId()).c_str()),
                               ImVec2(0, ImGui::GetFrameHeightWithSpacing() + 5));
        ImU32 col = ImGui::GetColorU32(ImGuiCol_Text);
        if (available < item->quantity)
        {
            col = 0xFF0000FF;   // Red.
        }
        ImGui::PushStyleColor(ImGuiCol_Text, col);
        ImGui::InputFloat(std::string("##QtyInputFloat" + item->reference.GetId()).c_str(), &item->quantity, 1.0f, 10.0f);
        ImGui::PopStyleColor();
        ImGui::EndChildFrame();
        ImGui::NextColumn();
    }
    ImGui::EndChildFrame();

}

void HandlePopupOutputPickerInput()
{
    static FilterUtils::FilterHandler itFilter;

    if (ImGui::BeginCombo("Created Item", tmpItemsForOutput.at(tmpSelectedOut).reference.GetId().c_str()))
    {
        itFilter.Render();
        int currItem = -1;
        for (auto& item : tmpItemsForOutput)
        {
            currItem++;
            if (itFilter.CheckMatch(item.reference.GetItem(), 0) == false &&
                itFilter.CheckMatch(item.reference.GetItem(), 1) == false)
            {
                continue;
            }
            ImGui::Separator();
            if (ImGui::Selectable(item.reference.GetId().c_str()))
            {
                tmpSelectedOut = currItem;
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::TextWrapped(std::string("\t" + item.reference.GetItem().GetDescription()).c_str());
            }
        }
        ImGui::EndCombo();
    }
    else
    {
        itFilter.ClearText();
    }
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
    File::SaveFile(tmpOutputFilePath, FileType::INDEX_CSV, L"*.csv");

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
            << R"(")" << item.GetName() << R"(",)"
            << item.GetRawOutput().GetId() << R"(,")";
        for (auto& i : item.GetRawItems())
        {
            outputFile << i.GetId() << ":\t" << i.GetQuantity() * tmpQuantityToMake << " " << DB::Item::GetItemByID(i.GetId()).GetUnit() << std::endl;
        }

        outputFile << R"(")" << std::endl;
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
    ShellExecute(nullptr, nullptr,
                 StringUtils::LongStringToString(tmpOutputFilePath).c_str(),
                 nullptr, nullptr, SW_SHOW);
}
