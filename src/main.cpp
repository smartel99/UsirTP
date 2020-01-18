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
#include <sstream>

#include "utils/db/MongoCore.h"
#include "utils/db/Category.h"

/* Private defines */

using bsoncxx::builder::stream::document;
/* Private variable declarations */


/* Private function declarations */


int main(int argc, char** argv)
{
    DB::Init();

    DB::Category::Init();

    DB::Category::AddCategory(DB::Category::Category("Default", "DFLT"));
    DB::Category::Category nc = DB::Category::Category("Default", "DFLT");
    nc = DB::Category::GetCategoryByName("Default");
    std::cout << "Edited Category: " << nc.GetName() << ", " << nc.GetPrefix() << std::endl;
    DB::Category::DeleteCategory(nc);
    nc = DB::Category::GetCategoryByName("Default");
    std::cout << "Edited Category: " << nc.GetName() << ", " << nc.GetPrefix() << std::endl;

    while (1);

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
