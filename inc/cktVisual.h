#ifndef CKT_VISUAL_H
#define CKT_VISUAL_H


#include <bits/stdc++.h>
#include "abcApi.h"
#include "cktNtk.h"


void            WriteDotNtk           (abc::Abc_Ntk_t * pNtk, abc::Vec_Ptr_t * vNodes, abc::Vec_Ptr_t * vNodesShow, char * pFileName, int fGateNames, int fUseReverse);
char *          NtkPrintSop           (char * pSop);
int             NtkCountLogicNodes    (abc::Vec_Ptr_t * vNodes);
void            Visualize             (abc::Abc_Ntk_t * pAbcNtk, std::string fileName);
void            Visualize             (Ckt_Ntk_t & ckt, std::string fileName);


#endif
