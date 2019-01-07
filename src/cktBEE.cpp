#include "cktBEE.h"


using namespace std;
using namespace abc;


Ckt_Rpl_Pair_t::Ckt_Rpl_Pair_t(Ckt_Gate_t * p_ts, Ckt_Gate_t * p_ss)
    : pTS(p_ts), pSS(p_ss), addedER(0)
{
}


Ckt_Rpl_Pair_t::Ckt_Rpl_Pair_t(const Ckt_Rpl_Pair_t & other)
    : pTS(other.pTS), pSS(other.pSS), addedER(0)
{
}


Ckt_Rpl_Pair_t::~Ckt_Rpl_Pair_t(void)
{
}


std::ostream & operator << (ostream & os, const Ckt_Rpl_Pair_t & pr)
{
    cout << pr.pTS->GetName() << "\t" << pr.pSS->GetName() << "\t" << pr.addedER;
    return os;
}


void BatchErrorEstimation(Ckt_Gate_Net_t & ckt, Ckt_Gate_Net_t & cktRef)
{
    assert(&ckt != &cktRef);
    assert(ckt.GetAbcNtk() != cktRef.GetAbcNtk());
    assert(HasSamePo(ckt, cktRef));
    // get topological sequence
    vector <Ckt_Gate_t *> pOrderedObjs;
    ckt.SortObjects(pOrderedObjs);
    // update fanout cone information
    ckt.UpdateFoCone();
    // find cuts and sub-networks
    for (auto & pCktObj : pOrderedObjs) {
        FindCut(ckt, pCktObj->cut, *pCktObj);
        BuildSubNtk(pOrderedObjs, pCktObj->cutNtk);
    }
    // get base error rate & backup
    cktRef.GenInputDist(314);
    ckt.GenInputDist(314);
    cktRef.FeedForward();
    ckt.FeedForward(pOrderedObjs);
    int baseError = ckt.GetErrorRate(cktRef);
    ckt.BackupSimRes();
    // init parital difference
    for (int i = 0; i < ckt.GetPoNum(); ++i)
        ckt.GetPo(i)->SetBD(i, static_cast <uint64_t> (ULLONG_MAX));
    // get arrival time
    ckt.GetArrivalTime();
    // get candidiate pairs
    vector <Ckt_Rpl_Pair_t> pairs;
    GetValidPair(ckt, pOrderedObjs, pairs);
    // batch
    vector <uint64_t> isPoICorrect(ckt.GetPoNum(), 0);
    for (int fb = 0; fb < ckt.GetValClustersNum(); ++fb) {
        // check PO correctness
        uint64_t isCorrect = static_cast <uint64_t> (ULLONG_MAX);
        for (int i = 0; i < ckt.GetPoNum(); ++i) {
            isPoICorrect[i] = ~(ckt.GetPo(i)->GetCluster(fb) ^ cktRef.GetPo(i)->GetCluster(fb));
            isCorrect &= isPoICorrect[i];
        }
        // get partial boolean difference
        GetBooleanDifference(ckt, pOrderedObjs, isPoICorrect, fb);
        // update added error rate
        GetAddedErrorRate(ckt, pairs, fb, isCorrect);
    }
    // Visualize(ckt, "test.dot");
    for (auto & pr : pairs)
        cout << pr << endl;
    // ckt.PrintCut();
    // ckt.PrintCutNtk();
}


Ckt_Gate_t * CheckExpansion(list <Ckt_Gate_t *> & cut)
{
    for (auto ppCktObj1 = cut.begin(); ppCktObj1 != cut.end(); ++ppCktObj1) {
        auto ppCktObj2 = ppCktObj1;
        ++ppCktObj2;
        for (; ppCktObj2 != cut.end(); ++ppCktObj2) {
            assert((*ppCktObj1)->GetFoConeSize() == (*ppCktObj2)->GetFoConeSize());
            assert((*ppCktObj1)->GetTopoId() != (*ppCktObj2)->GetTopoId());
            for (int i = 0; i < (*ppCktObj1)->GetFoConeSize(); ++i) {
                if ((*ppCktObj1)->GetFoCone(i) & (*ppCktObj2)->GetFoCone(i)) {
                    Ckt_Gate_t * pRet = nullptr;
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


void Expand(Ckt_Gate_t & cktObj, list <Ckt_Gate_t *> & cut)
{
    for (int i = 0; i < cktObj.GetFanoutNum(); ++i) {
        Ckt_Gate_t * pCktFo = cktObj.GetFanout(i);
        if (!pCktFo->GetVisited() && !pCktFo->IsDanggling()) {
            cut.emplace_back(pCktFo);
            pCktFo->SetVisited();
        }
    }
}


void FindCut(Ckt_Gate_Net_t & ckt, list <Ckt_Gate_t *> & cut, Ckt_Gate_t & cktSrcObj)
{
    // init
    cut.clear();
    ckt.SetAllUnvisited2();
    // expand the source object
    Expand(cktSrcObj, cut);
    // expand until all objects in the cut are disjoint
    Ckt_Gate_t * pCktExpd = nullptr;
    while ((pCktExpd = CheckExpansion(cut)) != nullptr)
        Expand(*pCktExpd, cut);
}


void BuildSubNtk(vector <Ckt_Gate_t *> & pOrdObjs, list <Ckt_Gate_t *> & subNtk)
{
    subNtk.clear();
    for (auto & pCktObj : pOrdObjs)
        if (pCktObj->GetVisited())
            subNtk.emplace_back(pCktObj);
}


bool HasSamePo(Ckt_Gate_Net_t & ckt1, Ckt_Gate_Net_t & ckt2)
{
    if (ckt1.GetPoNum() != ckt2.GetPoNum())
        return false;
    for (int i = 0 ; i < ckt1.GetPoNum(); ++i)
        if (ckt1.GetPo(i)->GetName() != ckt2.GetPo(i)->GetName())
            return false;
    return true;
}


void GetBooleanDifference(Ckt_Gate_Net_t & ckt, vector <Ckt_Gate_t *> & pOrdObjs, vector <uint64_t> & isPoICorrect, int fb)
{
    for (auto ppCktObj = pOrdObjs.rbegin(); ppCktObj != pOrdObjs.rend(); ++ppCktObj) {
        if ((*ppCktObj)->cut.empty())
            continue;
        // init
        (*ppCktObj)->BDPlus = 0;
        (*ppCktObj)->BDMinus = static_cast <uint64_t> (ULLONG_MAX);
        for (int i = 0; i < ckt.GetPoNum(); ++i)
            (*ppCktObj)->SetBD(i, 0);
        // flip
        (*ppCktObj)->FlipCluster(fb);
        // simulate
        ckt.FeedForward((*ppCktObj)->cutNtk, fb);
        // record boolean difference
        for (auto & pCktObj : (*ppCktObj)->cut) {
            for (int i = 0; i < ckt.GetPoNum(); ++i) {
                (*ppCktObj)->SelfOrBD(
                    i,
                    pCktObj->XorClusterBak(fb) & pCktObj->GetBD(i)
                );
            }
        }
        for (int i = 0; i < ckt.GetPoNum(); ++i) {
            (*ppCktObj)->BDPlus |= (*ppCktObj)->GetBD(i);
            (*ppCktObj)->BDMinus &= (isPoICorrect[i] ^ (*ppCktObj)->GetBD(i));
        }
        // recover
        for (auto & pCktObj : (*ppCktObj)->cutNtk)
            pCktObj->RecoverCluster(fb);
        (*ppCktObj)->FlipCluster(fb);
    }
}


void GetAddedErrorRate(Ckt_Gate_Net_t & ckt, vector <Ckt_Rpl_Pair_t> & pairs, int fb, uint64_t isCorrect)
{
    for (auto & pr : pairs) {
        uint64_t isDiff = pr.pTS->GetCluster(fb) ^ pr.pSS->GetCluster(fb);
        pr.addedER += CountOneNum(isCorrect & pr.pTS->BDPlus & isDiff);
        pr.addedER -= CountOneNum(~isCorrect & pr.pTS->BDMinus & isDiff);
    }
}


void GetValidPair(Ckt_Gate_Net_t & ckt, vector <Ckt_Gate_t *> & pOrdObjs, std::vector <Ckt_Rpl_Pair_t> & pairs)
{
    pairs.clear();
    float invDelay = Mio_LibraryReadDelayInvRise(static_cast<Mio_Library_t *> (Abc_FrameReadLibGen()));
    for (auto & pCktTS : pOrdObjs) {
        if (pCktTS->IsDanggling() || pCktTS->IsPI() || pCktTS->IsPO() || pCktTS->IsConst())
            continue;
        for (auto & pCktSS : pOrdObjs) {
            if (pCktSS->IsAddedInv() || pCktSS->IsPO() || pCktTS == pCktSS)
                continue;
            if (pCktTS->GetArrivalTime() >= pCktSS->GetArrivalTime())
                pairs.emplace_back(Ckt_Rpl_Pair_t(pCktTS, pCktSS));

            if (pCktTS->IsInv() || pCktSS->IsInv() || pCktSS->IsConst())
                continue;
            if (pCktTS->GetArrivalTime() >= pCktSS->GetArrivalTime() + invDelay) {
                Ckt_Gate_t * pCktInv = ckt.GetInverter2(*pCktSS);
                pairs.emplace_back(Ckt_Rpl_Pair_t(pCktTS, pCktInv));
            }
        }
    }
}


void ReplaceTest(Ckt_Gate_Net_t & ckt)
{
    Ckt_Gate_Net_t cktRef(ckt.GetAbcNtk(), ckt.GetValClustersNum() * 64);
    assert(HasSamePo(ckt, cktRef));
    // get topological sequence
    vector <Ckt_Gate_t *> pOrderedObjs;
    ckt.SortObjects(pOrderedObjs);
    // get base error rate & backup
    cktRef.GenInputDist(314);
    ckt.GenInputDist(314);
    cktRef.FeedForward();
    ckt.FeedForward(pOrderedObjs);
    // get arrival time
    ckt.GetArrivalTime();
    // get candidate pairs
    vector <Ckt_Rpl_Pair_t> pairs;
    GetValidPair(ckt, pOrderedObjs, pairs);
    // replace and recover
    vector <Ckt_Rpl_Info_t> info;
    for (auto & pr : pairs) {
        ckt.Replace(*pr.pTS, *pr.pSS, info);
        ckt.FeedForward();
        cout << pr.pTS->GetName() << "\t" << pr.pSS->GetName() << "\t" << ckt.GetErrorRate(cktRef) << endl;
        ckt.RecoverFromRpl(info);
        // Ckt_Cec(cktRef, ckt);
    }
}



