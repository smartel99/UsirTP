/**
 ******************************************************************************
 * @addtogroup Fonts
 * @{
 * @file    Fonts
 * @author  Samuel Martel
 * @brief   Header for the Fonts module.
 *
 * @date 1/13/2020 10:16:03 AM
 *
 ******************************************************************************
 */
#ifndef _Fonts
#define _Fonts

/*****************************************************************************/
/* Includes */
#include "vendor/imgui/imgui.h"
#include <vector>
#include <iostream>

/**
 * @namespace   Fonts
 * @brief       The namespace containing everything related to fonts.
 */
namespace Fonts
{
/*****************************************************************************/
/* Exported defines */
#define DEFAULT_FONT_SIZE   1   // Normal

// Check if the currently loaded font is the default ImGui font.
#define IS_FONT_DEFAULT     (Fonts::GetActiveFontName().find("default")!=std::string::npos)


/*****************************************************************************/
/* Exported macro */


/*****************************************************************************/
/* Exported types */

/**
 * @enum    FontStatus
 * @brief   Return status of the functions related to fonts operation.
 */
enum class FontStatus
{
    FONT_OK,
    FONT_INEXISTANT,
    FONT_NOTHING_TO_POP
};

/**
 * @class Font
 * @brief A class for storing a font as a ImFont and its name.
 */
class Font
{
public:
    /**
     * @brief   Instantiate Font with the passed values.
     * @param   font: A pointer to an ImFont object already loaded in memory.
     * @param   name: The name to assign to that Font.
     * @retval  The newly created instance.
     */
    Font(ImFont* font, std::string name) :
        m_Font(font), m_Name(name)
    {
    }

    /**
     * @brief   Get a pointer to the ImFont.
     * @param   None
     * @retval  The pointer to the ImFont.
     */
    inline ImFont* GetFont() const
    {
        return m_Font;
    }

    /**
     * @brief   Get the name of the Font.
     * @param   None
     * @retval  The name of the Font.
     */
    inline std::string GetName() const
    {
        return m_Name;
    }

private:
    ImFont* m_Font;     /**< A pointer to the font loaded in memory */
    std::string m_Name; /**< The name assigned to that font */
};

/*****************************************************************************/
/* Exported functions */
void Load(int fontSize = DEFAULT_FONT_SIZE);
FontStatus Get(ImFont* fontOut, int idx);
FontStatus Get(ImFont* fontOut, std::string name);
std::string GetActiveFontName(void);
FontStatus Push(int idx);
FontStatus Push(std::string name);
FontStatus Pop();
FontStatus SetDefault(int idx);
FontStatus SetDefault(std::string name);
}
/* Have a wonderful day :) */
#endif /* _Fonts */
/**
 * @}
 */
/****** END OF FILE ******/
