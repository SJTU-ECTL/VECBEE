#include "cktBEE.h"


using namespace std;


void BatchErrorEstimation(Ckt_Ntk_t & ckt, Ckt_Ntk_t & cktRef)
{
    assert(&ckt != &cktRef);
    assert(ckt.GetAbcNtk() != cktRef.GetAbcNtk());
    // get topological sequence
    vector <Ckt_Obj_t *> pOrderedObjs;
    ckt.SortObjects(pOrderedObjs);
    // update fanout cone information
    ckt.UpdateFoCone();
    // find cuts and sub-networks
    for (auto & pCktObj : pOrderedObjs) {
        FindCut(ckt, pCktObj->cut, *pCktObj);
        BuildSubNtk(pOrderedObjs, pCktObj->cutNtk);
    }
    // get base error rate
    cktRef.GenInputDist(314);
    ckt.GenInputDist(314);
    cktRef.FeedForward();
    ckt.FeedForward(pOrderedObjs);
    float baseError = ckt.GetErrorRate(cktRef);
    // get arrival time
    ckt.GetArrivalTime();
}


Ckt_Obj_t * CheckExpansion(list <Ckt_Obj_t *> & cut)
{
    for (auto ppCktObj1 = cut.begin(); ppCktObj1 != cut.end(); ++ppCktObj1) {
        auto ppCktObj2 = ppCktObj1;
        ++ppCktObj2;
        for (; ppCktObj2 != cut.end(); ++ppCktObj2) {
            assert((*ppCktObj1)->GetFoConeSize() == (*ppCktObj2)->GetFoConeSize());
            assert((*ppCktObj1)->GetTopoId() != (*ppCktObj2)->GetTopoId());
            for (int i = 0; i < (*ppCktObj1)->GetFoConeSize(); ++i) {
                if ((*ppCktObj1)->GetFoCone(i) & (*ppCktObj2)->GetFoCone(i)) {
                    Ckt_Obj_t * pRet = nullptr;
                    if ((*ppCktObj1)->GetTopoId() < (*ppCktObj2)->GetTopoId()) {
                        pRet = *ppCktObj1;
                        cut.erase(ppCktObj1);
                    }
                    else {
                        pRet = *ppCktObj2;
                        cut.erase(ppCktObj2);
                    }
                    return pRet;
                }
            }
        }
    }
    return nullptr;
}


void Expand(Ckt_Obj_t & cktObj, list <Ckt_Obj_t *> & cut)
{
    for (int i = 0; i < cktObj.GetFanoutNum(); ++i) {
        Ckt_Obj_t * pCktFo = cktObj.GetFanout(i);
        if (!pCktFo->GetVisited() && !pCktFo->IsDanggling()) {
            cut.emplace_back(pCktFo);
            pCktFo->SetVisited();
        }
    }
}


void FindCut(Ckt_Ntk_t & ckt, list <Ckt_Obj_t *> & cut, Ckt_Obj_t & cktSrcObj)
{
    // init
    cut.clear();
    ckt.SetAllUnvisited2();
    // expand the source object
    Expand(cktSrcObj, cut);
    // expand until all objects in the cut are disjoint
    Ckt_Obj_t * pCktExpd = nullptr;
    while ((pCktExpd = CheckExpansion(cut)) != nullptr)
        Expand(*pCktExpd, cut);
}


void BuildSubNtk(vector <Ckt_Obj_t *> & pOrdObjs, list <Ckt_Obj_t *> & subNtk)
{
    subNtk.clear();
    for (auto & pCktObj : pOrdObjs)
        if (pCktObj->GetVisited())
            subNtk.emplace_back(pCktObj);
}
