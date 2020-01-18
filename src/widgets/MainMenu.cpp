#include "MainMenu.h"
#include "vendor/json/json.hpp"
#include "widgets/Logger.h"
#include "widgets/Options.h"


MainMenu::MainMenu(void)
= default;

void MainMenu::Process(void)
{
    if ( ImGui::BeginMenuBar() )
    {
        if ( ImGui::BeginMenu("File") )
        {
            FileMenu();
            ImGui::EndMenu();
        }
        if ( ImGui::BeginMenu("Edit") )
        {
            EditMenu();
            ImGui::EndMenu();
        }
        if ( ImGui::BeginMenu("Help") )
        {
            HelpMenu();
            ImGui::EndMenu();
        }
    }
    ImGui::EndMenuBar();
}

void MainMenu::FileMenu(void)
{
    if ( ImGui::MenuItem("Open Logger") )
    {
        Logging::OpenConsole();
    }
    ImGui::Separator();
    if ( ImGui::MenuItem("Options") )
    {
        Options::Open();
    }
}

void MainMenu::EditMenu(void)
{
}

void MainMenu::HelpMenu(void)
{
    if ( ImGui::MenuItem("User Guide") )
    {
    }
    if ( ImGui::MenuItem("About") )
    {
    }
}