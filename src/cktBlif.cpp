#include "cktBlif.h"


using namespace std;
using namespace abc;


void Ckt_WriteBlif(Ckt_Gate_Net_t & ckt, const string & fileName)
{
    Abc_Ntk_t * pNtkTemp = Abc_NtkToNetlist(Abc_NtkDup(ckt.GetAbcNtk()));
    Io_WriteBlif(pNtkTemp, const_cast<char *>(fileName.c_str()), 1, 1, 1);
    Abc_NtkDelete(pNtkTemp);
}


void Ckt_WriteBlif(Ckt_Sop_Net_t & ckt, const string & fileName)
{
    Abc_Ntk_t * pNtkTemp = Abc_NtkToNetlist(Abc_NtkDup(ckt.GetAbcNtk()));
    Io_WriteBlif(pNtkTemp, const_cast<char *>(fileName.c_str()), 1, 1, 1);
    Abc_NtkDelete(pNtkTemp);
}


void Ckt_WriteBlif(Abc_Ntk_t * pAbcNtk, const string & fileName)
{
    Abc_Ntk_t * pNtkTemp = Abc_NtkToNetlist(Abc_NtkDup(pAbcNtk));
    Io_WriteBlif(pNtkTemp, const_cast<char *>(fileName.c_str()), 1, 1, 1);
    Abc_NtkDelete(pNtkTemp);
}
