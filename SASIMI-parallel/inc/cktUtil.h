#ifndef CKT_UTIL_H
#define CKT_UTIL_H


#include "headers.h"
#include "abcApi.h"
#include "debugAssert.h"


// evaluate
void Ckt_EvalASIC(Abc_Ntk_t * pNtk, std::string fileName, double maxDelay, bool isOutput = false);
void Ckt_EvalASIC(IN Abc_Ntk_t * pNtk, IN std::string && fileName, IN double maxDelay, OUT float & area, OUT float & delay);
void Ckt_EvalFPGA(Abc_Ntk_t * pNtk, std::string fileName, std::string map = "strash;if -K 6 -a;");
float Ckt_GetArea(Abc_Ntk_t * pNtk);
float Ckt_GetDelay(Abc_Ntk_t * pNtk);

// timing
int Ckt_GetObjArrivalTime(Abc_Obj_t * pObj, int roundBit);

// misc
void Ckt_NtkRename(Abc_Ntk_t * pNtk, const char * name);
Abc_Obj_t * Ckt_GetConst(Abc_Ntk_t * pNtk, bool isConst1 = true);
Abc_Obj_t * Ckt_FindNodeByName(Abc_Ntk_t * pNtk, char * nodeName);
void Ckt_PrintNodeFunc(Abc_Obj_t * pNode);
void Ckt_PrintHopFunc(Hop_Obj_t * pHopObj, Vec_Ptr_t * vFanins);
void Ckt_WriteBlif(Abc_Ntk_t * pNtk, std::string fileName);
void Ckt_WriteDot(Abc_Ntk_t * pNtk, std::string fileName, std::unordered_map <std::string, int> & name2Level, std::vector <std::string> & lastLACs, std::vector <Abc_Ntk_t *> & pNtks);
void Ckt_WriteDotNtk(Abc_Ntk_t * pNtk, Vec_Ptr_t * vNodes, Vec_Ptr_t * vNodesShow, char * pFileName, int fGateNames, int fUseReverse, std::unordered_map <std::string, int> & name2Level, std::vector <std::string> & lastLACs, std::vector <Abc_Ntk_t *> & pNtks);
void Ckt_PrintSop(std::string sop);
void Ckt_PrintNodes(Vec_Ptr_t * vFanins);
void Ckt_PrintFanins(Abc_Obj_t * pObj);
void Ckt_ReplaceByName(Abc_Ntk_t * pNtk, std::string tsName, std::string ssName);
int CallSystem(const std::string && cmd);


// extern
extern "C" {
void Abc_NtkShow(Abc_Ntk_t * pNtk, int fGateNames, int fSeq, int fUseReverse);
float Abc_NtkDelayTrace(Abc_Ntk_t * pNtk, Abc_Obj_t * pOut, Abc_Obj_t * pIn, int fPrint);
}


#endif
