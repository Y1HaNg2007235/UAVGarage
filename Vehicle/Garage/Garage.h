#ifndef GARAGE_GARAGE_H
#define GARAGE_GARAGE_H

#include "Application/Tasks/BattRotate.h"
#include "Application/Tasks/BattPush.h"

namespace garage
{

// 车库整体聚合类：持有各个任务，统一初始化和更新。
class Garage
{
public:
    Garage();
    bool init();
    void battRotateUpdate();
    void battPushUpdate();

    BattRotateTask battRotate;
    BattPushTask battPush;
};

}

#endif
