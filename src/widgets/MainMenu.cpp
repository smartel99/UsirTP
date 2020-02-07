#include "MainMenu.h"
#include "vendor/json/json.hpp"
#include "widgets/Logger.h"
#include "widgets/Options.h"
#include "widgets/CategoryViewer.h"
#include "widgets/Login.h"
#include "widgets/Viewer.h"


MainMenu::MainMenu()
= default;
static void DrawStyleEditor();
static void DrawPerfMonitor();
static bool isEditorActive = false;
static bool isPerMonitorActive = false;

void MainMenu::Process()
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            FileMenu();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            EditMenu();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help"))
        {
            HelpMenu();
            ImGui::EndMenu();
        }
    }
    ImGui::EndMenuBar();

    if (isEditorActive == true)
    {
        DrawStyleEditor();
    }

    if (isPerMonitorActive == true)
    {
        DrawPerfMonitor();
    }
}

void MainMenu::FileMenu()
{
    if (ImGui::MenuItem("Login"))
    {
        Login::OpenPopup();
    }
    if (ImGui::MenuItem("Open Logger"))
    {
        Logging::OpenConsole();
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Options"))
    {
        Options::Open();
    }
}

void MainMenu::EditMenu()
{
    if (ImGui::MenuItem("Category Editor"))
    {
        CategoryViewer::Open();
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Style Editor"))
    {
        isEditorActive = true;
    }
}

void MainMenu::HelpMenu()
{

    if (ImGui::MenuItem("Change Log"))
    {
        Viewer::ShowChangeLog();
    }
    if (ImGui::MenuItem("Feedback/Suggestion"))
    {
        Viewer::ShowSuggestionBox();
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Performance Monitor"))
    {
        isPerMonitorActive = true;
    }
}

void DrawStyleEditor()
{
    ImGui::Begin("Style Editor", &isEditorActive);

    ImGui::ShowStyleEditor();

    ImGui::End();
}

void DrawPerfMonitor()
{
    ImGui::ShowMetricsWindow(&isPerMonitorActive);
}
