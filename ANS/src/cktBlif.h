#ifndef CKT_BLIF_H
#define CKT_BLIF_H


#include <bits/stdc++.h>
#include "abcApi.h"
#include "cktGateNet.h"
#include "cktSopNet.h"


void Ckt_WriteBlif(Ckt_Gate_Net_t & ckt, const std::string & fileName);
void Ckt_WriteBlif(Ckt_Sop_Net_t & ckt, const std::string & fileName);
void Ckt_WriteBlif(abc::Abc_Ntk_t * pAbcNtk, const std::string & fileName);


#endif
