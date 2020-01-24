#include "Viewer.h"
#include "utils/Config.h"
#include "utils/db/MongoCore.h"
#include "utils/db/Category.h"
#include "utils/db/Item.h"
#include "utils/db/Bom.h"
#include "vendor/imgui/imgui.h"
#include "widgets/BomViewer.h"
#include "widgets/ItemViewer.h"

void Viewer::Init()
{
    DB::Init(Config::GetField<std::string>("uri"));
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
