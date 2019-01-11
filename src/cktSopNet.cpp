#include "cktSopNet.h"


using namespace std;
using namespace abc;
using namespace boost;


Ckt_Sop_Net_t::Ckt_Sop_Net_t(Abc_Ntk_t * p_abc_ntk, int nFrames)
    : nValueClusters(nFrames / 64)
{
    Abc_Obj_t * pAbcObj, * pFanin;
    int i;

    assert(Abc_NtkIsSopLogic(p_abc_ntk));

    // duplicate network
    pAbcNtk = Abc_NtkDup(p_abc_ntk);

    // init circuit objects, use pTemp to temporarily store the reflection
    Abc_NtkForEachObj(pAbcNtk, pAbcObj, i) {
        cktObjs.emplace_back(Ckt_Sop_t(pAbcObj, this));
        pAbcObj->pTemp = static_cast <void *> (&(cktObjs.back()));
    }

    // get pointers of PI/PO/CONST0/CONST1
    pCktConst0 = pCktConst1 = nullptr;
    for (auto & obj : cktObjs) {
        if (obj.IsPI())
            pCktPis.emplace_back(&obj);
        else if (obj.IsPO())
            pCktPos.emplace_back(&obj);
        else if (obj.IsConst0())
            pCktConst0 = &obj;
        else if (obj.IsConst1())
            pCktConst1 = &obj;
    }
    if (pCktConst0 == nullptr) {
        cktObjs.emplace_back(Ckt_Sop_t(Abc_NtkCreateNodeConst0(pAbcNtk), this));
        pCktConst0 = &(cktObjs.back());
    }
    if (pCktConst1 == nullptr) {
        cktObjs.emplace_back(Ckt_Sop_t(Abc_NtkCreateNodeConst1(pAbcNtk), this));
        pCktConst1 = &(cktObjs.back());
    }

    // add fanin/fanout information, use information saved in pTemp
    for (auto & obj : cktObjs) {
        Abc_ObjForEachFanin(obj.GetAbcObj(), pFanin, i)
            obj.AddFanin(static_cast <Ckt_Sop_t *> (pFanin->pTemp));
    }
}


Ckt_Sop_Net_t::Ckt_Sop_Net_t(const Ckt_Sop_Net_t & other)
    : nValueClusters(other.nValueClusters)
{
    Abc_Obj_t * pAbcObj, * pFanin;
    int i;

    // deep copy
    pAbcNtk = Abc_NtkDup(other.pAbcNtk);

    // init circuit objects, use pTemp to temporarily store the reflection
    Abc_NtkForEachObj(pAbcNtk, pAbcObj, i) {
        cktObjs.emplace_back(Ckt_Sop_t(pAbcObj, this));
        pAbcObj->pTemp = static_cast <void *> (&(cktObjs.back()));
    }

    // get pointers of PI/PO/CONST0/CONST1
    pCktConst0 = pCktConst1 = nullptr;
    for (auto & obj : cktObjs) {
        if (obj.IsPI())
            pCktPis.emplace_back(&obj);
        else if (obj.IsPO())
            pCktPos.emplace_back(&obj);
        else if (obj.IsConst0())
            pCktConst0 = &obj;
        else if (obj.IsConst1())
            pCktConst1 = &obj;
    }
    if (pCktConst0 == nullptr) {
        cktObjs.emplace_back(Ckt_Sop_t(Abc_NtkCreateNodeConst0(pAbcNtk), this));
        pCktConst0 = &(cktObjs.back());
    }
    if (pCktConst1 == nullptr) {
        cktObjs.emplace_back(Ckt_Sop_t(Abc_NtkCreateNodeConst1(pAbcNtk), this));
        pCktConst1 = &(cktObjs.back());
    }

    // add fanin/fanout information, use information saved in pTemp
    for (auto & obj : cktObjs) {
        Abc_ObjForEachFanin(obj.GetAbcObj(), pFanin, i)
            obj.AddFanin(static_cast <Ckt_Sop_t *> (pFanin->pTemp));
    }
}


Ckt_Sop_Net_t::~Ckt_Sop_Net_t(void)
{
    // free network (includes ABC objects added by user)
    Abc_NtkDelete(pAbcNtk);
}


void Ckt_Sop_Net_t::PrintInfo(void) const
{
    cout << "---------------- Network information ----------------" << endl;
    cout << "Name\tType\tSOP" << endl;
    for (auto & obj : cktObjs) {
        cout << obj.GetName() << "\t" << obj.GetType() << "\t";
        obj.PrintPCN();
        cout << endl;
    }
}
