/**
 * @file Fonts.cpp
 *
 * @author Samuel Martel
 * @date February 2020
 *
 * @brief Source for the Fonts module.
 *
 */

#include "Fonts.h"
#include "utils/Document.h"
#include "widgets/Logger.h"

// Cache for all the loaded fonts.
static std::vector<Fonts::Font> fonts;
// Pointer to the font that is currently loaded.
static ImFont* loadedFont = nullptr;
// The name of the font that is currently loaded.
// Idk why I didn't just use a Font object for that instead *shrugs*.
static std::string loadedFontName = "";
// The name of the default font used by ImGui.
static std::string defaultFontName = "default";

static int FindFontIndex(std::string fontName);

/**
 * @namespace    Fonts
 * @brief        Namespace containing everything related to Fonts
 */
namespace Fonts
{
/**
 * @brief   Load all fonts that can be found in the "/res/fonts" directory using the `fontSize`
 * @param   fontSize: The index of the size to use.
 *          - 0: 13.0f
 *          - 1: 16.0f
 *          - 2: 18.0f
 *          - 3: 20.0f
 * @retval  None
 */
void Load(int fontSize)
{
    // Hardcoded font sizes.
    static const float fontSizes[] = { 13.0f, 16.0f, 18.0f, 20.0f };
    // Get the absolute path of the font directory.
    std::string fontDirPath = File::GetCurrentPath() + "/res/fonts";

    // Get all the files located in the font directory.
    std::vector<std::string> fontFilePaths = File::GetFilesInDir(fontDirPath);

    // Get the general configuration structure of the ImGui context.
    ImGuiIO& io = ImGui::GetIO();

    // Load default font so we at least have that one in the cache.
    io.Fonts->AddFontDefault();
    fonts.emplace_back(Fonts::Font(io.Fonts->Fonts[0], "default"));

    // For each file found in the fonts directory:
    for (const std::string& fontFilePath : fontFilePaths)
    {
        // Make ImGui try to load the file as a font and get the data from it.
        ImFont* fontData = io.Fonts->AddFontFromFileTTF(fontFilePath.c_str(), fontSizes[fontSize]);
        // Get the filename out of the file's path.
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
    // For each Font in the cache:
    for (const Fonts::Font& font : fonts)
    {
        // If the font's name contains "Regular", use that font.
        if (font.GetName().find("Regular") != std::string::npos)
        {
            io.FontDefault = font.GetFont();
            defaultFontName = font.GetName();
            break;
        }
    }

    // If, during the above loop, we didn't find a font to use,
    // the default ImGui font will already be loaded, so no actions are needed.
}

/**
 * @brief   Fetch a Font from the cache by its ID.
 * @param   fontOut: A pointer where the Font, if found, will be stored.
 * @param   idx: The index in cache where the desired Font is located.
 * @retval  `FontStatus::FONT_OK` if the Font is found, `FontStatus::FONT_INEXISTANT` otherwise.
 */
FontStatus Fonts::Get(ImFont* fontOut, int idx)
{
    // If `idx` is greater or equal to the number of Fonts in the cache, we won't find anything.
    if (idx >= fonts.size())
    {
        return FontStatus::FONT_INEXISTANT;
    }

    // If `idx` is valid, get the font.
    fontOut = fonts[idx].GetFont();

    return FontStatus::FONT_OK;
}

/**
 * @brief   Fetch a Font from the cache by its name.
 * @param   fontOut: A pointer where the Font, if found, will be stored.
 * @param   name: The name of the Font to fetch.
 * @retval  `FontStatus::FONT_OK` if the Font is found, `FontStatus::FONT_INEXISTANT` otherwise.
 */
FontStatus Fonts::Get(ImFont* fontOut, std::string name)
{
    // Get an ID from the name.
    int idx = FindFontIndex(name);

    // Fetch the Font by its ID.
    return Get(fontOut, idx);
}

/**
 * @brief   Get the name of the Font that is currently loaded.
 * @param   None
 * @retval  The name of that Font.
 */
std::string GetActiveFontName()
{
    // If there is a Font loaded:
    if (loadedFont != nullptr)
    {
        // Return its name.
        return loadedFontName;
    }
    // Otherwise:
    else
    {
        // Return the default font's name.
        return defaultFontName;
    }
}

/**
 * @brief   Temporarily overrides ImGui's default font by the one corresponding to `idx` in the cache.
 * @param   idx: The position in cache where the desired Font is located.
 * @retval  `FontStatus::FONT_OK` if the Font is loaded successfully, `FontStatus::FONT_INEXISTANT` otherwise.
 *
 * @note    Don't forget to pop any Fonts you push!
 */
FontStatus Fonts::Push(int idx)
{
    // If `idx` is greater than or equal to the size of the cache, we won't find the Font in there.
    if (idx >= fonts.size())
    {
        return FontStatus::FONT_INEXISTANT;
    }

    // If there's currently a Font loaded:
    if (loadedFont != nullptr)
    {
        // Pop it from the ImGui Stack.
        Fonts::Pop();
    }

    // Update the current Font's information.
    loadedFont = fonts[idx].GetFont();
    loadedFontName = fonts[idx].GetName();
    // Push the Font on the ImGui stack.
    ImGui::PushFont(loadedFont);
    return FontStatus::FONT_OK;
}

/**
 * @brief   Temporarily overrides ImGui's default font by the one passed as parameter.
 * @param   name: The name of the Font to load.
 * @retval  `FontStatus::FONT_OK` if the Font is loaded successfully, `FontStatus::FONT_INEXISTANT` otherwise.
 *
 * @note    Don't forget to pop any Fonts you push!
 */
FontStatus Fonts::Push(std::string name)
{
    int idx = FindFontIndex(name);

    return Push(idx);
}

/**
 * @brief   Pop any Fonts that have been pushed on the ImGui stack.
 * @param   None
 * @retval  `FontStatus::FONT_OK` if a Font has been popped,
 *          `FontStatus::FONT_NOTHING_TO_POP` otherwise.
 */
FontStatus Fonts::Pop()
{
    // If there's no Font to pop, well don't pop anything.
    if (loadedFont == nullptr)
    {
        return FontStatus::FONT_NOTHING_TO_POP;
    }

    // Clear the loaded Font.
    loadedFont = nullptr;
    loadedFontName = "";
    // Pop the Font from the ImGui stack.
    ImGui::PopFont();
    return FontStatus::FONT_OK;
}

/**
 * @brief   Update the Font used for all the text by ImGui by the one desired.
 * @param   idx: The index in cache where the desired Font is located.
 * @retval  `FontStatus::FONT_OK` if the operation is successful, `FontStatus::FONT_OK` otherwise.
 */
FontStatus SetDefault(int idx)
{
    // If `idx` is greater than or equal to the size of the cache, we won't find the Font in there.
    if (idx >= fonts.size())
    {
        return FontStatus::FONT_INEXISTANT;
    }

    // Get the ImGui structure containing all the configurations.
    ImGuiIO& io = ImGui::GetIO();

    // Update the default Font's information.
    io.FontDefault = fonts[idx].GetFont();
    defaultFontName = fonts[idx].GetName();
    return FontStatus::FONT_OK;
}

/**
 * @brief   Update the Font used for all the text by ImGui by the one desired.
 * @param   name: The name of the Font to use as the default one.
 * @retval  `FontStatus::FONT_OK` if the operation is successful, `FontStatus::FONT_OK` otherwise.
 */
FontStatus SetDefault(std::string name)
{
    int idx = FindFontIndex(name);

    return SetDefault(idx);
}
}

/**
 * @brief   Find, in the Font cache, a Font that has the same name as `fontName`.
 * @param   fontName: The name of the Font to search.
 * @retval  If a Font has been found: Its index in the cache.
 *          Otherwise: -1
 */
int FindFontIndex(std::string fontName)
{
    // Initial return value.
    int idx = -1;

    // For each Font in the cache:
    for (const Fonts::Font& font : fonts)
    {
        idx++;
        // If the Font's name matches `fontName`:
        if (font.GetName().find(fontName) != std::string::npos)
        {
            // We have the desired Font, bail out.
            break;
        }
    }
    // Return -1 or the index of the Font that was found.
    return idx;
}
