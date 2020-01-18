#include "Options.h"
#include "utils/Config.h"
#include "utils/Fonts.h"
#include "vendor/imgui/imgui.h"
#include "widgets/Popup.h"
#include "widgets/Logger.h"


static bool isOpen = false;
static int logLevel = Logging::DEFAULT_LOG_LEVEL;
static int fontSize = DEFAULT_FONT_SIZE;

void Options::Init(void)
{
    try
    {
        logLevel = Config::GetField<int>("LogLevel");
    }
    catch ( std::invalid_argument )
    {
        Config::SetField("LogLevel", logLevel);
    }
    try
    {
        fontSize = Config::GetField<int>("FontSize");
    }
    catch ( std::invalid_argument )
    {
        Config::SetField("FontSize", fontSize);
    }
}

void Options::Open(void)
{
    isOpen = true;
}

void Options::Render(void)
{
    static const char* fontSizes[] = { "Small", "Normal", "Large", "Extra Large" };
    static const char* logLevels[] = { "Debug", "Info", "Warning", "Error", "Critical", "None" };
    static const char* currentFontSize = fontSizes[fontSize];
    static const char* currentLogLevel = logLevels[logLevel];

    if ( isOpen == false )
    {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_FirstUseEver);

    if ( !ImGui::Begin("Options", &isOpen) )
    {
        ImGui::End();
        return;
    }

    if ( ImGui::BeginCombo("Logging Level", currentLogLevel) )
    {
        for ( int n = 0; n < IM_ARRAYSIZE(logLevels); n++ )
        {
            bool is_selected = (currentLogLevel == logLevels[n]);
            if ( ImGui::Selectable(logLevels[n], is_selected) )
            {
                currentLogLevel = logLevels[n];
                logLevel = n;
                Logging::SetLogLevel(Logging::LogLevelEnum_t(n));
                Config::SetField("LogLevel", logLevel);
            }
            if ( is_selected == true )
            {
                // Set the initial focus when opening the combo.
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    if ( ImGui::BeginCombo("Font Size", currentFontSize) )
    {
        ImGuiSelectableFlags flags = IS_FONT_DEFAULT == true ?
            ImGuiSelectableFlags_Disabled : ImGuiSelectableFlags_None;

        for ( int n = 0; n < IM_ARRAYSIZE(fontSizes); n++ )
        {
            bool is_selected = (currentFontSize == fontSizes[n]);
            if ( ImGui::Selectable(fontSizes[n], is_selected) )
            {
                currentFontSize = fontSizes[n];
                fontSize = n;
                Config::SetField("FontSize", fontSize);
                Popup::Init("Font Size Changed");
                Popup::AddCall(Popup::TextCentered, "The software must be restarted\n"
                               "for the changes to take effect");
            }
            if ( is_selected == true )
            {
                // Set the initial focus when opening the combo.
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }


    ImGui::End();
}
