#include "cktObj.h"


using namespace std;
using namespace abc;


Ckt_Obj_t::Ckt_Obj_t(Abc_Obj_t * p_abc_obj, shared_ptr <Ckt_Ntk_t> p_ckt_ntk)
{
    pAbcObj = p_abc_obj;
    pCktNtk = p_ckt_ntk;
    simValues.clear();
    pCktFanins.clear();
    pCktFanouts.clear();
}


Ckt_Obj_t::~Ckt_Obj_t(void)
{
}
