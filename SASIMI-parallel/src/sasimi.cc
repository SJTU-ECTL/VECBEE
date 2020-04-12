#include "sasimi.h"


using namespace std;


SASIMI_Manager_t::SASIMI_Manager_t(Abc_Ntk_t * pNtk, int nFrame, Metric_t metricType, double errorBound):
    nFrame(nFrame), metricType(metricType), errorBound(errorBound)
{
}


SASIMI_Manager_t::~SASIMI_Manager_t()
{
}


void SASIMI_Manager_t::GreedySelection(Abc_Ntk_t * pOriNtk)
{
    // init
    Abc_Ntk_t * pAppNtk = Abc_NtkDup(pOriNtk);
    PatchConst(pOriNtk);
    PatchConst(pAppNtk);
    Simulator_t oriSmlt(pOriNtk, nFrame);
    Simulator_t appSmlt(pAppNtk, nFrame);
    oriSmlt.Input("in/c880.in");
    oriSmlt.Simulate();
    appSmlt.Input("in/c880.in");

    // iteation
    double error = 0;
    vector < vector <tVec> > bds; // cpm[o][n][i]
    vector <Vec_Ptr_t *> vMffcs;
    while (error < errorBound) {
        appSmlt.Simulate();
        CollectMFFC(pAppNtk, vMffcs);
        Abc_NtkDelayTrace(pAppNtk, nullptr, nullptr, 0);
        GetCPM(oriSmlt, appSmlt, bds);
        CollectAllLACs(oriSmlt, appSmlt, bds, vMffcs);
        FreeMFFC(vMffcs);
        break;
    }

    // clean up
    Abc_NtkDelete(pAppNtk);
}


void SASIMI_Manager_t::PatchConst(Abc_Ntk_t * pNtk)
{
    Ckt_GetConst(pNtk, 0);
    Ckt_GetConst(pNtk, 1);
}


void SASIMI_Manager_t::CollectMFFC(IN Abc_Ntk_t * pAppNtk, OUT vector <Vec_Ptr_t *> & vMffcs)
{
    vMffcs.resize(Abc_NtkObjNum(pAppNtk) + 1);
    Abc_Obj_t * pNode = nullptr;
    int i = 0;
    Abc_NtkForEachNode(pAppNtk, pNode, i) {
        DASSERT(pNode->Id < static_cast <int> (vMffcs.size()));
        vMffcs[pNode->Id] = Vec_PtrAlloc(100);
        Abc_NodeDeref_rec( pNode );
        Abc_NodeMffcConeSupp( pNode, vMffcs[pNode->Id], nullptr );
        Abc_NodeRef_rec( pNode );
    }
    // Abc_NtkForEachNode(pAppNtk, pNode, i) {
    //     Abc_Obj_t * pObj = nullptr;
    //     int j = 0;
    //     cout << Abc_ObjName(pNode) << " tar,";
    //     Vec_PtrForEachEntry(Abc_Obj_t *, vMffcs[pNode->Id], pObj, j)
    //         cout << Abc_ObjName(pObj) << ",";
    //     cout << endl;
    // }
}


void SASIMI_Manager_t::FreeMFFC(vector <Vec_Ptr_t *> & vMffcs)
{
    for (auto & vNodeMffc: vMffcs) {
        if (vNodeMffc != nullptr) {
            Vec_PtrFree(vNodeMffc);
            vNodeMffc = nullptr;
        }
    }
}


void SASIMI_Manager_t::GetCPM(IN Simulator_t & oriSmlt, IN Simulator_t & appSmlt, OUT vector < vector <tVec> > & bds)
{
    // check
    Abc_Ntk_t * pOriNtk = oriSmlt.GetNetwork();
    Abc_Ntk_t * pAppNtk = appSmlt.GetNetwork();
    DASSERT(pOriNtk != pAppNtk);
    DASSERT(SmltChecker(&oriSmlt, &appSmlt));
    // get disjoint cuts and the corresponding networks
    appSmlt.BuildCutNtks();
    // simulate networks of disjoint cuts
    appSmlt.SimulateCutNtks();
    // topological sort
    Vec_Ptr_t * vNodes = Abc_NtkDfs(pAppNtk, 0);
    // init boolean difference
    int nBlock = appSmlt.GetBlockNum();
    bds.resize(Abc_NtkPoNum(pAppNtk));
    for (auto & bdPo: bds) {
        bdPo.resize(appSmlt.GetMaxId() + 1);
        for (auto & bdObjPo: bdPo)
            bdObjPo.resize(nBlock);
    }
    // compute boolean difference
    int i = 0;
    Abc_Obj_t * pAppPo = nullptr;
    Abc_NtkForEachPo(pAppNtk, pAppPo, i)
        appSmlt.UpdateBoolDiff(pAppPo, vNodes, bds[i]);
    // clean up
    Vec_PtrFree(vNodes);
    // Abc_NtkForEachPo(pAppNtk, pAppPo, i) {
    //     Abc_Obj_t * pObj = nullptr;
    //     int j = 0;
    //     Abc_NtkForEachNode(pAppNtk, pObj, j) {
    //         if (string(Abc_ObjName(pObj)) == "[36964]") {
    //             for (int k = 0; k < appSmlt.GetFrameNum(); ++k) {
    //                 cout << Abc_ObjName(pObj) << "," << Abc_ObjName(pAppPo) << "," << Ckt_GetBit(bds[i][pObj->Id][0], k) << endl;
    //             }
    //         }
    //     }
    // }
}


void SASIMI_Manager_t::CollectAllLACs(IN Simulator_t & oriSmlt, IN Simulator_t & appSmlt, IN vector < vector <tVec> > & bds, IN vector <Vec_Ptr_t *> & vMffcs)
{
    Abc_Ntk_t * pOriNtk = oriSmlt.GetNetwork();
    Abc_Ntk_t * pAppNtk = appSmlt.GetNetwork();
    // update PO correctness
    int nBlock = oriSmlt.GetBlockNum();
    vector <tVec> poKRight(Abc_NtkPoNum(pAppNtk), tVec(nBlock));
    tVec allPoRight(nBlock);
    for (int i = 0; i < nBlock; ++i)
        allPoRight[i] = static_cast <uint64_t> (ULLONG_MAX);
    for (int i = 0; i < Abc_NtkPoNum(pOriNtk); ++i) {
        Abc_Obj_t * pOriPo = Abc_NtkPo(pOriNtk, i);
        Abc_Obj_t * pAppPo = Abc_NtkPo(pAppNtk, i);
        for (int j = 0; j < nBlock; ++j) {
            poKRight[i][j] = ~(oriSmlt.GetValues(pOriPo, j) ^ appSmlt.GetValues(pAppPo, j));
            allPoRight[j] &= poKRight[i][j];
        }
    }
    // update increase/decrease flag
    vector <tVec> isERInc(Abc_NtkObjNum(pAppNtk) + 1, tVec(nBlock));
    vector <tVec> isERDec(Abc_NtkObjNum(pAppNtk) + 1, tVec(nBlock));
    Abc_Obj_t * pObj = nullptr;
    int i = 0;
    Abc_NtkForEachNode(pAppNtk, pObj, i) {
        for (int j = 0; j < nBlock; ++ j) {
            isERInc[pObj->Id][j] = 0;
            isERDec[pObj->Id][j] = static_cast <uint64_t> (ULLONG_MAX);
        }
    }
    for (int i = 0; i < Abc_NtkPoNum(pOriNtk); ++i) {
        int j = 0;
        Abc_NtkForEachNode(pAppNtk, pObj, j) {
            for (int k = 0; k < nBlock; ++k) {
                isERInc[pObj->Id][k] |= bds[i][pObj->Id][k];
                isERDec[pObj->Id][k] &= (poKRight[i][k] ^ bds[i][pObj->Id][k]);
            }
        }
    }
    uint64_t lastBlockMask = 0;
    for (int i = 0; i < appSmlt.GetLastBlockLen(); ++i)
        Ckt_SetBit(lastBlockMask, i);
    Abc_NtkForEachNode(pAppNtk, pObj, i) {
        for (int j = 0; j < nBlock; ++ j) {
            isERInc[pObj->Id][j] &= allPoRight[j];
            isERDec[pObj->Id][j] &= ~allPoRight[j];
            if (j == nBlock - 1) {
                isERInc[pObj->Id][j] &= lastBlockMask;
                isERDec[pObj->Id][j] &= lastBlockMask;
            }
        }
    }
    // update PI flag
    int sourceLen = (Abc_NtkPiNum(pAppNtk) >> 6) + 1;
    vector <tVec> sources(Abc_NtkObjNum(pAppNtk) + 1);
    Abc_NtkForEachObj(pAppNtk, pObj, i)
        sources[pObj->Id].resize(sourceLen , 0);
    Abc_NtkForEachPi(pAppNtk, pObj, i)
        Ckt_SetBit(sources[pObj->Id][i >> 6], i);
    Vec_Ptr_t * vNodes = Abc_NtkDfs(pAppNtk, 0);
    Vec_PtrForEachEntry(Abc_Obj_t *, vNodes, pObj, i) {
        Abc_Obj_t * pFanin = nullptr;
        int j = 0;
        Abc_ObjForEachFanin(pObj, pFanin, j) {
            for (int k = 0; k < sourceLen; ++k)
                sources[pObj->Id][k] |= sources[pFanin->Id][k];
        }
    }
    Vec_PtrFree(vNodes);
    // collect one LAC for each node
    int baseER = GetER(&oriSmlt, &appSmlt);
    Abc_NtkForEachNode(pAppNtk, pObj, i) {
        if (!Abc_NodeIsConst(pObj)) {
            CollectNodeLAC(pObj, appSmlt, isERInc, isERDec, sources, vMffcs, baseER);
        }
    }
}


void SASIMI_Manager_t::CollectNodeLAC(IN Abc_Obj_t * pTS, IN Simulator_t & appSmlt, IN vector <tVec> & isERInc, IN vector <tVec> & isERDec, IN vector <tVec> & sources, IN vector <Vec_Ptr_t *> & vMffcs, IN int baseER)
{
    Abc_Ntk_t * pAppNtk = appSmlt.GetNetwork();
    DASSERT(pAppNtk == pTS->pNtk);
    double errorBoundFrame = errorBound * appSmlt.GetFrameNum();
    double invDelay = Mio_LibraryReadDelayInvMax((Mio_Library_t *)Abc_FrameReadLibGen()) + 0.1;
    int areaInv = Mio_LibraryReadAreaInv((Mio_Library_t *)Abc_FrameReadLibGen());
    int areaBuf = Mio_LibraryReadAreaBuf((Mio_Library_t *)Abc_FrameReadLibGen());
    // consider constant replacement
    Abc_Obj_t * pConst0 = Ckt_GetConst(pAppNtk, 0);
    pair <int, int> dErrors;
    GetDError(appSmlt, pTS, pConst0, isERInc, isERDec, dErrors);
    if (baseER + dErrors.first <= errorBoundFrame) {
        double dArea = GetDArea(pTS, pConst0, vMffcs);
        cout << Abc_ObjName(pTS) << ",0," << dErrors.first / static_cast <double>(appSmlt.GetFrameNum()) << ",," << dArea << endl;
    }
    Abc_Obj_t * pConst1 = Ckt_GetConst(pAppNtk, 1);
    GetDError(appSmlt, pTS, pConst1, isERInc, isERDec, dErrors);
    if (baseER + dErrors.first <= errorBoundFrame) {
        double dArea = GetDArea(pTS, pConst1, vMffcs);
        cout << Abc_ObjName(pTS) << ",1," << dErrors.first / static_cast <double>(appSmlt.GetFrameNum()) << ",," << dArea << endl;
    }

    // consider other nodes' replacement
    Abc_Obj_t * pSS= nullptr;
    int i = 0;
    Abc_NtkForEachNode(pAppNtk, pSS, i) {
        if (Abc_NodeIsConst(pSS))
            continue;
        if (Ckt_GetObjArrivalTime(pSS, 3) > Ckt_GetObjArrivalTime(pTS, 3))
            continue;
        if (pTS == pSS)
            continue;
        DASSERT(sources[pTS->Id].size() == sources[pSS->Id].size());
        bool isSkip = true;
        for (int j = 0; j < static_cast <int>(sources[pTS->Id].size()); ++j) {
            if (sources[pTS->Id][j] & sources[pSS->Id][j]) {
                isSkip = false;
                break;
            }
        }
        if (isSkip)
            continue;
        if (Abc_NodeIsBuf(pTS) && Abc_ObjFanin0(pTS) == pSS)
            continue;
        Abc_Obj_t * pFanin = nullptr;
        int j = 0;
        isSkip = false;
        Abc_ObjForEachFanin(pTS, pFanin, j) {
            if (pFanin == pSS) {
                isSkip = true;
                break;
            }
        }
        if (isSkip)
            continue;
        GetDError(appSmlt, pTS, pSS, isERInc, isERDec, dErrors);
        if (baseER + dErrors.first <= errorBoundFrame ||
           (baseER + dErrors.second <= errorBoundFrame && Ckt_GetObjArrivalTime(pTS, 3) >= Ckt_GetObjArrivalTime(pSS, 3) +  invDelay * 1000)) {
            double dArea = GetDArea(pTS, pSS, vMffcs);
            if (dErrors.first <= dErrors.second) {
                Abc_Obj_t * pFanout = nullptr;
                int j = 0;
                bool isPoDriver = false;
                Abc_ObjForEachFanout(pSS, pFanout, j) {
                    if (Abc_ObjIsPo(pFanout)) {
                        isPoDriver = true;
                        break;
                    }
                }
                if (isPoDriver)
                    dArea -= areaBuf;
            }
            else if (Ckt_GetObjArrivalTime(pTS, 3) >= Ckt_GetObjArrivalTime(pSS, 3) +  invDelay * 1000) {
                dArea -= areaInv;
            }
            cout << Abc_ObjName(pTS) << "," << Abc_ObjName(pSS) << "," << dErrors.first / static_cast <double>(appSmlt.GetFrameNum()) << "," << dErrors.second / static_cast <double>(appSmlt.GetFrameNum()) << "," << dArea << endl;
        }
    }
    if (Abc_NodeIsBuf(pTS) && Abc_ObjIsPi(Abc_ObjFanin0(pTS)))
        return;
    // consider PI replacement
    Abc_NtkForEachPi(pAppNtk, pSS, i) {
        bool isSkip = true;
        for (int j = 0; j < static_cast <int>(sources[pTS->Id].size()); ++j) {
            if (sources[pTS->Id][j] & sources[pSS->Id][j]) {
                isSkip = false;
                break;
            }
        }
        if (isSkip)
            continue;
        GetDError(appSmlt, pTS, pSS, isERInc, isERDec, dErrors);
        if (baseER + dErrors.first <= errorBoundFrame || baseER + dErrors.second <= errorBoundFrame) {
            double dArea = GetDArea(pTS, pSS, vMffcs);
            if (dErrors.first <= dErrors.second) {
                dArea -= areaBuf;
            }
            else {
                dArea -= areaInv;
            }
            cout << Abc_ObjName(pTS) << "," << Abc_ObjName(pSS) << "," << dErrors.first / static_cast <double>(appSmlt.GetFrameNum()) << "," << dErrors.second / static_cast <double>(appSmlt.GetFrameNum()) << "," << dArea << endl;
        }
        // cout << Abc_ObjName(pTS) << "," << Abc_ObjName(pSS) << "," << dErrors.first / static_cast <double>(appSmlt.GetFrameNum()) << "," << dErrors.second / static_cast <double>(appSmlt.GetFrameNum()) << endl;
    }
}


void SASIMI_Manager_t::GetDError(IN Simulator_t & appSmlt, IN Abc_Obj_t * pTS, IN Abc_Obj_t * pSS, IN vector <tVec> & isERInc, IN vector <tVec> & isERDec, OUT pair<int, int> & dErrors)
{
    Abc_Ntk_t * pAppNtk = appSmlt.GetNetwork();
    DASSERT(pAppNtk == pTS->pNtk);
    DASSERT(pAppNtk == pSS->pNtk);
    int nBlock = appSmlt.GetBlockNum();
    int dError = 0, dInvError = 0;
    for (int i = 0; i < nBlock; ++i) {
        uint64_t isChanged = appSmlt.GetValues(pTS, i) ^ appSmlt.GetValues(pSS, i);
        uint64_t incFlag = isERInc[pTS->Id][i] & isChanged;
        dError += Ckt_CountOneNum(incFlag);
        uint64_t decFlag = isERDec[pTS->Id][i] & isChanged;
        dError -= Ckt_CountOneNum(decFlag);
        if (!Abc_NodeIsConst(pSS)) {
            incFlag = isERInc[pTS->Id][i] & ~isChanged;
            dInvError += Ckt_CountOneNum(incFlag);
            decFlag = isERDec[pTS->Id][i] & ~isChanged;
            dInvError -= Ckt_CountOneNum(decFlag);
        }
    }
    dErrors.first = dError;
    dErrors.second = dInvError;
}


double SASIMI_Manager_t::GetDArea(Abc_Obj_t * pTS, Abc_Obj_t * pSS, vector <Vec_Ptr_t *> & vMffcs)
{
    DASSERT(Abc_NtkIsMappedLogic(pTS->pNtk) && pTS->pNtk == pSS->pNtk);
    Abc_Obj_t * pObj = nullptr;
    int i = 0;
    set <Abc_Obj_t *> m1;
    Vec_PtrForEachEntry(Abc_Obj_t *, vMffcs[pTS->Id], pObj, i)
        m1.insert(pObj);
    set <Abc_Obj_t *> m2;
    if (Abc_ObjIsNode(pSS)) {
        Vec_PtrForEachEntry(Abc_Obj_t *, vMffcs[pSS->Id], pObj, i)
            m2.insert(pObj);
    }

    vector <Abc_Obj_t *> mInter(max(m1.size(), m2.size()));
    auto iter = set_intersection(m1.begin(), m1.end(), m2.begin(), m2.end(), mInter.begin());
    mInter.resize(iter - mInter.begin());
    vector <Abc_Obj_t *> mDiff(m1.size());
    iter = set_difference(m1.begin(), m1.end(), mInter.begin(), mInter.end(), mDiff.begin());
    mDiff.resize(iter - mDiff.begin());

    double dArea = 0;
    for (auto & pObj: mDiff)
        dArea += Mio_GateReadArea( (Mio_Gate_t *)pObj->pData );

    // if (string(Abc_ObjName(pTS)) == "[37789]" && string(Abc_ObjName(pSS)) == "[37787]") {
    //     cout << "TS MFFC " << Abc_ObjName(pTS) << ":";
    //     Vec_PtrForEachEntry(Abc_Obj_t *, vMffcs[pTS->Id], pObj, i)
    //         cout << Abc_ObjName(pObj) << ",";
    //     cout << endl;
    //     cout << "SS MFFC " << Abc_ObjName(pSS) << ":";
    //     Vec_PtrForEachEntry(Abc_Obj_t *, vMffcs[pSS->Id], pObj, i)
    //         cout << Abc_ObjName(pObj) << ",";
    //     cout << endl;
    //     cout << "intersection:";
    //     for (auto & pObj: mInter)
    //         cout << Abc_ObjName(pObj);
    //     cout << endl;
    //     cout << "saved:";
    //     for (auto & pObj: mDiff)
    //         cout << Abc_ObjName(pObj) << ",";
    //     cout << endl;
    // }
    return dArea;
}
