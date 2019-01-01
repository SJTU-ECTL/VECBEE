#ifndef CKT_BEE_H
#define CKT_BEE_H


#include "cktNtk.h"


void BatchErrorEstimation(Ckt_Ntk_t & ckt, Ckt_Ntk_t & cktRef);
Ckt_Obj_t * CheckExpansion(std::list <Ckt_Obj_t *> & cut);
void Expand(Ckt_Obj_t & cktObj, std::list <Ckt_Obj_t *> & cut);
void FindCut(Ckt_Ntk_t & ckt, std::list <Ckt_Obj_t *> & cut, Ckt_Obj_t & cktSrcObj);
void BuildSubNtk(std::vector <Ckt_Obj_t *> & pOrdObjs, std::list <Ckt_Obj_t *> & subNtk);
bool HasSamePo(Ckt_Ntk_t & ckt1, Ckt_Ntk_t & ckt2);
void GetBooleanDifference(Ckt_Ntk_t & ckt, std::vector <Ckt_Obj_t *> & pOrdObjs, std::vector <uint64_t> & isPoICorrect, int fb);


#endif
