/**
 ******************************************************************************
 * @addtogroup UsirTP
 * @{
 * @file    main.cpp
 * @author  Samuel Martel
 * @brief   main application file
 *
 * @date January 2nd 2020, 10:40
 *
 ******************************************************************************
 */

 /* Includes */
#include "Application.h"
#include <iostream>

/* Private defines */

/* Private variable declarations */


/* Private function declarations */


int main(int argc, char** argv)
{
    Application app;
    if (app.GetHasError() == true)
    {
        std::cout << "Unable to initialize application" << std::endl;
        return -1;
    }

    app.Run();

    return 0;
}

/* Private function definitions */

/* Have a wonderful day :) */
/**
* @}
*/
/****** END OF FILE ******/
