#include "MainMenu.h"
#include "vendor/json/json.hpp"
#include "widgets/Logger.h"
#include "widgets/Options.h"
#include "widgets/CategoryViewer.h"
#include "widgets/Login.h"


MainMenu::MainMenu(void)
= default;
static void DrawStyleEditor();
static bool isEditorActive = false;

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
}

void MainMenu::FileMenu(void)
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

void MainMenu::EditMenu(void)
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

void MainMenu::HelpMenu(void)
{
    if (ImGui::MenuItem("User Guide"))
    {
    }
    if (ImGui::MenuItem("About"))
    {
    }
}

void DrawStyleEditor()
{
    ImGui::Begin("Style Editor", &isEditorActive);

    ImGui::ShowStyleEditor();

    ImGui::End();
}
