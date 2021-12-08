#include "cktSingSel.h"

using namespace std;
using namespace abc;


Ckt_Sing_Sel_Candi_t::Ckt_Sing_Sel_Candi_t(void)
    : pCktObj(nullptr), type(Ckt_Sop_Cat_t::INTER), addedER(0), newER(0), score(-FLT_MAX)
{
    SOP.clear();
}

Ckt_Sing_Sel_Candi_t::Ckt_Sing_Sel_Candi_t(Ckt_Sop_t * p_ckt_obj, vector <string> _sop, Ckt_Sop_Cat_t _type)
    : pCktObj(p_ckt_obj), type(_type), addedER(0), newER(0), score(-FLT_MAX)
{
    SOP.assign(_sop.begin(), _sop.end());
}


Ckt_Sing_Sel_Candi_t::Ckt_Sing_Sel_Candi_t(const Ckt_Sing_Sel_Candi_t & other)
    : pCktObj(other.pCktObj), type(other.type), addedER(other.addedER), score(other.score)
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


void Ckt_BatchErrorEstimation(Ckt_Sop_Net_t & ckt, Ckt_Sop_Net_t & cktRef, Ckt_Sing_Sel_Candi_t & res)
{
    clock_t t = clock();
    assert(&ckt != &cktRef);
    assert(ckt.GetAbcNtk() != cktRef.GetAbcNtk());
    assert(Ckt_HasSamePo(ckt, cktRef));
    // get topological sequence
    vector <Ckt_Sop_t *> pOrderedObjs;
    ckt.SortObjects(pOrderedObjs);
    // get candidiates
    vector <Ckt_Sing_Sel_Candi_t> candis;
    Ckt_GetALCs(ckt, pOrderedObjs, candis);
    cout << "#candidates = " << candis.size() << endl;
    if (candis.empty()) {
        cout << "Warning: no more ASE candidates" << endl;
        res.pCktObj = nullptr;
        return;
    }
    // build cut networks
    Ckt_BuildCutNtksFast(ckt, pOrderedObjs);
    cout << "Build cut network time = " << clock() - t << endl;
    // simulate base network
    random_device rd;
    unsigned seed = static_cast <unsigned>(rd());
    cktRef.GenInputDist(seed);
    ckt.GenInputDist(seed);
    cktRef.FeedForward();
    ckt.FeedForward(pOrderedObjs);
    int baseError = ckt.GetErrorRate(cktRef);
    // cout << "Base error = " << baseError << endl;
    // simulate cut networks
    for (auto & pCktObj : pOrderedObjs) {
        if (pCktObj->IsPI() || pCktObj->IsPO() || pCktObj->IsConst())
            continue;
        pCktObj->GetCutNtk()->FeedForwardCutNtk();
    }

    // t = clock();
    // init BD
    for (auto & pCktObj : pOrderedObjs) {
        pCktObj->ResizeBD();
        pCktObj->ResizeBDInc();
        pCktObj->ResizeBDDec();
        pCktObj->ResetBDInc();
        pCktObj->ResetBDDec();
    }
    // get BD & BDInc & BDDec
    vector <uint64_t> isPoCorrect(ckt.GetSimNum());
    vector <uint64_t> isCorrect(ckt.GetSimNum(), static_cast <uint64_t> (ULLONG_MAX));
    for (int i = 0; i < ckt.GetPoNum(); ++i) {
        Ckt_Sop_t * pCktPo = ckt.GetPo(i);
        Ckt_Sop_t * pRefCktPo = cktRef.GetPo(i);
        // init PO's boolean difference & get correctness of PO
        for (int j = 0; j < ckt.GetPoNum(); ++j) {
            if (i == j)
                ckt.GetPo(j)->SetBD();
           else
                ckt.GetPo(j)->ResetBD();
            for (int k = 0; k < ckt.GetSimNum(); ++k) {
                isPoCorrect[k] = ~(pCktPo->GetCluster(k) ^ pRefCktPo->GetCluster(k));
                isCorrect[k] &= isPoCorrect[k];
            }
        }
        // get boolean difference on one PO
        for (auto ppCktObj = pOrderedObjs.rbegin(); ppCktObj != pOrderedObjs.rend(); ++ppCktObj) {
            Ckt_Sop_t * pCktObj = *ppCktObj;
            if (pCktObj->IsPI() || pCktObj->IsPO() || pCktObj->IsConst())
                continue;
            // get influence on cuts
            Ckt_Sop_Net_t * pCutNtk = pCktObj->GetCutNtk();
            pCktObj->ResetBD();
            for (int k = 0; k < pCutNtk->GetPoNum(); ++k)
                pCktObj->UpdateBD(pCutNtk->GetPo(k));
            // record the influence on all POs
            pCktObj->UpdateBDInc();
            pCktObj->UpdateBDDec(isPoCorrect);
        }
    }

    // update added error rate
    vector <uint64_t> values(ckt.GetSimNum());
    for (auto & candi : candis) {
        Ckt_Sop_t * pCktObj = candi.pCktObj;
        // simulate the node, save the result in values
        pCktObj->GetClustersValue(candi.SOP, candi.type, values);
        // values ^= valueClusters
        pCktObj->XorClustersValue(values);
        // update error rate
        candi.addedER += pCktObj->GetIncER(isCorrect, values);
        candi.addedER -= pCktObj->GetDecER(isCorrect, values);
    }
    cout << "Get boolean difference time = " << clock() - t << endl;
    // find the best candidate
    res.score = -FLT_MAX;
    for (auto & candi : candis) {
        float newER = static_cast <float> (baseError + candi.addedER);
        float score = (candi.pCktObj->GetNLiterals() - GetLiteralsNum(candi.SOP)) / (newER + 1);
        // cout << candi << "\t" << newER << endl;
        if (score > res.score) {
            res.pCktObj = candi.pCktObj;
            res.SOP.assign(candi.SOP.begin(), candi.SOP.end());
            res.type = candi.type;
            res.addedER = candi.addedER;
            res.newER = newER;
            res.score = score;
        }
    }
}


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


void Ckt_BuildCutNtks(Ckt_Sop_Net_t & ckt, vector <Ckt_Sop_t *> & pOrderedObjs)
{
    // clear cut networks
    ckt.ClearCutNtks();
    // update fanout cone information
    ckt.UpdateFoCone();
    // find cuts and sub-networks
    list <Ckt_Sop_t *> cut;
    list <Ckt_Sop_t *> subNtk;

    // Ckt_ClearCutNtks(ckt);
    for (auto & pCktObj : pOrderedObjs) {
        if (pCktObj->IsPI() || pCktObj->IsPO() || pCktObj->IsConst())
            continue;
        ckt.SetAllUnvisited2();
        Ckt_ObjFindCut(*pCktObj, cut);
        Ckt_CollectVisited(pOrderedObjs, subNtk);
        pCktObj->SetCutNtk(Ckt_CreateNtkFrom(*pCktObj, subNtk, cut));
    }
}


void Ckt_BuildCutNtksFast(Ckt_Sop_Net_t & ckt, vector <Ckt_Sop_t *> & pOrderedObjs)
{
    // clear cut networks
    ckt.ClearCutNtks();
    // update fanout cone information
    // ckt.UpdateFoCone();
    // find cuts and sub-networks
    list <Ckt_Sop_t *> cut;
    list <Ckt_Sop_t *> subNtk;

    // Ckt_ClearCutNtks(ckt);
    for (auto & pCktObj : pOrderedObjs) {
        if (pCktObj->IsPI() || pCktObj->IsPO() || pCktObj->IsConst())
            continue;
        // ckt.SetAllUnvisited2();
        // Ckt_ObjFindCut(*pCktObj, cut);
        cut.clear();
        Ckt_ObjExpand(*pCktObj, cut);
        // Ckt_CollectVisited(pOrderedObjs, subNtk);
        subNtk.clear();
        for (auto & cutEle: cut)
            subNtk.emplace_back(cutEle);
        pCktObj->SetCutNtk(Ckt_CreateNtkFrom(*pCktObj, subNtk, cut));
    }
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
    set <string> appearedPattern;
    tmp.clear();
    for (auto & pCktObj : pOrdObjs) {
        if (pCktObj->IsPI() || pCktObj->IsPO() || pCktObj->IsConst())
            continue;
        appearedPattern.clear();
        candis.emplace_back(Ckt_Sing_Sel_Candi_t(pCktObj, tmp, Ckt_Sop_Cat_t::CONST0));
        candis.emplace_back(Ckt_Sing_Sel_Candi_t(pCktObj, tmp, Ckt_Sop_Cat_t::CONST1));
        Ckt_GetALCsRecur(pCktObj, candis, 0, 0, 0, maxLiteralNum, appearedPattern);
    }
}


void Ckt_GetALCsRecur(Ckt_Sop_t * pCktObj, vector < Ckt_Sing_Sel_Candi_t > & candis, int i, int j, int n, int maxLiteralNum, set <string> & pattern)
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
        if (!tmp.empty() && n && !Ckt_CheckPattern(pattern, tmp))
            candis.emplace_back(Ckt_Sing_Sel_Candi_t(pCktObj, tmp, pCktObj->GetType()));
        return;
    }
    else if (j >= pCktObj->GetSOPISize(i))
        Ckt_GetALCsRecur(pCktObj, candis, i + 1, 0, n, maxLiteralNum, pattern);
    else {
        // do not change
        Ckt_GetALCsRecur(pCktObj, candis, i, j + 1, n, maxLiteralNum, pattern);
        // change
        if (pCktObj->GetSOPIJ(i, j) != '-') {
            char bak = pCktObj->GetSOPIJ(i, j);
            pCktObj->SetSOPIJ(i, j, '-');
            Ckt_GetALCsRecur(pCktObj, candis, i, j + 1, n + 1, maxLiteralNum, pattern);
            pCktObj->SetSOPIJ(i, j, bak);
        }
    }
}


void Ckt_EnumerateTest(Ckt_Sop_Net_t & ckt, Ckt_Sop_Net_t & cktRef)
{
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
    }
}


float Ckt_SingleSelectionOnce(Ckt_Sop_Net_t & ckt, Ckt_Sop_Net_t & cktRef, int EThres)
{
    // find best approximate simplified expression
    Ckt_Sing_Sel_Candi_t bestASE;
    static int tt;
    clock_t t = clock();
    Ckt_BatchErrorEstimation(ckt, cktRef, bestASE);
    tt += clock() - t;

    // change function
    if (bestASE.pCktObj == nullptr) {
        cout << "Warning: no more ASE candidates" << endl;
        return 1.0;
    }
    else {
        // cout << "Added error of best ASE = " << bestASE.addedER << endl;
        // cout << "New error of best ASE = " << bestASE.newER << endl;
        Ckt_Sing_Sel_Info_t rplInfo;
        if (bestASE.newER <= EThres) {
            bestASE.pCktObj->ReplaceBy(bestASE.SOP, bestASE.type, rplInfo);
            // // verify correctness of batch error estimation
            // vector <Ckt_Sop_t *> pOrderedObjs;
            // ckt.SortObjects(pOrderedObjs);
            // ckt.FeedForward(pOrderedObjs);
            // int newError = ckt.GetErrorRate(cktRef);
            // cout << "Best ASE = " << bestASE << "\t" << "newError = " << newError << endl;
            // assert(newError == bestASE.newER);
            // // measure area and delay
            // Ckt_Synthesis2(ckt);
        }
        else {
            cout << "single selection time = " << tt / 1000000.0 << endl;
        }
        return bestASE.newER;
    }
}


bool Ckt_CheckPattern(set <string> & pattern, vector <string> & SOP)
{
    string tmp("");
    for (auto & str : SOP)
        tmp += str;
    if (pattern.count(tmp))
        return true;
    else {
        pattern.insert(tmp);
        return false;
    }
}
