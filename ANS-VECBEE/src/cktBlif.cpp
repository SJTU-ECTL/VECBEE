#include "cktBlif.h"


void Ckt_WriteBlif(Ckt_Gate_Net_t & ckt, const std::string & fileName)
{
    abc::Abc_Ntk_t * pNtkTemp = Abc_NtkToNetlist(Abc_NtkDup(ckt.GetAbcNtk()));
    Io_WriteBlif(pNtkTemp, const_cast<char *>(fileName.c_str()), 1, 1, 1);
    Abc_NtkDelete(pNtkTemp);
}


void Ckt_WriteBlif(Ckt_Sop_Net_t & ckt, const std::string & fileName)
{
    abc::Abc_Ntk_t * pNtkTemp = Abc_NtkToNetlist(Abc_NtkDup(ckt.GetAbcNtk()));
    Io_WriteBlif(pNtkTemp, const_cast<char *>(fileName.c_str()), 1, 1, 1);
    Abc_NtkDelete(pNtkTemp);
}


void Ckt_WriteBlif(abc::Abc_Ntk_t * pAbcNtk, const std::string & fileName)
{
    abc::Abc_Ntk_t * pNtkTemp = Abc_NtkToNetlist(Abc_NtkDup(pAbcNtk));
    Io_WriteBlif(pNtkTemp, const_cast<char *>(fileName.c_str()), 1, 1, 1);
    Abc_NtkDelete(pNtkTemp);
}
