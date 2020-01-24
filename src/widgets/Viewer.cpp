#include "Viewer.h"
#include "utils/Config.h"
#include "utils/db/MongoCore.h"
#include "utils/db/Category.h"
#include "utils/db/Item.h"
#include "utils/db/Bom.h"
#include "vendor/imgui/imgui.h"
#include "widgets/BomViewer.h"
#include "widgets/ItemViewer.h"
#include "widgets/Logger.h"

void Viewer::Init()
{
    // Look for URI in config file. 
    // If it is found, use it to connect to the database.
    // Otherwise, use the default parameter of `DB::Init`.
    auto uri = Config::GetField<std::string>("uri");
    if (uri.empty())
    {
        Logging::System.Error("No URI found in config file, using localhost database.");
        DB::Init();
    }
    else
    {
        DB::Init(uri);
    }
    DB::Category::Init();
    DB::Item::Init();
    DB::BOM::Init();
}

void Viewer::Render()
{
    ImGui::BeginTabBar("##TabBar");

    if (ImGui::BeginTabItem("Inventory"))
    {
        ItemViewer::Render();
        ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("BOM"))
    {
        BomViewer::Render();
        ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
}
