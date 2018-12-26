#ifndef CKT_CEC_H
#define CKT_CEC_H

#include <bits/stdc++.h>
#include "cktNtk.h"
#include "abcApi.h"


void Ckt_Cec(Ckt_Ntk_t & ckt1, Ckt_Ntk_t & ckt2);
void Abc_NtkCecFraig( abc::Abc_Ntk_t * pNtk1, abc::Abc_Ntk_t * pNtk2, int nSeconds, int fVerbose );
void Abc_NtkVerifyReportError( abc::Abc_Ntk_t * pNtk1, abc::Abc_Ntk_t * pNtk2, int * pModel );


#endif
