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
    std::vector <Vec_Ptr_t *> vMffcs;
    while (error < errorBound) {
        appSmlt.Simulate();
        CollectMFFC(pAppNtk, vMffcs);
        Abc_NtkDelayTrace(pAppNtk, nullptr, nullptr, 0);
        GetCPM(oriSmlt, appSmlt, bds);
        CollectAllLACs(oriSmlt, appSmlt, bds);
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
        vMffcs[pNode->Id] = nullptr;
        if ( Abc_ObjFanoutNum(pNode) > 1 ) {
            vMffcs[pNode->Id] = Vec_PtrAlloc(100);
            Abc_NodeDeref_rec( pNode );
            Abc_NodeMffcConeSupp( pNode, vMffcs[pNode->Id], nullptr );
            Abc_NodeRef_rec( pNode );
        }
    }
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


void SASIMI_Manager_t::CollectAllLACs(IN Simulator_t & oriSmlt, IN Simulator_t & appSmlt, IN vector < vector <tVec> > & bds)
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
    Abc_NtkForEachNode(pAppNtk, pObj, i) {
        if (!Abc_NodeIsConst(pObj)) {
            CollectNodeLAC(pObj, appSmlt, isERInc, isERDec, sources);
        }
    }
}


void SASIMI_Manager_t::CollectNodeLAC(IN Abc_Obj_t * pTS, IN Simulator_t & appSmlt, IN vector <tVec> & isERInc, IN vector <tVec> & isERDec, IN vector <tVec> & sources)
{
    Abc_Ntk_t * pAppNtk = appSmlt.GetNetwork();
    DASSERT(pAppNtk == pTS->pNtk);
    // consider constant replacement
    Abc_Obj_t * pConst0 = Ckt_GetConst(pAppNtk, 0);
    pair <int, int> errors;
    GetDError(appSmlt, pTS, pConst0, isERInc, isERDec, errors);
    cout << Abc_ObjName(pTS) << ",0," << errors.first / static_cast <double>(appSmlt.GetFrameNum()) << endl;
    Abc_Obj_t * pConst1 = Ckt_GetConst(pAppNtk, 1);
    GetDError(appSmlt, pTS, pConst1, isERInc, isERDec, errors);
    cout << Abc_ObjName(pTS) << ",1," << errors.first / static_cast <double>(appSmlt.GetFrameNum()) << endl;
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
        GetDError(appSmlt, pTS, pSS, isERInc, isERDec, errors);
        cout << Abc_ObjName(pTS) << "," << Abc_ObjName(pSS) << "," << errors.first / static_cast <double>(appSmlt.GetFrameNum()) << "," << errors.second / static_cast <double>(appSmlt.GetFrameNum()) << endl;
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
        GetDError(appSmlt, pTS, pSS, isERInc, isERDec, errors);
        cout << Abc_ObjName(pTS) << "," << Abc_ObjName(pSS) << "," << errors.first / static_cast <double>(appSmlt.GetFrameNum()) << "," << errors.second / static_cast <double>(appSmlt.GetFrameNum()) << endl;
    }
}


void SASIMI_Manager_t::GetDError(IN Simulator_t & appSmlt, IN Abc_Obj_t * pTS, IN Abc_Obj_t * pSS, IN vector <tVec> & isERInc, IN vector <tVec> & isERDec, OUT pair<int, int> & errors)
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
    errors.first = dError;
    errors.second = dInvError;
}
