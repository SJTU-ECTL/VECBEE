#include "cktNtk.h"


using namespace std;
using namespace abc;


Ckt_Ntk_t::Ckt_Ntk_t(Abc_Ntk_t * p_abc_ntk)
    : pAbcNtk(p_abc_ntk)
{
    Abc_Obj_t * pAbcObj;
    int i;
    Abc_NtkForEachObj(p_abc_ntk, pAbcObj, i)
        cktObjs.emplace_back(Ckt_Obj_t(pAbcObj));
}


Ckt_Ntk_t::~Ckt_Ntk_t(void)
{
}


void Ckt_Ntk_t::PrintInfo(void) const
{
    cout << "---------------- Network information ----------------" << endl;
    cout << "Name\tType" << endl;
    for (auto & obj : cktObjs) {
        cout << obj.GetName() << "\t" << obj.GetObjType() << endl;
    }
}
