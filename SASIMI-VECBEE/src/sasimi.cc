#include "sasimi.h"


using namespace std;
using namespace std::filesystem;


SASIMI_Manager_t::SASIMI_Manager_t(Abc_Ntk_t * pNtk, int nFrame, int maxLevel, Metric_t metricType, double errorBound):
    nFrame(nFrame), maxLevel(maxLevel), metricType(metricType), errorBound(errorBound)
{
}


SASIMI_Manager_t::~SASIMI_Manager_t()
{
}


void SASIMI_Manager_t::GreedySelection(Abc_Ntk_t * pOriNtk, string outPrefix)
{
    // init
    Abc_Ntk_t * pAppNtk = Abc_NtkDup(pOriNtk);
    PatchConst(pOriNtk);
    PatchConst(pAppNtk);
    Simulator_t oriSmlt(pOriNtk, nFrame);

    // iteration
    vector < vector <tVec> > bds; // cpm[PO][obj][frameBlock]
    vector <Vec_Ptr_t *> vMffcs;
    vector <LAC_t> nodeLACs;
    vector <LAC_t> candLACs;
    random_device rd;
    clock_t st = clock();
    cntRound = 0;
    while (1) {
        cout << "--------------- round " << ++cntRound << " ---------------" << endl;
        if (cntRound == 4)
            break;
        Simulator_t * pAppSmlt = new Simulator_t(pAppNtk, nFrame);
        unsigned seed = static_cast <unsigned> (rd());
        // unsigned seed = 2531465778;
        cout << "seed = " << seed << endl;
        cout << "maxLevel = " << maxLevel << endl;
        oriSmlt.Input(seed);
        oriSmlt.Simulate();
        pAppSmlt->Input(seed);
        pAppSmlt->Simulate();
        // GetCPM(oriSmlt, *pAppSmlt, bds);
        GetCPMOneCut(oriSmlt, *pAppSmlt, bds);
        Abc_NtkDelayTrace(pAppNtk, nullptr, nullptr, 0);
        CollectMFFC(*pAppSmlt, vMffcs);
        if (metricType == Metric_t::ER)
            CollectAllLACsUnderER(oriSmlt, *pAppSmlt, bds, vMffcs, nodeLACs);
        else
            CollectAllLACsUnderNMED(oriSmlt, *pAppSmlt, bds, vMffcs, nodeLACs);
        FreeMFFC(vMffcs);
        SortCandLACs(nodeLACs, candLACs);

        // {
        //     static int64_t totLACs;
        //     static int64_t CP;
        //     static double AERD;
        //     double accError = 0.0;
        //     double estError = 0.0;
        //     int baseER = GetER(&oriSmlt, pAppSmlt);
        //     for (auto & candLac: candLACs) {
        //         ++totLACs;
        //         accError = MeasureSASIMIER(&oriSmlt, pAppSmlt, candLac.GetTS(), candLac.GetSS(), candLac.GetIsInv(), true);
        //         estError = (baseER + candLac.GetDError()) / static_cast <double>(pAppSmlt->GetFrameNum());
        //         if (abs(accError - estError) < 1e-10) ++CP;
        //         AERD += abs(accError - estError);
        //         cout << totLACs << "," << CP / static_cast <double> (totLACs) << "," << AERD / totLACs << endl;
        //     }
        // }
        int res = ApplyBestLAC(oriSmlt, *pAppSmlt, candLACs, 10, outPrefix, seed);
        delete pAppSmlt;
        if (res) {
            cout << "exceed error bound" << endl;
            break;
        }
        cout << "time = " << clock() - st << " us" << endl;
    }

    // clean up
    Abc_NtkDelete(pAppNtk);
}


void SASIMI_Manager_t::GreedySelectionInaccurate(Abc_Ntk_t * pOriNtk, string outPrefix)
{
    // init
    Abc_Ntk_t * pAppNtk = Abc_NtkDup(pOriNtk);
    PatchConst(pOriNtk);
    PatchConst(pAppNtk);
    Simulator_t oriSmlt(pOriNtk, nFrame);

    // iteration
    vector < vector <tVec> > bds; // cpm[PO][obj][frameBlock]
    vector <Vec_Ptr_t *> vMffcs;
    vector <LAC_t> nodeLACs;
    vector <LAC_t> candLACs;
    random_device rd;
    clock_t st = clock();
    cntRound = 0;
    while (1) {
        cout << "--------------- round " << ++cntRound << " ---------------" << endl;
        Simulator_t * pAppSmlt = new Simulator_t(pAppNtk, nFrame);
        unsigned seed = static_cast <unsigned> (rd());
        // unsigned seed = 2531465778;
        cout << "seed = " << seed << endl;
        cout << "maxLevel = " << maxLevel << endl;
        oriSmlt.Input(seed);
        oriSmlt.Simulate();
        pAppSmlt->Input(seed);
        pAppSmlt->Simulate();
        // GetCPM(oriSmlt, *pAppSmlt, bds);
        // GetCPMOneCut(oriSmlt, *pAppSmlt, bds);
        Abc_NtkDelayTrace(pAppNtk, nullptr, nullptr, 0);
        CollectMFFC(*pAppSmlt, vMffcs);
        if (metricType == Metric_t::ER)
            CollectAllLACsUnderERInaccurate(oriSmlt, *pAppSmlt, bds, vMffcs, nodeLACs);
        else
            assert(0);
        FreeMFFC(vMffcs);
        SortCandLACs(nodeLACs, candLACs);
        int res = ApplyBestLACInaccurate(oriSmlt, *pAppSmlt, candLACs, 1, outPrefix, seed);
        delete pAppSmlt;
        if (res) {
            cout << "exceed error bound" << endl;
            break;
        }
        cout << "time = " << clock() - st << " us" << endl;
    }

    // clean up
    Abc_NtkDelete(pAppNtk);
}


void SASIMI_Manager_t::PatchConst(Abc_Ntk_t * pNtk)
{
    Ckt_GetConst(pNtk, 0);
    Ckt_GetConst(pNtk, 1);
}


void SASIMI_Manager_t::CollectMFFC(IN Simulator_t & appSmlt, OUT vector <Vec_Ptr_t *> & vMffcs)
{
    Abc_Ntk_t * pAppNtk = appSmlt.GetNetwork();
    vMffcs.resize(appSmlt.GetMaxId() + 1);
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
}


void SASIMI_Manager_t::GetCPMOneCut(IN Simulator_t & oriSmlt, IN Simulator_t & appSmlt, OUT vector < vector <tVec> > & bds)
{
    // check
    Abc_Ntk_t * pOriNtk = oriSmlt.GetNetwork();
    Abc_Ntk_t * pAppNtk = appSmlt.GetNetwork();
    DASSERT(pOriNtk != pAppNtk);
    DASSERT(SmltChecker(&oriSmlt, &appSmlt));
    // get 1-cuts and the corresponding networks
    appSmlt.BuildOneCutNtks(maxLevel);
    // simulate networks of disjoint cuts
    appSmlt.SimulateOneCutNtks();
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
        appSmlt.UpdateBoolDiffOneCut(i, vNodes, bds[i]);

    // clean up
    Vec_PtrFree(vNodes);
}


void SASIMI_Manager_t::CollectAllLACsUnderER(IN Simulator_t & oriSmlt, IN Simulator_t & appSmlt, IN vector < vector <tVec> > & bds, IN vector <Vec_Ptr_t *> & vMffcs, OUT vector <LAC_t> & nodeLACs)
{
    Abc_Ntk_t * pOriNtk = oriSmlt.GetNetwork();
    Abc_Ntk_t * pAppNtk = appSmlt.GetNetwork();
    nodeLACs.resize(appSmlt.GetMaxId() + 1);
    for (auto & nodeLAC : nodeLACs)
        nodeLAC.Init();
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
    vector <tVec> isERInc(appSmlt.GetMaxId() + 1, tVec(nBlock));
    vector <tVec> isERDec(appSmlt.GetMaxId() + 1, tVec(nBlock));
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
    vector <tVec> sources(appSmlt.GetMaxId() + 1);
    Abc_NtkForEachObj(pAppNtk, pObj, i)
        sources[pObj->Id].resize(sourceLen, 0);
    Abc_NtkForEachPi(pAppNtk, pObj, i)
        Ckt_SetBit(sources[pObj->Id][i >> 6], i);
    vector <Abc_Obj_t *> danglePIs;
    Abc_NtkForEachPi(pAppNtk, pObj, i) {
        if (!Abc_ObjFanoutNum(pObj)) {
            danglePIs.emplace_back(pObj);
        }
    }
    Vec_Ptr_t * vNodes = Abc_NtkDfs(pAppNtk, 0);
    Vec_PtrForEachEntry(Abc_Obj_t *, vNodes, pObj, i) {
        Abc_Obj_t * pFanin = nullptr;
        int j = 0;
        Abc_ObjForEachFanin(pObj, pFanin, j) {
            for (int k = 0; k < sourceLen; ++k)
                sources[pObj->Id][k] |= sources[pFanin->Id][k];
        }
        for (auto & danglePI: danglePIs) {
            for (int k = 0; k < sourceLen; ++k)
                sources[pObj->Id][k] |= sources[danglePI->Id][k];
        }
    }
    Vec_PtrFree(vNodes);
    // collect one LAC for each node
    int baseER = GetER(&oriSmlt, &appSmlt);
    boost::timer::progress_display pd(Abc_NtkNodeNum(pAppNtk));
    Abc_NtkForEachNode(pAppNtk, pObj, i) {
        if (!Abc_NodeIsConst(pObj)) {
            CollectNodeLACUnderER(pObj, oriSmlt, appSmlt, isERInc, isERDec, sources, vMffcs, baseER, nodeLACs[pObj->Id]);
        }
        ++pd;
    }
}


void SASIMI_Manager_t::CollectAllLACsUnderERInaccurate(IN Simulator_t & oriSmlt, IN Simulator_t & appSmlt, IN vector < vector <tVec> > & bds, IN vector <Vec_Ptr_t *> & vMffcs, OUT vector <LAC_t> & nodeLACs)
{
    Abc_Ntk_t * pOriNtk = oriSmlt.GetNetwork();
    Abc_Ntk_t * pAppNtk = appSmlt.GetNetwork();
    nodeLACs.resize(appSmlt.GetMaxId() + 1);
    for (auto & nodeLAC : nodeLACs)
        nodeLAC.Init();
    // // update PO correctness
    // int nBlock = oriSmlt.GetBlockNum();
    // vector <tVec> poKRight(Abc_NtkPoNum(pAppNtk), tVec(nBlock));
    // tVec allPoRight(nBlock);
    // for (int i = 0; i < nBlock; ++i)
    //     allPoRight[i] = static_cast <uint64_t> (ULLONG_MAX);
    // for (int i = 0; i < Abc_NtkPoNum(pOriNtk); ++i) {
    //     Abc_Obj_t * pOriPo = Abc_NtkPo(pOriNtk, i);
    //     Abc_Obj_t * pAppPo = Abc_NtkPo(pAppNtk, i);
    //     for (int j = 0; j < nBlock; ++j) {
    //         poKRight[i][j] = ~(oriSmlt.GetValues(pOriPo, j) ^ appSmlt.GetValues(pAppPo, j));
    //         allPoRight[j] &= poKRight[i][j];
    //     }
    // }
    // // update increase/decrease flag
    // vector <tVec> isERInc(appSmlt.GetMaxId() + 1, tVec(nBlock));
    // vector <tVec> isERDec(appSmlt.GetMaxId() + 1, tVec(nBlock));
    Abc_Obj_t * pObj = nullptr;
    int i = 0;
    // Abc_NtkForEachNode(pAppNtk, pObj, i) {
    //     for (int j = 0; j < nBlock; ++ j) {
    //         isERInc[pObj->Id][j] = 0;
    //         isERDec[pObj->Id][j] = static_cast <uint64_t> (ULLONG_MAX);
    //     }
    // }
    // for (int i = 0; i < Abc_NtkPoNum(pOriNtk); ++i) {
    //     int j = 0;
    //     Abc_NtkForEachNode(pAppNtk, pObj, j) {
    //         for (int k = 0; k < nBlock; ++k) {
    //             isERInc[pObj->Id][k] |= bds[i][pObj->Id][k];
    //             isERDec[pObj->Id][k] &= (poKRight[i][k] ^ bds[i][pObj->Id][k]);
    //         }
    //     }
    // }
    // uint64_t lastBlockMask = 0;
    // for (int i = 0; i < appSmlt.GetLastBlockLen(); ++i)
    //     Ckt_SetBit(lastBlockMask, i);
    // Abc_NtkForEachNode(pAppNtk, pObj, i) {
    //     for (int j = 0; j < nBlock; ++ j) {
    //         isERInc[pObj->Id][j] &= allPoRight[j];
    //         isERDec[pObj->Id][j] &= ~allPoRight[j];
    //         if (j == nBlock - 1) {
    //             isERInc[pObj->Id][j] &= lastBlockMask;
    //             isERDec[pObj->Id][j] &= lastBlockMask;
    //         }
    //     }
    // }
    // update PI flag
    int sourceLen = (Abc_NtkPiNum(pAppNtk) >> 6) + 1;
    vector <tVec> sources(appSmlt.GetMaxId() + 1);
    Abc_NtkForEachObj(pAppNtk, pObj, i)
        sources[pObj->Id].resize(sourceLen, 0);
    Abc_NtkForEachPi(pAppNtk, pObj, i)
        Ckt_SetBit(sources[pObj->Id][i >> 6], i);
    vector <Abc_Obj_t *> danglePIs;
    Abc_NtkForEachPi(pAppNtk, pObj, i) {
        if (!Abc_ObjFanoutNum(pObj)) {
            danglePIs.emplace_back(pObj);
        }
    }
    Vec_Ptr_t * vNodes = Abc_NtkDfs(pAppNtk, 0);
    Vec_PtrForEachEntry(Abc_Obj_t *, vNodes, pObj, i) {
        Abc_Obj_t * pFanin = nullptr;
        int j = 0;
        Abc_ObjForEachFanin(pObj, pFanin, j) {
            for (int k = 0; k < sourceLen; ++k)
                sources[pObj->Id][k] |= sources[pFanin->Id][k];
        }
        for (auto & danglePI: danglePIs) {
            for (int k = 0; k < sourceLen; ++k)
                sources[pObj->Id][k] |= sources[danglePI->Id][k];
        }
    }
    Vec_PtrFree(vNodes);
    // collect one LAC for each node
    int baseER = GetER(&oriSmlt, &appSmlt);
    Abc_NtkForEachNode(pAppNtk, pObj, i) {
        if (!Abc_NodeIsConst(pObj)) {
            // CollectNodeLACUnderER(pObj, oriSmlt, appSmlt, isERInc, isERDec, sources, vMffcs, baseER, nodeLACs[pObj->Id]);
            CollectNodeLACUnderERInaccurate(pObj, oriSmlt, appSmlt, sources, vMffcs, baseER, nodeLACs[pObj->Id]);
        }
    }
}


void SASIMI_Manager_t::CollectAllLACsUnderNMED(IN Simulator_t & oriSmlt, IN Simulator_t & appSmlt, IN vector < vector <tVec> > & bds, IN vector <Vec_Ptr_t *> & vMffcs, OUT vector <LAC_t> & nodeLACs)
{
    Abc_Ntk_t * pAppNtk = appSmlt.GetNetwork();
    nodeLACs.resize(appSmlt.GetMaxId() + 1);
    for (auto & nodeLAC : nodeLACs)
        nodeLAC.Init();
    // update PI flag
    int sourceLen = (Abc_NtkPiNum(pAppNtk) >> 6) + 1;
    vector <tVec> sources(appSmlt.GetMaxId() + 1);
    Abc_Obj_t * pObj = nullptr;
    int i = 0;
    Abc_NtkForEachObj(pAppNtk, pObj, i)
        sources[pObj->Id].resize(sourceLen, 0);
    Abc_NtkForEachPi(pAppNtk, pObj, i)
        Ckt_SetBit(sources[pObj->Id][i >> 6], i);
    vector <Abc_Obj_t *> danglePIs;
    Abc_NtkForEachPi(pAppNtk, pObj, i) {
        if (!Abc_ObjFanoutNum(pObj)) {
            danglePIs.emplace_back(pObj);
        }
    }
    Vec_Ptr_t * vNodes = Abc_NtkDfs(pAppNtk, 0);
    Vec_PtrForEachEntry(Abc_Obj_t *, vNodes, pObj, i) {
        Abc_Obj_t * pFanin = nullptr;
        int j = 0;
        Abc_ObjForEachFanin(pObj, pFanin, j) {
            for (int k = 0; k < sourceLen; ++k)
                sources[pObj->Id][k] |= sources[pFanin->Id][k];
        }
        for (auto & danglePI: danglePIs) {
            for (int k = 0; k < sourceLen; ++k)
                sources[pObj->Id][k] |= sources[danglePI->Id][k];
        }
    }
    Vec_PtrFree(vNodes);
    // collect correct outputs
    int nPo = Abc_NtkPoNum(pAppNtk);
    int nBlock = appSmlt.GetBlockNum();
    DASSERT(nPo <= 33, "don't support two many POs");
    int frameCnt = 0;
    vector <int64_t> oriOutputs(nFrame);
    vector <int64_t> appOutputs(nFrame);
    for (int i = 0; i < nBlock; ++i) {
        for (int j = 0; j < 64; ++j) {
            if (frameCnt >= nFrame)
                break;
            oriOutputs[frameCnt] = oriSmlt.GetOutputFast(i, j);
            appOutputs[frameCnt] = appSmlt.GetOutputFast(i, j);
            ++frameCnt;
        }
    }
    // collect one LAC for each node
    int64_t baseNMED = GetNMEDFast(oriOutputs, appOutputs);
    tVec bdNode(nFrame, 0);
    boost::timer::progress_display pd(Abc_NtkNodeNum(pAppNtk));
    Abc_NtkForEachNode(pAppNtk, pObj, i) {
        if (!Abc_NodeIsConst(pObj)) {
            ReorganizeBD(bds, pObj, bdNode); // cpm[PO][obj][frameBlock] -> cpm[frame][POBlock]
            CollectNodeLACUnderNMED(pObj, appSmlt, oriOutputs, appOutputs, bdNode, sources, vMffcs, baseNMED, nodeLACs[pObj->Id]);
        }
        ++pd;
    }
}


void SASIMI_Manager_t::CollectNodeLACUnderER(IN Abc_Obj_t * pTS, IN Simulator_t & oriSmlt, IN Simulator_t & appSmlt, IN vector <tVec> & isERInc, IN vector <tVec> & isERDec, IN vector <tVec> & sources, IN vector <Vec_Ptr_t *> & vMffcs, IN int baseER, OUT LAC_t & nodeLAC)
{
    static int64_t totLACs;
    static int64_t CP;
    static double AERD;
    double accError = 0.0;
    double estError = 0.0;

    Abc_Ntk_t * pAppNtk = appSmlt.GetNetwork();
    DASSERT(pAppNtk == pTS->pNtk);
    double errorBoundInt = errorBound * appSmlt.GetFrameNum();
    double invDelay = Mio_LibraryReadDelayInvMax((Mio_Library_t *)Abc_FrameReadLibGen()) + 0.1;
    int areaInv = Mio_LibraryReadAreaInv((Mio_Library_t *)Abc_FrameReadLibGen());
    int areaBuf = Mio_LibraryReadAreaBuf((Mio_Library_t *)Abc_FrameReadLibGen());
    nodeLAC.SetFOM(0.0);
    // consider constant replacement
    Abc_Obj_t * pConst0 = Ckt_GetConst(pAppNtk, 0);
    pair <int, int> dErrors;
    GetDER(appSmlt, pTS, pConst0, isERInc, isERDec, dErrors);
    // cout << Abc_ObjName(pTS) << ",0," << dErrors.first / static_cast <double>(appSmlt.GetFrameNum()) << ",," << endl;
    if (baseER + dErrors.first <= errorBoundInt) {
        double dArea = GetDArea(pTS, pConst0, vMffcs);
        double tempFOM = dArea / (dErrors.first + 1e-10);
        if ((dErrors.first < 0 && nodeLAC.GetFOM() > tempFOM) || (dErrors.first >= 0 && nodeLAC.GetFOM() >= 0 && nodeLAC.GetFOM() < tempFOM))
            nodeLAC.Update(pTS, pConst0, false, dErrors.first, dArea, tempFOM);
    }
    // if (baseER + dErrors.first <= errorBoundInt) {
    //     ++totLACs;
    //     accError = MeasureSASIMIER(&oriSmlt, &appSmlt, pTS, pConst0, 0, true);
    //     estError = (baseER + dErrors.first) / static_cast <double>(appSmlt.GetFrameNum());
    //     if (abs(accError - estError) < 1e-10) ++CP;
    //     AERD += abs(accError - estError);
    // }

    Abc_Obj_t * pConst1 = Ckt_GetConst(pAppNtk, 1);
    GetDER(appSmlt, pTS, pConst1, isERInc, isERDec, dErrors);
    // cout << Abc_ObjName(pTS) << ",1," << dErrors.first / static_cast <double>(appSmlt.GetFrameNum()) << ",," << endl;
    if (baseER + dErrors.first <= errorBoundInt) {
        double dArea = GetDArea(pTS, pConst1, vMffcs);
        double tempFOM = dArea / (dErrors.first + 1e-10);
        if ((dErrors.first < 0 && nodeLAC.GetFOM() > tempFOM) || (dErrors.first >= 0 && nodeLAC.GetFOM() >= 0 && nodeLAC.GetFOM() < tempFOM))
            nodeLAC.Update(pTS, pConst1, false, dErrors.first, dArea, tempFOM);
    }
    // if (baseER + dErrors.first <= errorBoundInt) {
    //     ++totLACs;
    //     accError = MeasureSASIMIER(&oriSmlt, &appSmlt, pTS, pConst1, 0, true);
    //     estError = (baseER + dErrors.first) / static_cast <double>(appSmlt.GetFrameNum());
    //     if (abs(accError - estError) < 1e-10) ++CP;
    //     AERD += abs(accError - estError);
    // }

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
        GetDER(appSmlt, pTS, pSS, isERInc, isERDec, dErrors);
        // cout << Abc_ObjName(pTS) << "," << Abc_ObjName(pSS) << "," << dErrors.first / static_cast <double>(appSmlt.GetFrameNum()) << "," << dErrors.second / static_cast <double>(appSmlt.GetFrameNum()) << endl;
        if (baseER + dErrors.first <= errorBoundInt || (baseER + dErrors.second <= errorBoundInt && Ckt_GetObjArrivalTime(pTS, 3) >= Ckt_GetObjArrivalTime(pSS, 3) +  invDelay * 1000)) {
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
                double tempFOM = dArea / (dErrors.first + 1e-10);
                if ((dErrors.first < 0 && nodeLAC.GetFOM() > tempFOM) || (dErrors.first >= 0 && nodeLAC.GetFOM() >= 0 && nodeLAC.GetFOM() < tempFOM))
                    nodeLAC.Update(pTS, pSS, false, dErrors.first, dArea, tempFOM);
            }
            else if (Ckt_GetObjArrivalTime(pTS, 3) >= Ckt_GetObjArrivalTime(pSS, 3) +  invDelay * 1000) {
                dArea -= areaInv;
                double tempFOM = dArea / (dErrors.second + 1e-10);
                if ((dErrors.second < 0 && nodeLAC.GetFOM() > tempFOM) || (dErrors.second >= 0 && nodeLAC.GetFOM() >= 0 && nodeLAC.GetFOM() < tempFOM))
                    nodeLAC.Update(pTS, pSS, true, dErrors.second, dArea, tempFOM);
            }
        }
        // if (baseER + dErrors.first <= errorBoundInt || (baseER + dErrors.second <= errorBoundInt && Ckt_GetObjArrivalTime(pTS, 3) >= Ckt_GetObjArrivalTime(pSS, 3) +  invDelay * 1000)) {
        //     ++totLACs;
        //     accError = MeasureSASIMIER(&oriSmlt, &appSmlt, pTS, pSS, 0, true);
        //     estError = (baseER + dErrors.first) / static_cast <double>(appSmlt.GetFrameNum());
        //     if (abs(accError - estError) < 1e-10) ++CP;
        //     AERD += abs(accError - estError);
        //     ++totLACs;
        //     accError = MeasureSASIMIER(&oriSmlt, &appSmlt, pTS, pSS, 1, true);
        //     estError = (baseER + dErrors.second) / static_cast <double>(appSmlt.GetFrameNum());
        //     if (abs(accError - estError) < 1e-10) ++CP;
        //     AERD += abs(accError - estError);
        // }
    }

    // consider PI replacement
    if (Abc_NodeIsBuf(pTS) && Abc_ObjIsPi(Abc_ObjFanin0(pTS)))
        return;
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
        GetDER(appSmlt, pTS, pSS, isERInc, isERDec, dErrors);
        // cout << Abc_ObjName(pTS) << "," << Abc_ObjName(pSS) << "," << dErrors.first / static_cast <double>(appSmlt.GetFrameNum()) << "," << dErrors.second / static_cast <double>(appSmlt.GetFrameNum()) << endl;
        if (baseER + dErrors.first <= errorBoundInt || baseER + dErrors.second <= errorBoundInt) {
            double dArea = GetDArea(pTS, pSS, vMffcs);
            if (dErrors.first <= dErrors.second) {
                dArea -= areaBuf;
                double tempFOM = dArea / (dErrors.first + 1e-10);
                if ((dErrors.first < 0 && nodeLAC.GetFOM() > tempFOM) || (dErrors.first >= 0 && nodeLAC.GetFOM() >= 0 && nodeLAC.GetFOM() < tempFOM))
                    nodeLAC.Update(pTS, pSS, false, dErrors.first, dArea, tempFOM);
            }
            else {
                dArea -= areaInv;
                double tempFOM = dArea / (dErrors.second + 1e-10);
                if ((dErrors.second < 0 && nodeLAC.GetFOM() > tempFOM) || (dErrors.second >= 0 && nodeLAC.GetFOM() >= 0 && nodeLAC.GetFOM() < tempFOM))
                    nodeLAC.Update(pTS, pSS, true, dErrors.second, dArea, tempFOM);
            }
        }
        // if (baseER + dErrors.first <= errorBoundInt || baseER + dErrors.second <= errorBoundInt) {
        //     ++totLACs;
        //     accError = MeasureSASIMIER(&oriSmlt, &appSmlt, pTS, pSS, 0, true);
        //     estError = (baseER + dErrors.first) / static_cast <double>(appSmlt.GetFrameNum());
        //     if (abs(accError - estError) < 1e-10) ++CP;
        //     AERD += abs(accError - estError);
        //     ++totLACs;
        //     accError = MeasureSASIMIER(&oriSmlt, &appSmlt, pTS, pSS, 1, true);
        //     estError = (baseER + dErrors.second) / static_cast <double>(appSmlt.GetFrameNum());
        //     if (abs(accError - estError) < 1e-10) ++CP;
        //     AERD += abs(accError - estError);
        // }
    }
    // cout << totLACs << "," << CP / static_cast <double> (totLACs) << "," << AERD / totLACs << endl;
}


void SASIMI_Manager_t::CollectNodeLACUnderERInaccurate(IN Abc_Obj_t * pTS, IN Simulator_t & oriSmlt, IN Simulator_t & appSmlt, IN vector <tVec> & sources, IN vector <Vec_Ptr_t *> & vMffcs, IN int baseER, OUT LAC_t & nodeLAC)
{
    // static int64_t totLACs;
    // static int64_t CP;
    // static double AERD;
    // double accError = 0.0;
    // double estError = 0.0;

    Abc_Ntk_t * pAppNtk = appSmlt.GetNetwork();
    DASSERT(pAppNtk == pTS->pNtk);
    double errorBoundInt = errorBound * appSmlt.GetFrameNum() * 5;
    double invDelay = Mio_LibraryReadDelayInvMax((Mio_Library_t *)Abc_FrameReadLibGen()) + 0.1;
    int areaInv = Mio_LibraryReadAreaInv((Mio_Library_t *)Abc_FrameReadLibGen());
    int areaBuf = Mio_LibraryReadAreaBuf((Mio_Library_t *)Abc_FrameReadLibGen());
    nodeLAC.SetFOM(0.0);
    // consider constant replacement
    Abc_Obj_t * pConst0 = Ckt_GetConst(pAppNtk, 0);
    pair <int, int> dErrors;
    GetDERInaccurate(appSmlt, pTS, pConst0, dErrors);
    // cout << Abc_ObjName(pTS) << ",0," << dErrors.first / static_cast <double>(appSmlt.GetFrameNum()) << ",," << endl;
    if (baseER + dErrors.first <= errorBoundInt) {
        double dArea = GetDArea(pTS, pConst0, vMffcs);
        double tempFOM = dArea / (dErrors.first + 1e-10);
        if ((dErrors.first < 0 && nodeLAC.GetFOM() > tempFOM) || (dErrors.first >= 0 && nodeLAC.GetFOM() >= 0 && nodeLAC.GetFOM() < tempFOM))
            nodeLAC.Update(pTS, pConst0, false, dErrors.first, dArea, tempFOM);
    }
    // ++totLACs;
    // accError = MeasureSASIMIER(&oriSmlt, &appSmlt, pTS, pConst0, 0, true);
    // estError = (baseER + dErrors.first) / static_cast <double>(appSmlt.GetFrameNum());
    // if (abs(accError - estError) < 1e-10) ++CP;
    // AERD += abs(accError - estError);

    Abc_Obj_t * pConst1 = Ckt_GetConst(pAppNtk, 1);
    GetDERInaccurate(appSmlt, pTS, pConst1, dErrors);
    // cout << Abc_ObjName(pTS) << ",1," << dErrors.first / static_cast <double>(appSmlt.GetFrameNum()) << ",," << endl;
    if (baseER + dErrors.first <= errorBoundInt) {
        double dArea = GetDArea(pTS, pConst1, vMffcs);
        double tempFOM = dArea / (dErrors.first + 1e-10);
        if ((dErrors.first < 0 && nodeLAC.GetFOM() > tempFOM) || (dErrors.first >= 0 && nodeLAC.GetFOM() >= 0 && nodeLAC.GetFOM() < tempFOM))
            nodeLAC.Update(pTS, pConst1, false, dErrors.first, dArea, tempFOM);
    }
    // ++totLACs;
    // accError = MeasureSASIMIER(&oriSmlt, &appSmlt, pTS, pConst1, 0, true);
    // estError = (baseER + dErrors.first) / static_cast <double>(appSmlt.GetFrameNum());
    // if (abs(accError - estError) < 1e-10) ++CP;
    // AERD += abs(accError - estError);

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
        GetDERInaccurate(appSmlt, pTS, pSS, dErrors);
        // cout << Abc_ObjName(pTS) << "," << Abc_ObjName(pSS) << "," << dErrors.first / static_cast <double>(appSmlt.GetFrameNum()) << "," << dErrors.second / static_cast <double>(appSmlt.GetFrameNum()) << endl;
        if (baseER + dErrors.first <= errorBoundInt || (baseER + dErrors.second <= errorBoundInt && Ckt_GetObjArrivalTime(pTS, 3) >= Ckt_GetObjArrivalTime(pSS, 3) +  invDelay * 1000)) {
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
                double tempFOM = dArea / (dErrors.first + 1e-10);
                if ((dErrors.first < 0 && nodeLAC.GetFOM() > tempFOM) || (dErrors.first >= 0 && nodeLAC.GetFOM() >= 0 && nodeLAC.GetFOM() < tempFOM))
                    nodeLAC.Update(pTS, pSS, false, dErrors.first, dArea, tempFOM);
            }
            else if (Ckt_GetObjArrivalTime(pTS, 3) >= Ckt_GetObjArrivalTime(pSS, 3) +  invDelay * 1000) {
                dArea -= areaInv;
                double tempFOM = dArea / (dErrors.second + 1e-10);
                if ((dErrors.second < 0 && nodeLAC.GetFOM() > tempFOM) || (dErrors.second >= 0 && nodeLAC.GetFOM() >= 0 && nodeLAC.GetFOM() < tempFOM))
                    nodeLAC.Update(pTS, pSS, true, dErrors.second, dArea, tempFOM);
            }
        }
        // ++totLACs;
        // accError = MeasureSASIMIER(&oriSmlt, &appSmlt, pTS, pSS, 0, true);
        // estError = (baseER + dErrors.first) / static_cast <double>(appSmlt.GetFrameNum());
        // if (abs(accError - estError) < 1e-10) ++CP;
        // AERD += abs(accError - estError);
        // ++totLACs;
        // accError = MeasureSASIMIER(&oriSmlt, &appSmlt, pTS, pSS, 1, true);
        // estError = (baseER + dErrors.second) / static_cast <double>(appSmlt.GetFrameNum());
        // if (abs(accError - estError) < 1e-10) ++CP;
        // AERD += abs(accError - estError);
    }

    // consider PI replacement
    if (Abc_NodeIsBuf(pTS) && Abc_ObjIsPi(Abc_ObjFanin0(pTS)))
        return;
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
        GetDERInaccurate(appSmlt, pTS, pSS, dErrors);
        // cout << Abc_ObjName(pTS) << "," << Abc_ObjName(pSS) << "," << dErrors.first / static_cast <double>(appSmlt.GetFrameNum()) << "," << dErrors.second / static_cast <double>(appSmlt.GetFrameNum()) << endl;
        if (baseER + dErrors.first <= errorBoundInt || baseER + dErrors.second <= errorBoundInt) {
            double dArea = GetDArea(pTS, pSS, vMffcs);
            if (dErrors.first <= dErrors.second) {
                dArea -= areaBuf;
                double tempFOM = dArea / (dErrors.first + 1e-10);
                if ((dErrors.first < 0 && nodeLAC.GetFOM() > tempFOM) || (dErrors.first >= 0 && nodeLAC.GetFOM() >= 0 && nodeLAC.GetFOM() < tempFOM))
                    nodeLAC.Update(pTS, pSS, false, dErrors.first, dArea, tempFOM);
            }
            else {
                dArea -= areaInv;
                double tempFOM = dArea / (dErrors.second + 1e-10);
                if ((dErrors.second < 0 && nodeLAC.GetFOM() > tempFOM) || (dErrors.second >= 0 && nodeLAC.GetFOM() >= 0 && nodeLAC.GetFOM() < tempFOM))
                    nodeLAC.Update(pTS, pSS, true, dErrors.second, dArea, tempFOM);
            }
        }
        // ++totLACs;
        // accError = MeasureSASIMIER(&oriSmlt, &appSmlt, pTS, pSS, 0, true);
        // estError = (baseER + dErrors.first) / static_cast <double>(appSmlt.GetFrameNum());
        // if (abs(accError - estError) < 1e-10) ++CP;
        // AERD += abs(accError - estError);
        // ++totLACs;
        // accError = MeasureSASIMIER(&oriSmlt, &appSmlt, pTS, pSS, 1, true);
        // estError = (baseER + dErrors.second) / static_cast <double>(appSmlt.GetFrameNum());
        // if (abs(accError - estError) < 1e-10) ++CP;
        // AERD += abs(accError - estError);
    }
    // cout << totLACs << "," << CP / static_cast <double> (totLACs) << "," << AERD / totLACs << endl;
}


void SASIMI_Manager_t::CollectNodeLACUnderNMED(IN Abc_Obj_t * pTS, IN Simulator_t & appSmlt, IN vector <int64_t> & oriOutputs, IN vector <int64_t> & appOutputs, IN tVec & bdNode, IN vector <tVec> & sources, IN vector <Vec_Ptr_t * > & vMffcs, IN int64_t baseNMED, OUT LAC_t & nodeLAC)
{
    Abc_Ntk_t * pAppNtk = appSmlt.GetNetwork();
    DASSERT(pAppNtk == pTS->pNtk);
    int64_t errorBoundInt = floor(errorBound * appSmlt.GetFrameNum() * (pow(2, Abc_NtkPoNum(pAppNtk)) - 1));
    double invDelay = Mio_LibraryReadDelayInvMax((Mio_Library_t *)Abc_FrameReadLibGen()) + 0.1;
    int areaInv = Mio_LibraryReadAreaInv((Mio_Library_t *)Abc_FrameReadLibGen());
    int areaBuf = Mio_LibraryReadAreaBuf((Mio_Library_t *)Abc_FrameReadLibGen());
    nodeLAC.SetFOM(0.0);

    vector <int64_t> appOutputsNew(nFrame);
    for (int k = 0; k < nFrame; ++k)
        appOutputsNew[k] = appOutputs[k] ^ bdNode[k];

    // consider constant replacement
    Abc_Obj_t * pConst0 = Ckt_GetConst(pAppNtk, 0);
    pair <int64_t, int64_t> dErrors;
    GetDNMED(appSmlt, oriOutputs, appOutputs, appOutputsNew, pTS, pConst0, dErrors);
    if (baseNMED + dErrors.first <= errorBoundInt) {
        double dArea = GetDArea(pTS, pConst0, vMffcs);
        double tempFOM = dArea / (dErrors.first + 1e-10);
        if ((dErrors.first < 0 && nodeLAC.GetFOM() > tempFOM) || (dErrors.first >= 0 && nodeLAC.GetFOM() >= 0 && nodeLAC.GetFOM() < tempFOM))
            nodeLAC.Update(pTS, pConst0, false, dErrors.first, dArea, tempFOM);
        // cout << Abc_ObjName(pTS) << ",0," << dErrors.first / static_cast <double>(appSmlt.GetFrameNum()) << ",," << dArea << endl;
    }

    Abc_Obj_t * pConst1 = Ckt_GetConst(pAppNtk, 1);
    GetDNMED(appSmlt, oriOutputs, appOutputs, appOutputsNew, pTS, pConst1, dErrors);
    if (baseNMED + dErrors.first <= errorBoundInt) {
        double dArea = GetDArea(pTS, pConst1, vMffcs);
        double tempFOM = dArea / (dErrors.first + 1e-10);
        if ((dErrors.first < 0 && nodeLAC.GetFOM() > tempFOM) || (dErrors.first >= 0 && nodeLAC.GetFOM() >= 0 && nodeLAC.GetFOM() < tempFOM))
            nodeLAC.Update(pTS, pConst1, false, dErrors.first, dArea, tempFOM);
        // cout << Abc_ObjName(pTS) << ",1," << dErrors.first / static_cast <double>(appSmlt.GetFrameNum()) << ",," << dArea << endl;
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
        GetDNMED(appSmlt, oriOutputs, appOutputs, appOutputsNew, pTS, pSS, dErrors);
        if (baseNMED + dErrors.first <= errorBoundInt || (baseNMED + dErrors.second <= errorBoundInt && Ckt_GetObjArrivalTime(pTS, 3) >= Ckt_GetObjArrivalTime(pSS, 3) +  invDelay * 1000)) {
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
                double tempFOM = dArea / (dErrors.first + 1e-10);
                if ((dErrors.first < 0 && nodeLAC.GetFOM() > tempFOM) || (dErrors.first >= 0 && nodeLAC.GetFOM() >= 0 && nodeLAC.GetFOM() < tempFOM))
                    nodeLAC.Update(pTS, pSS, false, dErrors.first, dArea, tempFOM);

            }
            else if (Ckt_GetObjArrivalTime(pTS, 3) >= Ckt_GetObjArrivalTime(pSS, 3) +  invDelay * 1000) {
                dArea -= areaInv;
                double tempFOM = dArea / (dErrors.second + 1e-10);
                if ((dErrors.second < 0 && nodeLAC.GetFOM() > tempFOM) || (dErrors.second >= 0 && nodeLAC.GetFOM() >= 0 && nodeLAC.GetFOM() < tempFOM))
                    nodeLAC.Update(pTS, pSS, true, dErrors.second, dArea, tempFOM);
            }
        }
    }

    // consider PI replacement
    if (Abc_NodeIsBuf(pTS) && Abc_ObjIsPi(Abc_ObjFanin0(pTS)))
        return;
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
        GetDNMED(appSmlt, oriOutputs, appOutputs, appOutputsNew, pTS, pSS, dErrors);
        if (baseNMED + dErrors.first <= errorBoundInt || baseNMED + dErrors.second <= errorBoundInt) {
            double dArea = GetDArea(pTS, pSS, vMffcs);
            if (dErrors.first <= dErrors.second) {
                dArea -= areaBuf;
                double tempFOM = dArea / (dErrors.first + 1e-10);
                if ((dErrors.first < 0 && nodeLAC.GetFOM() > tempFOM) || (dErrors.first >= 0 && nodeLAC.GetFOM() >= 0 && nodeLAC.GetFOM() < tempFOM))
                    nodeLAC.Update(pTS, pSS, false, dErrors.first, dArea, tempFOM);
            }
            else {
                dArea -= areaInv;
                double tempFOM = dArea / (dErrors.second + 1e-10);
                if ((dErrors.second < 0 && nodeLAC.GetFOM() > tempFOM) || (dErrors.second >= 0 && nodeLAC.GetFOM() >= 0 && nodeLAC.GetFOM() < tempFOM))
                    nodeLAC.Update(pTS, pSS, true, dErrors.second, dArea, tempFOM);
            }
        }
    }
}


void SASIMI_Manager_t::SortCandLACs(IN std::vector <LAC_t> & nodeLACs, OUT std::vector <LAC_t> & candLACs)
{
    candLACs.clear();
    for (auto & nodeLAC: nodeLACs) {
        if (nodeLAC.GetFOM() != 0)
            candLACs.emplace_back(nodeLAC);
    }
    sort(candLACs.begin(), candLACs.end());
    cout << "size of nodeLACs = " << nodeLACs.size() << endl;
    cout << "size of candLACs = " << candLACs.size() << endl;
    // for (auto & candLAC: candLACs) {
    //     candLAC.Print();
    //     cout << endl;
    // }
    
}


void SynthAndMap(Abc_Ntk_t * pNtk)
{
    string Command;
    string ntkName(pNtk->pName);
    float area, delay;

    Abc_Frame_t * pAbc = Abc_FrameGetGlobalFrame();
    Abc_FrameReplaceCurrentNetwork(pAbc, Abc_NtkDup(pNtk));

    for (int i = 0; i < 1; ++i) {
        Command = string("balance; rewrite; refactor; balance; rewrite; rewrite -z; balance; refactor -z; rewrite -z; balance");
        assert( !Cmd_CommandExecute(pAbc, Command.c_str()) );
        Command = string("map -a;");
        assert( !Cmd_CommandExecute(pAbc, Command.c_str()) );
    }
    area = Ckt_GetArea(Abc_FrameReadNtk(pAbc));
    delay = Abc_NtkDelayTrace(Abc_FrameReadNtk(pAbc), nullptr, nullptr, 0); 
    cout << "new area = " << area << endl;
    cout << "new delay = " << delay << endl;
}


int SASIMI_Manager_t::ApplyBestLAC(Simulator_t & oriSmlt, Simulator_t & appSmlt, vector <LAC_t> & candLACs, int topNum, string outPrefix, unsigned seed)
{
    if (candLACs.empty())
        return 1;
    Abc_Ntk_t * pOriNtk = oriSmlt.GetNetwork();
    Abc_Ntk_t * pAppNtk = appSmlt.GetNetwork();
    Abc_Obj_t * pTS = nullptr;
    Abc_Obj_t * pSS = nullptr;
    bool isInv = false;
    double error = DBL_MAX;
    double baseError = (metricType == Metric_t::ER)? GetER(&oriSmlt, &appSmlt) / static_cast <double> (nFrame): GetNMED(&oriSmlt, &appSmlt);
    double estiError = 0.0;

    for (int i = 0; i < topNum && i < static_cast <int> (candLACs.size()); ++i) {
        pTS = candLACs[i].GetTS();
        pSS = candLACs[i].GetSS();
        isInv = candLACs[i].GetIsInv();
        DASSERT(pTS != nullptr);
        DASSERT(pSS != nullptr);
        DASSERT(pTS->pNtk == pAppNtk && pSS->pNtk == pAppNtk);
        error = (metricType == Metric_t::ER)? MeasureSASIMIER(&oriSmlt, &appSmlt, pTS, pSS, isInv, true): MeasureSASIMINMED(&oriSmlt, &appSmlt, pTS, pSS, isInv, true);
        typedef boost::multiprecision::cpp_dec_float_100 bigFlt;
        typedef boost::multiprecision::int256_t bigInt;
        estiError = (metricType == Metric_t::ER)?
            baseError + candLACs[i].GetDError() / static_cast <double> (nFrame):
            baseError + static_cast <double> (static_cast <bigFlt> (candLACs[i].GetDError()) / static_cast <bigFlt> ((static_cast <bigInt> (1) << Abc_NtkPoNum(pAppNtk)) - 1));
        if (maxLevel == INT_MAX) {
            if (abs(error - estiError) > 1e-8)
                cout << "mismatch error: " << error << ", " << estiError << endl;
        }
        if (error <= errorBound)
            break;
    }
    if (pTS == nullptr || pSS == nullptr)
        return 1;
    if (error > errorBound)
        return 1;
    if (!isInv) {
        if (Abc_ObjIsNode(pSS) && Abc_NodeIsConst0(pSS))
            cout << Abc_ObjName(pTS) << " is replaced by zero with estimated error " << estiError << endl;
        else if (Abc_ObjIsNode(pSS) && Abc_NodeIsConst1(pSS))
            cout << Abc_ObjName(pTS) << " is replaced by one with estimated error " << estiError << endl;
        else
            cout << Abc_ObjName(pTS) << " is replaced by " << Abc_ObjName(pSS) << " with estimated error " << error << endl;
        ReplaceObj(pTS, pSS);
    }
    else {
        DASSERT(!(Abc_ObjIsNode(pSS) && Abc_NodeIsConst(pSS)));
        cout << Abc_ObjName(pTS) << " is replaced by " << Abc_ObjName(pSS) << " with inverter with estimated error " << error << endl;
        Abc_Obj_t * pInv = Abc_NtkCreateNodeInv(pAppNtk, pSS);
        ReplaceObj(pTS, pInv);
    }
    double accError = (metricType == Metric_t::ER)? MeasureER(pOriNtk, pAppNtk, nFrame, seed): MeasureNMED(pOriNtk, pAppNtk, nFrame, seed);
    cout << "error = " << accError << endl;
    DASSERT(accError == error);
    double highAccError = (metricType == Metric_t::ER)? MeasureER(pOriNtk, pAppNtk, 1000000, seed): MeasureNMED(pOriNtk, pAppNtk, 1000000, seed);
    cout << "high accuracy error = " << highAccError << endl;
    cout << "area = " << Ckt_GetArea(pAppNtk) << endl;
    cout << "delay = " << Ckt_GetDelay(pAppNtk) << endl;
    cout << "#gates = " << Abc_NtkNodeNum(pAppNtk) << endl;
    // SynthAndMap(pAppNtk);
    // if (highAccError > errorBound)
    //     assert(0);

    ostringstream command;
    command << outPrefix << "_" << cntRound << "_" << accError << "_" << Ckt_GetArea(pAppNtk) << "_" << Ckt_GetDelay(pAppNtk) << ".blif";
    cout << "output circuit " << command.str() << endl;
    Ckt_WriteBlif(pAppNtk, command.str());
    return 0;
}


int SASIMI_Manager_t::ApplyBestLACInaccurate(Simulator_t & oriSmlt, Simulator_t & appSmlt, vector <LAC_t> & candLACs, int topNum, string outPrefix, unsigned seed)
{
    if (candLACs.empty())
        return 1;
    Abc_Ntk_t * pOriNtk = oriSmlt.GetNetwork();
    Abc_Ntk_t * pAppNtk = appSmlt.GetNetwork();
    Abc_Obj_t * pTS = nullptr;
    Abc_Obj_t * pSS = nullptr;
    bool isInv = false;
    double error = DBL_MAX;
    double baseError = (metricType == Metric_t::ER)? GetER(&oriSmlt, &appSmlt) / static_cast <double> (nFrame): GetNMED(&oriSmlt, &appSmlt);
    double estiError = 0.0;

    for (int i = 0; i < topNum && i < static_cast <int> (candLACs.size()); ++i) {
        pTS = candLACs[i].GetTS();
        pSS = candLACs[i].GetSS();
        isInv = candLACs[i].GetIsInv();
        DASSERT(pTS != nullptr);
        DASSERT(pSS != nullptr);
        DASSERT(pTS->pNtk == pAppNtk && pSS->pNtk == pAppNtk);
        error = (metricType == Metric_t::ER)? MeasureSASIMIER(&oriSmlt, &appSmlt, pTS, pSS, isInv, true): MeasureSASIMINMED(&oriSmlt, &appSmlt, pTS, pSS, isInv, true);
        typedef boost::multiprecision::cpp_dec_float_100 bigFlt;
        typedef boost::multiprecision::int256_t bigInt;
        estiError = (metricType == Metric_t::ER)?
            baseError + candLACs[i].GetDError() / static_cast <double> (nFrame):
            baseError + static_cast <double> (static_cast <bigFlt> (candLACs[i].GetDError()) / static_cast <bigFlt> ((static_cast <bigInt> (1) << Abc_NtkPoNum(pAppNtk)) - 1));
        if (maxLevel == INT_MAX) {
            if (abs(error - estiError) > 1e-8)
                cout << "mismatch error: " << error << ", " << estiError << endl;
        }
        if (error <= errorBound)
            break;
    }
    if (pTS == nullptr || pSS == nullptr)
        return 1;
    // if (error > errorBound)
    //     return 1;
    if (!isInv) {
        if (Abc_ObjIsNode(pSS) && Abc_NodeIsConst0(pSS))
            cout << Abc_ObjName(pTS) << " is replaced by zero with estimated error " << estiError << endl;
        else if (Abc_ObjIsNode(pSS) && Abc_NodeIsConst1(pSS))
            cout << Abc_ObjName(pTS) << " is replaced by one with estimated error " << estiError << endl;
        else
            cout << Abc_ObjName(pTS) << " is replaced by " << Abc_ObjName(pSS) << " with estimated error " << error << endl;
        ReplaceObj(pTS, pSS);
    }
    else {
        DASSERT(!(Abc_ObjIsNode(pSS) && Abc_NodeIsConst(pSS)));
        cout << Abc_ObjName(pTS) << " is replaced by " << Abc_ObjName(pSS) << " with inverter with estimated error " << error << endl;
        Abc_Obj_t * pInv = Abc_NtkCreateNodeInv(pAppNtk, pSS);
        ReplaceObj(pTS, pInv);
    }
    double accError = (metricType == Metric_t::ER)? MeasureER(pOriNtk, pAppNtk, nFrame, seed): MeasureNMED(pOriNtk, pAppNtk, nFrame, seed);
    cout << "error = " << accError << endl;
    DASSERT(accError == error);
    double highAccError = (metricType == Metric_t::ER)? MeasureER(pOriNtk, pAppNtk, 1000000, seed): MeasureNMED(pOriNtk, pAppNtk, 1000000, seed);
    cout << "high accuracy error = " << highAccError << endl;
    cout << "area = " << Ckt_GetArea(pAppNtk) << endl;
    cout << "delay = " << Ckt_GetDelay(pAppNtk) << endl;
    cout << "#gates = " << Abc_NtkNodeNum(pAppNtk) << endl;
    // SynthAndMap(pAppNtk);
    if (highAccError > errorBound)
        assert(0);

    ostringstream command;
    command << outPrefix << "_" << cntRound << "_" << accError << "_" << Ckt_GetArea(pAppNtk) << "_" << Ckt_GetDelay(pAppNtk) << ".blif";
    cout << "output circuit " << command.str() << endl;
    Ckt_WriteBlif(pAppNtk, command.str());
    return 0;
}


void SASIMI_Manager_t::GetDER(IN Simulator_t & appSmlt, IN Abc_Obj_t * pTS, IN Abc_Obj_t * pSS, IN vector <tVec> & isERInc, IN vector <tVec> & isERDec, OUT pair <int, int> & dErrors)
{
    Abc_Ntk_t * pAppNtk = appSmlt.GetNetwork();
    DASSERT(pAppNtk == pTS->pNtk);
    DASSERT(pAppNtk == pSS->pNtk);
    int nBlock = appSmlt.GetBlockNum();
    int dError = 0, dInvError = 0;
    int incEr = 0, decEr = 0;
    for (int i = 0; i < nBlock; ++i) {
        uint64_t isChanged = appSmlt.GetValues(pTS, i) ^ appSmlt.GetValues(pSS, i);
        uint64_t incFlag = isERInc[pTS->Id][i] & isChanged;
        dError += Ckt_CountOneNum(incFlag);
        incEr += Ckt_CountOneNum(incFlag);
        uint64_t decFlag = isERDec[pTS->Id][i] & isChanged;
        dError -= Ckt_CountOneNum(decFlag);
        decEr += Ckt_CountOneNum(decFlag);
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


void SASIMI_Manager_t::GetDERInaccurate(IN Simulator_t & appSmlt, IN Abc_Obj_t * pTS, IN Abc_Obj_t * pSS, OUT pair <int, int> & dErrors)
{
    Abc_Ntk_t * pAppNtk = appSmlt.GetNetwork();
    DASSERT(pAppNtk == pTS->pNtk);
    DASSERT(pAppNtk == pSS->pNtk);
    int nBlock = appSmlt.GetBlockNum();
    int dError = 0, dInvError = 0;
    int incEr = 0, decEr = 0;
    for (int i = 0; i < nBlock; ++i) {
        uint64_t isChanged = appSmlt.GetValues(pTS, i) ^ appSmlt.GetValues(pSS, i);
        uint64_t incFlag = isChanged;
        dError += Ckt_CountOneNum(incFlag);
        incEr += Ckt_CountOneNum(incFlag);
        // uint64_t decFlag = isERDec[pTS->Id][i] & isChanged;
        // dError -= Ckt_CountOneNum(decFlag);
        // decEr += Ckt_CountOneNum(decFlag);
        if (!Abc_NodeIsConst(pSS)) {
            incFlag = ~isChanged;
            dInvError += Ckt_CountOneNum(incFlag);
            // decFlag = isERDec[pTS->Id][i] & ~isChanged;
            // dInvError -= Ckt_CountOneNum(decFlag);
        }
    }
    dErrors.first = dError;
    dErrors.second = dInvError;
}


void SASIMI_Manager_t::GetDNMED(IN Simulator_t & appSmlt, IN vector <int64_t> & oriOutputs, IN vector <int64_t> & appOutputs, IN vector <int64_t> & appOutputsNew, IN Abc_Obj_t * pTS, IN Abc_Obj_t * pSS, OUT pair <int64_t, int64_t> & dErrors)
{
    int64_t dError = 0;
    int64_t dInvError = 0;
    int nBlock = appSmlt.GetBlockNum();
    int frameCnt = 0;
    for (int i = 0; i < nBlock; ++i) {
        uint64_t isChanged = appSmlt.GetValues(pTS, i) ^ appSmlt.GetValues(pSS, i);
        for (int j = 0; j < 64; ++j) {
            if (frameCnt >= nFrame)
                break;
            if (isChanged & 1) {
                dError -= abs(oriOutputs[frameCnt] - appOutputs[frameCnt]);
                dError += abs(appOutputsNew[frameCnt] - oriOutputs[frameCnt]);
            }
            else {
                dInvError -= abs(oriOutputs[frameCnt] - appOutputs[frameCnt]);
                dInvError += abs(appOutputsNew[frameCnt] - oriOutputs[frameCnt]);
            }
            isChanged >>= 1;
            ++frameCnt;
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

    return dArea;
}


void SASIMI_Manager_t::ReplaceObj(Abc_Obj_t * pNodeOld, Abc_Obj_t * pNodeNew)
{
    assert( !Abc_ObjIsComplement(pNodeOld) );
    assert( !Abc_ObjIsComplement(pNodeNew) );
    assert( pNodeOld->pNtk == pNodeNew->pNtk );
    assert( pNodeOld != pNodeNew );
    assert( Abc_ObjFanoutNum(pNodeOld) > 0 );
    // transfer the fanouts to the old node
    Abc_ObjTransferFanout( pNodeOld, pNodeNew );
    // remove the old node
    DeleteObj_rec( pNodeOld );
}


void SASIMI_Manager_t::DeleteObj_rec(Abc_Obj_t * pObj)
{
    Vec_Ptr_t * vNodes;
    int i;
    assert( !Abc_ObjIsComplement(pObj) );
    assert( !Abc_ObjIsPi(pObj) );
    assert( Abc_ObjFanoutNum(pObj) == 0 );
    // delete fanins and fanouts
    vNodes = Vec_PtrAlloc( 100 );
    Abc_NodeCollectFanins( pObj, vNodes );
    Abc_NtkDeleteObj( pObj );
    Vec_PtrForEachEntry( Abc_Obj_t *, vNodes, pObj, i )
        if ( Abc_ObjIsNode(pObj) && Abc_ObjFanoutNum(pObj) == 0 && !Abc_NodeIsConst(pObj) )
            DeleteObj_rec( pObj );
    Vec_PtrFree( vNodes );
}


void SASIMI_Manager_t::ReorganizeBD(IN vector < vector <tVec> > & bds, IN Abc_Obj_t * pObj, OUT tVec & bdNode)
{
    // cpm[PO][obj][frame] -> cpm[frame][PO]
    int nPo = static_cast <int> (bds.size());
    DASSERT(nPo <= 33, "don't support two many POs");
    DASSERT(nPo);
    int nObj = static_cast <int> (bds[0].size());
    DASSERT(pObj->Id < nObj);
    DASSERT(nObj);
    int nBlock = static_cast <int> (bds[0][0].size());
    DASSERT(nBlock << 6 >= static_cast <int> (bdNode.size()));

    for (auto & bdNodeVal: bdNode)
        bdNodeVal = 0;
    for (int k = nPo - 1; k >= 0; --k) {
        tVec & bdPo = bds[k][pObj->Id];
        int frameCnt = 0;
        for (int i = 0; i < nBlock; ++i) {
            uint64_t bd = bdPo[i];
            for (int j = 0; j < 64; ++j) {
                if (frameCnt >= nFrame)
                    break;
                bdNode[frameCnt] <<= 1;
                if (bd & 1)
                    ++bdNode[frameCnt];
                bd >>= 1;
                ++frameCnt;
            }
        }
    }
}


LAC_t::LAC_t()
{
}


LAC_t::~LAC_t()
{
}


void LAC_t::Init()
{
    this->pTS = nullptr;
    this->pSS = nullptr;
    this->isInv = false;
    this->dError = 0.0;
    this->dArea = 0.0;
    this->FOM = 0.0;
}


void LAC_t::Update(Abc_Obj_t * pTS, Abc_Obj_t * pSS, bool isInv, double error, double dArea, double FOM)
{
    this->pTS = pTS;
    this->pSS = pSS;
    this->isInv = isInv;
    this->dError = error;
    this->dArea = dArea;
    this->FOM = FOM;
}


void LAC_t::Print() const
{
    if (pTS == nullptr || pSS == nullptr) {
        cout << "(null)";
    }
    else {
        if (Abc_ObjIsNode(pSS) && Abc_NodeIsConst0(pSS))
            cout << Abc_ObjName(pTS) << "," << "zero" << "," << isInv << "," << dError << "," << dArea << "," << FOM;
        else if (Abc_ObjIsNode(pSS) && Abc_NodeIsConst1(pSS))
            cout << Abc_ObjName(pTS) << "," << "one" << "," << isInv << "," << dError << "," << dArea << "," << FOM;
        else
            cout << Abc_ObjName(pTS) << "," << Abc_ObjName(pSS) << "," << isInv << "," << dError << "," << dArea << "," << FOM;
    }
}


bool LAC_t::operator < (const LAC_t & other) const
{
    if (this->FOM < 0 && other.FOM > 0)
        return true;
    if (this->FOM > 0 && other.FOM < 0)
        return false;
    if (this->FOM > 0 && other.FOM > 0) {
        if (this->FOM == other.FOM)
            return this->dArea > other.dArea;
        else
            return this->FOM > other.FOM;
    }
    if (this->FOM == other.FOM)
        return this->dArea > other.dArea;
    else
        return this->FOM < other.FOM;
}
