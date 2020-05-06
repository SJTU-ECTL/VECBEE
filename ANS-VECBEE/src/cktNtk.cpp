#include "cktNtk.h"


using namespace std;
using namespace abc;


Ckt_Ntk_t::Ckt_Ntk_t(Abc_Ntk_t * p_abc_ntk, int nFrames)
{
    pAbcNtk = Abc_NtkDup(p_abc_ntk);
    nSim = nFrames / 64;
    Abc_Obj_t * pAbcObj;
    int i;
    Abc_NtkForEachObj(pAbcNtk, pAbcObj, i) {
        // shared_ptr <Ckt_Obj_t> pCktObj(new Ckt_Obj_t(pAbcObj, shared_ptr <Ckt_Ntk_t> (this)));
        cout << Abc_ObjName(pAbcObj) << endl;
        new Ckt_Obj_t(pAbcObj, shared_from_this());
        // pAbcObj->pTemp = static_cast <void *> (pCktObj.get());
    }
}


Ckt_Ntk_t::~Ckt_Ntk_t(void)
{
}
