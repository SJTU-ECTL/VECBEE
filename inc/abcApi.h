#ifndef ABC_API_H
#define ABC_API_H

namespace abc
{
#include <misc/util/abc_global.h>
#include <misc/util/util_hack.h>
#include <base/main/main.h>
#include <base/main/mainInt.h>
#include <base/io/ioAbc.h>
#include <map/mio/mio.h>
#include <map/mio/mioInt.h>
#include <map/mapper/mapper.h>
#include <map/mapper/mapperInt.h>
#include <bdd/extrab/extraBdd.h>
#include <bdd/cudd/cuddInt.h>


struct Abc_ManTime_t_
{
    Abc_Time_t     tArrDef;
    Abc_Time_t     tReqDef;
    Vec_Ptr_t  *   vArrs;
    Vec_Ptr_t  *   vReqs;
    Abc_Time_t     tInDriveDef;
    Abc_Time_t     tOutLoadDef;
    Abc_Time_t *   tInDrive;
    Abc_Time_t *   tOutLoad;
};


}

#endif // ABC_API_H
