#include "cktSingSel.h"

using namespace std;
using namespace abc;


Ckt_Sing_Sel_Candi_t::Ckt_Sing_Sel_Candi_t(Ckt_Sop_t * p_ckt_obj, vector <string> _sop, Ckt_Sop_Cat_t _type)
    : pCktObj(p_ckt_obj), type(_type), addedER(0)
{
    SOP.assign(_sop.begin(), _sop.end());
}


Ckt_Sing_Sel_Candi_t::Ckt_Sing_Sel_Candi_t(const Ckt_Sing_Sel_Candi_t & other)
    : pCktObj(other.pCktObj), type(other.type), addedER(other.addedER)
{
    SOP.assign(other.SOP.begin(), other.SOP.end());
}


Ckt_Sing_Sel_Candi_t::~Ckt_Sing_Sel_Candi_t(void)
{
}


ostream & operator << (ostream & os, const Ckt_Sing_Sel_Candi_t & candi)
{
    cout << candi.pCktObj->GetName() << "\t" << candi.SOP << "\t" << candi.type << "\t" << candi.addedER;
    return os;
}


// void Ckt_BatchErrorEstimation(Ckt_Sop_Net_t & ckt, Ckt_Sop_Net_t & cktRef)
// {
//     assert(&ckt != &cktRef);
//     assert(ckt.GetAbcNtk() != cktRef.GetAbcNtk());
//     assert(Ckt_HasSamePo(ckt, cktRef));
//     // get topological sequence
//     vector <Ckt_Sop_t *> pOrderedObjs;
//     ckt.SortObjects(pOrderedObjs);
//     // update fanout cone information
//     ckt.UpdateFoCone();
//     // find cuts and sub-networks
//     for (auto & pCktObj : pOrderedObjs) {
//         Ckt_FindCut(ckt, pCktObj->cut, *pCktObj);
//         Ckt_BuildSubNtk(pOrderedObjs, pCktObj->cutNtk);
//     }
//     // get base error rate & backup
//     cktRef.GenInputDist(314);
//     ckt.GenInputDist(314);
//     cktRef.FeedForward();
//     ckt.FeedForward(pOrderedObjs);
//     // int baseError = ckt.GetErrorRate(cktRef);
//     ckt.BackupSimRes();
//     // init parital difference
//     for (int i = 0; i < ckt.GetPoNum(); ++i)
//         ckt.GetPo(i)->SetBD(i, static_cast <uint64_t> (ULLONG_MAX));
//     // get candidiates
//     vector <Ckt_Sing_Sel_Candi_t> candis;
//     Ckt_GetALCs(ckt, pOrderedObjs, candis);
//     // batch
//     clock_t st, ed;
//     int t1 = 0, t2 = 0;
//     vector <uint64_t> isPoICorrect(ckt.GetPoNum(), 0);
//     for (int fb = 0; fb < ckt.GetSimNum(); ++fb) {
//         // check PO correctness
//         uint64_t isCorrect = static_cast <uint64_t> (ULLONG_MAX);
//         for (int i = 0; i < ckt.GetPoNum(); ++i) {
//             isPoICorrect[i] = ~(ckt.GetPo(i)->GetCluster(fb) ^ cktRef.GetPo(i)->GetCluster(fb));
//             isCorrect &= isPoICorrect[i];
//         }
//         // get partial boolean difference
//         st = clock();
//         Ckt_GetBooleanDifference(ckt, pOrderedObjs, isPoICorrect, fb);
//         ed = clock();
//         t1 += (ed - st);
//         // update added error rate
//         st = clock();
//         Ckt_GetAddedErrorRate(candis, fb, isCorrect);
//         ed = clock();
//         t2 += (ed - st);
//     }
//     // for (auto & pr : candis)
//     //     cout << pr << endl;
//     cout << t1 << endl << t2 << endl;
//     cout << "average size of cutNtks = " << ckt.GetAverNtkSize() << endl;
// }


Ckt_Sop_t * Ckt_CheckExpansion(list <Ckt_Sop_t *> & cut)
{
    for (auto ppCktObj1 = cut.begin(); ppCktObj1 != cut.end(); ++ppCktObj1) {
        auto ppCktObj2 = ppCktObj1;
        ++ppCktObj2;
        for (; ppCktObj2 != cut.end(); ++ppCktObj2) {
            assert((*ppCktObj1)->GetFoConeSize() == (*ppCktObj2)->GetFoConeSize());
            assert((*ppCktObj1)->GetTopoId() != (*ppCktObj2)->GetTopoId());
            for (int i = 0; i < (*ppCktObj1)->GetFoConeSize(); ++i) {
                if ((*ppCktObj1)->GetFoCone(i) & (*ppCktObj2)->GetFoCone(i)) {
                    Ckt_Sop_t * pRet = nullptr;
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


void Ckt_ObjExpand(Ckt_Sop_t & cktObj, list <Ckt_Sop_t *> & cut)
{
    for (int i = 0; i < cktObj.GetFanoutNum(); ++i) {
        Ckt_Sop_t * pCktFo = cktObj.GetFanout(i);
        if (!pCktFo->GetVisited() && !pCktFo->IsDanggling()) {
            cut.emplace_back(pCktFo);
            pCktFo->SetVisited();
        }
    }
}


void Ckt_CollectVisited(vector <Ckt_Sop_t *> & pOrdObjs, list <Ckt_Sop_t *> & subNtk)
{
    subNtk.clear();
    for (auto & pCktObj : pOrdObjs)
        if (pCktObj->GetVisited())
            subNtk.emplace_back(pCktObj);
}


void Ckt_ObjFindCut(Ckt_Sop_t & cktSrcObj, list <Ckt_Sop_t *> & cut)
{
    cut.clear();
    // expand the source object
    Ckt_ObjExpand(cktSrcObj, cut);
    // expand until all objects in the cut are disjoint
    Ckt_Sop_t * pCktExpd = nullptr;
    while ((pCktExpd = Ckt_CheckExpansion(cut)) != nullptr)
        Ckt_ObjExpand(*pCktExpd, cut);
}


void Ckt_BuildCutNtks(Ckt_Sop_Net_t & ckt)
{
    // get topological sequence
    vector <Ckt_Sop_t *> pOrderedObjs;
    ckt.SortObjects(pOrderedObjs);
    // update fanout cone information
    ckt.UpdateFoCone();
    // find cuts and sub-networks
    list <Ckt_Sop_t *> cut;
    list <Ckt_Sop_t *> subNtk;
    clock_t st = clock();
    for (auto & pCktObj : pOrderedObjs) {
        if (pCktObj->IsPI() || pCktObj->IsPO() || pCktObj->IsConst())
            continue;
        ckt.SetAllUnvisited2();
        Ckt_ObjFindCut(*pCktObj, cut);
        Ckt_CollectVisited(pOrderedObjs, subNtk);
        pCktObj->SetCutNtk(Ckt_CreateNtkFrom(*pCktObj, subNtk, cut));
    }
    cout << clock() - st << endl;
    st = clock();
    for (auto & pCktObj : pOrderedObjs) {
        if (pCktObj->IsPI() || pCktObj->IsPO() || pCktObj->IsConst())
            continue;
        Ckt_SimCutNtk(*(pCktObj->GetCutNtk()));
    }
    cout << clock() - st << endl;
}


void Ckt_SimCutNtk(Ckt_Sop_Net_t & cutNtk)
{
    // init value clusters of PI
    Ckt_Sop_t * pCktPi = cutNtk.GetPi(0);
    pCktPi->FlipClustersFrom(pCktPi->GetOriObj());
    // feed forward
    auto pCktObj = cutNtk.GetPCktObjs()->begin();
    ++pCktObj;
    for (; pCktObj != cutNtk.GetPCktObjs()->end(); ++ pCktObj)
        pCktObj->UpdateClusters();
}


Ckt_Sop_Net_t * Ckt_CreateNtkFrom(Ckt_Sop_t & cktSrcObj, std::list <Ckt_Sop_t *> & subNtk, std::list <Ckt_Sop_t *> & cut)
{
    // make sure objects are in the same network
    Ckt_Sop_Net_t * pCktFaNtk = cktSrcObj.GetCktNtk();
    for (auto & pCktObj : subNtk)
        assert(pCktFaNtk == pCktObj->GetCktNtk());
    for (auto & pCktObj : cut)
        assert(pCktFaNtk == pCktObj->GetCktNtk());
    // start an empty network
    Ckt_Sop_Net_t * pCutNtk = new Ckt_Sop_Net_t(cktSrcObj, subNtk, cut, pCktFaNtk->GetSimNum() * 64);
    assert(pCutNtk != nullptr);

    return pCutNtk;
}



bool Ckt_HasSamePo(Ckt_Sop_Net_t & ckt1, Ckt_Sop_Net_t & ckt2)
{
    if (ckt1.GetPoNum() != ckt2.GetPoNum())
        return false;
    for (int i = 0 ; i < ckt1.GetPoNum(); ++i)
        if (ckt1.GetPo(i)->GetName() != ckt2.GetPo(i)->GetName())
            return false;
    return true;
}


void Ckt_GetALCs(Ckt_Sop_Net_t & ckt, vector <Ckt_Sop_t *> & pOrdObjs, vector < Ckt_Sing_Sel_Candi_t > & candis, int maxLiteralNum)
{
    candis.clear();
    vector <string> tmp;
    tmp.clear();
    for (auto & pCktObj : pOrdObjs) {
        if (pCktObj->IsPI() || pCktObj->IsPO() || pCktObj->IsConst())
            continue;
        candis.emplace_back(Ckt_Sing_Sel_Candi_t(pCktObj, tmp, Ckt_Sop_Cat_t::CONST0));
        candis.emplace_back(Ckt_Sing_Sel_Candi_t(pCktObj, tmp, Ckt_Sop_Cat_t::CONST1));
        Ckt_GetALCsRecur(pCktObj, candis, 0, 0, 0, maxLiteralNum);
    }
}


void Ckt_GetALCsRecur(Ckt_Sop_t * pCktObj, vector < Ckt_Sing_Sel_Candi_t > & candis, int i, int j, int n, int maxLiteralNum)
{
    if (n > maxLiteralNum)
        return;
    else if (i >= pCktObj->GetSOPSize()) {
        vector <string> tmp;
        pCktObj->CopySOP(tmp);
        for (auto it = tmp.begin(); it != tmp.end(); ++it) {
            bool isAllDC = true;
            for (auto & ch : *it) {
                if (ch != '-') {
                    isAllDC = false;
                    break;
                }
            }
            if (isAllDC) {
                tmp.erase(it);
                --it;
            }
        }
        if (!tmp.empty() && n)
            candis.emplace_back(Ckt_Sing_Sel_Candi_t(pCktObj, tmp, pCktObj->GetType()));
        return;
    }
    else if (j >= pCktObj->GetSOPISize(i))
        Ckt_GetALCsRecur(pCktObj, candis, i + 1, 0, n);
    else {
        // do not change
        Ckt_GetALCsRecur(pCktObj, candis, i, j + 1, n);
        // change
        if (pCktObj->GetSOPIJ(i, j) != '-') {
            char bak = pCktObj->GetSOPIJ(i, j);
            pCktObj->SetSOPIJ(i, j, '-');
            Ckt_GetALCsRecur(pCktObj, candis, i, j + 1, n + 1);
            pCktObj->SetSOPIJ(i, j, bak);
        }
    }
}


// void Ckt_GetBooleanDifference(Ckt_Sop_Net_t & ckt, vector <Ckt_Sop_t *> & pOrdObjs, vector <uint64_t> & isPoICorrect, int fb)
// {
//     for (auto ppCktObj = pOrdObjs.rbegin(); ppCktObj != pOrdObjs.rend(); ++ppCktObj) {
//         if ((*ppCktObj)->cut.empty())
//             continue;
//         // init
//         (*ppCktObj)->BDPlus = 0;
//         (*ppCktObj)->BDMinus = static_cast <uint64_t> (ULLONG_MAX);
//         for (int i = 0; i < ckt.GetPoNum(); ++i)
//             (*ppCktObj)->SetBD(i, 0);
//         // flip
//         (*ppCktObj)->FlipCluster(fb);
//         // simulate (2/3 time of the function)
//         ckt.FeedForward((*ppCktObj)->cutNtk, fb);
//         // record boolean difference
//         for (auto & pCktObj : (*ppCktObj)->cut) {
//             for (int i = 0; i < ckt.GetPoNum(); ++i) {
//                 (*ppCktObj)->SelfOrBD(
//                     i,
//                     pCktObj->XorClusterBak(fb) & pCktObj->GetBD(i)
//                 );
//             }
//         }
//         for (int i = 0; i < ckt.GetPoNum(); ++i) {
//             (*ppCktObj)->BDPlus |= (*ppCktObj)->GetBD(i);
//             (*ppCktObj)->BDMinus &= (isPoICorrect[i] ^ (*ppCktObj)->GetBD(i));
//         }
//         // recover
//         for (auto & pCktObj : (*ppCktObj)->cutNtk)
//             pCktObj->RecoverCluster(fb);
//         (*ppCktObj)->FlipCluster(fb);
//     }
// }
//
//
// void Ckt_GetAddedErrorRate(vector <Ckt_Sing_Sel_Candi_t> & candis, int fb, uint64_t isCorrect)
// {
//     for (auto & candi: candis) {
//         uint64_t isDiff = candi.pCktObj->GetCluster(fb) ^ candi.pCktObj->GetClusterValue(candi.SOP, candi.type, fb);
//         candi.addedER += Ckt_CountOneNum(isCorrect & candi.pCktObj->BDPlus & isDiff);
//         candi.addedER -= Ckt_CountOneNum(~isCorrect & candi.pCktObj->BDMinus & isDiff);
//     }
// }



void Ckt_EnumerateTest(Ckt_Sop_Net_t & ckt)
{
    Ckt_Sop_Net_t cktRef(ckt);
    assert(Ckt_HasSamePo(ckt, cktRef));
    // get topological sequence
    vector <Ckt_Sop_t *> pOrderedObjs;
    ckt.SortObjects(pOrderedObjs);
    // get base error rate & backup
    cktRef.GenInputDist(314);
    ckt.GenInputDist(314);
    cktRef.FeedForward();
    ckt.FeedForward(pOrderedObjs);
    // get candidate candis
    vector <Ckt_Sing_Sel_Candi_t> candis;
    Ckt_GetALCs(ckt, pOrderedObjs, candis);
    // replace and recover
    Ckt_Sing_Sel_Info_t info;
    for (auto & candi: candis) {
        ckt.Replace(*(candi.pCktObj), candi.SOP, candi.type, info);
        ckt.FeedForward();
        candi.addedER = ckt.GetErrorRate(cktRef);
        cout << candi << endl;
        ckt.RecoverFromRpl(info);
        // Ckt_Cec(cktRef, ckt);
    }
}
