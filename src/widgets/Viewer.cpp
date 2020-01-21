#include "Viewer.h"
#include "utils/db/MongoCore.h"
#include "utils/db/Category.h"
#include "utils/db/Item.h"
#include "utils/Config.h"

void Viewer::Init()
{
    DB::Init(Config::GetField<std::string>("uri"));
    DB::Category::Init();
    DB::Item::Init();

}

void Viewer::Render()
{
}
