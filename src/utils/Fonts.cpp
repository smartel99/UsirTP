#include "Fonts.h"
#include "utils/Document.h"
#include "widgets/Logger.h"

static std::vector<Fonts::Font> fonts;
static ImFont* loadedFont = NULL;
static std::string loadedFontName = "";
static std::string defaultFontName = "default";

static int FindFontIndex(std::string fontName);


namespace Fonts
{

    void Fonts::Load(int fontSize)
    {
        static const float fontSizes[] = { 13.0f, 16.0f, 18.0f, 20.0f };
        std::string fontDirPath = File::GetCurrentPath() + "/res/fonts";

        std::vector<std::string> fontFilePaths = File::GetFilesInDir(fontDirPath);

        ImGuiIO& io = ImGui::GetIO();
        
        // Load default font.
        io.Fonts->AddFontDefault();
        fonts.emplace_back(Fonts::Font(io.Fonts->Fonts[0], "default"));

        // Load all the fonts found in the font dir.
        for ( std::string fontFilePath : fontFilePaths )
        {
            ImFont* fontData = io.Fonts->AddFontFromFileTTF(fontFilePath.c_str(), fontSizes[fontSize]);
            std::string fontName = StringUtils::GetNameFromPath(fontFilePath);
            // Remove the font name to only leave the type (e.g. OpenSans-Bold -> Bold).
            // If fontName doesn't contain a '-', keep whole string.
            size_t idx = fontName.find_last_of("-") == std::string::npos ?
                0 :
                fontName.find_last_of("-") + 1;
            fontName = fontName.substr(idx);
            fonts.emplace_back(Fonts::Font(fontData, fontName));
        }

        // Find font to use as default.
        // Priority order is:
        //  - .ttf (in res/fonts) with "regular" in filename
        //  - Default font provided by ImGui
        for ( const Fonts::Font& font : fonts )
        {
            if ( font.GetName().find("Regular") != std::string::npos )
            {
                io.FontDefault = font.GetFont();
                defaultFontName = font.GetName();
                Logging::System.Debug("Found custom default font!");
                break;
            }
        }
    }

    FontStatusEnum_t Fonts::Get(ImFont* fontOut, int idx)
    {
        if ( idx >= fonts.size() )
        {
            return FONT_INEXISTANT;
        }

        fontOut = fonts[idx].GetFont();

        return FONT_OK;
    }

    FontStatusEnum_t Fonts::Get(ImFont* fontOut, std::string name)
    {
        int idx = FindFontIndex(name);

        return Get(fontOut, idx);
    }

    std::string GetActiveFontName(void)
    {
        if ( loadedFont != NULL )
        {
            return loadedFontName;
        }
        else
        {
            return defaultFontName;
        }
    }

    FontStatusEnum_t Fonts::Push(int idx)
    {
        if ( idx >= fonts.size() || defaultFontName.find("Blazed") != std::string::npos )
        {
            return FONT_INEXISTANT;
        }

        loadedFont = fonts[idx].GetFont();
        loadedFontName = fonts[idx].GetName();
        ImGui::PushFont(loadedFont);
        return FONT_OK;
    }

    FontStatusEnum_t Fonts::Push(std::string name)
    {
        int idx = FindFontIndex(name);

        return Push(idx);
    }

    FontStatusEnum_t Fonts::Pop()
    {
        if ( loadedFont == NULL )
        {
            return FONT_NOTHING_TO_POP;
        }
        loadedFont = NULL;
        loadedFontName = "";
        ImGui::PopFont();
        return FONT_OK;
    }

    FontStatusEnum_t SetDefault(int idx)
    {
        if ( idx >= fonts.size() )
        {
            return FONT_INEXISTANT;
        }

        ImGuiIO& io = ImGui::GetIO();

        io.FontDefault = fonts[idx].GetFont();
        defaultFontName = fonts[idx].GetName();
        return FONT_OK;
    }

    FontStatusEnum_t SetDefault(std::string name)
    {
        int idx = FindFontIndex(name);

        return SetDefault(idx);
    }
}

int FindFontIndex(std::string fontName)
{
    int idx = -1;

    for ( const Fonts::Font& font : fonts )
    {
        idx++;
        if ( font.GetName().find(fontName) != std::string::npos )
        {
            break;
        }
    }
    return idx;
}
