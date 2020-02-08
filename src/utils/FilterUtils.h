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

/**
 * @namespace   FilterUtils FilterUtils.h Filter Utils
 * @brief       The namespace containing all utility functions and class for
 *              filtering.
 */
namespace FilterUtils
{
/*****************************************************************************/
/* Exported defines */
#define MAX_INPUT_LENGTH 400


/*****************************************************************************/
/* Exported macro */


/*****************************************************************************/
/* Exported types */
/**
 * @class   FilterHandler
 * @brief   A handler for filtering functions on a set of objects
 */
class FilterHandler
{
public:
    //! Default constructor.
    FilterHandler() = default;
    //! Construct a filter with a list of categories to filter by.
    FilterHandler(const std::vector<std::string>& categories);

    /**
     * @brief   Check if the content of the `category` of the `item` matches the filter.
     * @param   item: The item to check.
     * @param   category: The category in the `item` to check.
     * @retval  `true` if the filter's text is found in the `category` of the `item`,
     *          `false` otherwise.
     *
     * @note    The definitions of this template are located in the source file.
     */
    template<class T>
    bool CheckMatch(const T& item, int category = -1);

    /**
     * @brief   Render the filter object and handles user inputs.
     * @param   None
     * @retval  None
     *
     * @note    This does not create an ImGui window but inserts itself
     *          wherever it is called. Calling this function outside of
     *          an ImGui context is undefined behavior.
     */
    void Render();

    /**
     * @brief   Empty the filter's input buffer.
     * @param   None
     * @retval  None
     */
    inline void ClearText()
    {
        memset(m_filterText, 0, sizeof(m_filterText));
    }

private:
    char m_filterText[MAX_INPUT_LENGTH] = { 0 };    /**< The text used by the filter */
    int m_selectedCategory = 0;                     /**< The category to use for the filtering */
    std::vector<std::string> m_categories = {};     /**< All the available categories to use as filters */
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
