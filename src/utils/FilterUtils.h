/**
 ******************************************************************************
 * @addtogroup FilterUtils
 * @{
 * @file    FilterUtils
 * @author  Samuel Martel
 * @brief   Header for the FilterUtils module.
 *
 * @date 1/27/2020 9:07:44 AM
 *
 ******************************************************************************
 */
#ifndef _FilterUtils
#define _FilterUtils

/*****************************************************************************/
/* Includes */
#include <iostream>
#include <vector>

namespace FilterUtils
{
/*****************************************************************************/
/* Exported defines */
#define MAX_INPUT_LENGHT 400


/*****************************************************************************/
/* Exported macro */


/*****************************************************************************/
/* Exported types */
class FilterHandler
{
public:
    FilterHandler() = default;
    FilterHandler(const std::vector<std::string>& categories);

    template<class T>
    bool CheckMatch(const T& item);

    void Render();

private:
    char m_filterText[MAX_INPUT_LENGHT] = { 0 };
    int m_selectedCategory = 0;
    std::vector<std::string> m_categories = {};
};

/*****************************************************************************/
/* Exported functions */

}
/* Have a wonderful day :) */
#endif /* _FilterUtils */
/**
 * @}
 */
/****** END OF FILE ******/
