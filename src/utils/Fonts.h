/**
 ******************************************************************************
 * @addtogroup Fonts
 * @{
 * @file    Fonts
 * @author  Client Microdata
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

namespace Fonts
{
/*****************************************************************************/
/* Exported defines */
#define DEFAULT_FONT_SIZE   1   // Normal
#define IS_FONT_DEFAULT     (Fonts::GetActiveFontName().find("default")!=std::string::npos)


/*****************************************************************************/
/* Exported macro */


/*****************************************************************************/
/* Exported types */
    typedef enum
    {
        FONT_OK,
        FONT_INEXISTANT,
        FONT_NOTHING_TO_POP
    }FontStatusEnum_t;

    class Font
    {
    public:
        Font(ImFont* font, std::string name) :
            m_Font(font), m_Name(name)
        {

        }

        inline ImFont* GetFont(void) const
        {
            return m_Font;
        }

        inline std::string GetName(void) const
        {
            return m_Name;
        }

    private:
        ImFont* m_Font;
        std::string m_Name;
    };

/*****************************************************************************/
/* Exported functions */
    void Load(int fontSize = DEFAULT_FONT_SIZE);
    FontStatusEnum_t Get(ImFont* fontOut, int idx);
    FontStatusEnum_t Get(ImFont* fontOut, std::string name);
    std::string GetActiveFontName(void);
    FontStatusEnum_t Push(int idx);
    FontStatusEnum_t Push(std::string name);
    FontStatusEnum_t Pop();
    FontStatusEnum_t SetDefault(int idx);
    FontStatusEnum_t SetDefault(std::string name);
}
/* Have a wonderful day :) */
#endif /* _Fonts */
/**
 * @}
 */
/****** END OF FILE ******/
