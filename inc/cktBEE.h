#ifndef CKT_BEE_H
#define CKT_BEE_H


#include "cktNtk.h"


void BatchErrorEstimation(Ckt_Ntk_t & ckt, Ckt_Ntk_t & cktRef);
Ckt_Obj_t * CheckExpansion(std::list <Ckt_Obj_t *> & cut);
void Expand(Ckt_Obj_t & cktObj, std::list <Ckt_Obj_t *> & cut, std::list <Ckt_Obj_t *> & subNtk);
void FindCut(Ckt_Ntk_t & ckt, std::list <Ckt_Obj_t *> & cut, std::list <Ckt_Obj_t *> & subNtk, Ckt_Obj_t & cktSrcObj);


#endif
