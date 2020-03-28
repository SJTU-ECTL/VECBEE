#ifndef CKT_TIMING_H
#define CKT_TIMING_H


#include <bits/stdc++.h>
#include "abcApi.h"
#include "cktGateNet.h"


float                           Abc_GetArrivalTime(abc::Abc_Ntk_t * pNtk);
void                            Abc_NtkTimePrepare(abc::Abc_Ntk_t * pNtk);
void                            Abc_NodeDelayTraceArrival(abc::Abc_Obj_t * pNode, abc::Vec_Int_t * vSlacks);
void                            Abc_ManTimeExpand(abc::Abc_ManTime_t * p, int nSize, int fProgressive);
abc::Abc_ManTime_t *            Abc_ManTimeStart(abc::Abc_Ntk_t * pNtk);

static inline void              Abc_NtkDelayTraceSetSlack(abc::Vec_Int_t * vSlacks, abc::Abc_Obj_t * pObj, int iFanin, float Num)   { abc::Vec_IntWriteEntry( vSlacks, abc::Vec_IntEntry(vSlacks, Abc_ObjId(pObj)) + iFanin, abc::Abc_Float2Int(Num) ); }
static inline abc::Abc_Time_t * Abc_NodeArrival(abc::Abc_Obj_t * pNode)                                                             { return (abc::Abc_Time_t *)pNode->pNtk->pManTime->vArrs->pArray[pNode->Id]; }
static inline float             Ckt_GetArrivalTime(Ckt_Gate_Net_t & ckt)                                                            { return Abc_GetArrivalTime(ckt.GetAbcNtk()); }
static inline float             Ckt_GetArrivalTime(Ckt_Gate_t & cktObj)                                                             { return (static_cast<abc::Abc_Time_t *>(cktObj.GetAbcObj()->pNtk->pManTime->vArrs->pArray[cktObj.GetAbcObj()->Id]))->Rise; }


#endif
