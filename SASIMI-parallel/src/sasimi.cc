#include "sasimi.h"


using namespace std;
using namespace std::filesystem;


SASIMI_Manager_t::SASIMI_Manager_t(Abc_Ntk_t * pNtk, int nFrame, Metric_t metricType, double errorBound):
    nFrame(nFrame), metricType(metricType), errorBound(errorBound)
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
    double error = 0;
    vector < vector <tVec> > bds; // cpm[o][n][i]
    vector <Vec_Ptr_t *> vMffcs;
    vector <LAC_t> nodeLACs;
    vector <LAC_t> candLACs;
    int cntRound = 0;
    random_device rd;
    clock_t st = clock();
    while (error < errorBound) {
        cout << "--------------- round " << ++cntRound << " ---------------" << endl;
        Simulator_t * pAppSmlt = new Simulator_t(pAppNtk, nFrame);
        unsigned seed = static_cast <unsigned> (rd());
        cout << "seed = " << seed << endl;
        oriSmlt.Input(seed);
        oriSmlt.Simulate();
        pAppSmlt->Input(seed);
        pAppSmlt->Simulate();
        GetCPM(oriSmlt, *pAppSmlt, bds);
        Abc_NtkDelayTrace(pAppNtk, nullptr, nullptr, 0);
        CollectMFFC(*pAppSmlt, vMffcs);
        CollectAllLACs(oriSmlt, *pAppSmlt, bds, vMffcs, nodeLACs);
        FreeMFFC(vMffcs);
        SortCandLACs(nodeLACs, pAppSmlt->GetFrameNum(), candLACs);
        int res = ApplyBestLAC(oriSmlt, *pAppSmlt, candLACs, 10, cntRound, outPrefix, seed);
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


void SASIMI_Manager_t::CollectAllLACs(IN Simulator_t & oriSmlt, IN Simulator_t & appSmlt, IN vector < vector <tVec> > & bds, IN vector <Vec_Ptr_t *> & vMffcs, OUT vector <LAC_t> & nodeLACs)
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

    // for (int i = 0; i < Abc_NtkPoNum(pOriNtk); ++i) {
    //     int j = 0;
    //     Abc_NtkForEachNode(pAppNtk, pObj, j) {
    //         if (string(Abc_ObjName(pObj)) == "[39009]" && cntRound == 6) {
    //             int k = 1363;
    //             cout << "Warning " << Abc_ObjName(Abc_NtkPo(pAppNtk, i)) << "," << Ckt_GetBit(bds[i][pObj->Id][k], 35) << "," << Ckt_GetBit(poKRight[i][k], 35) << endl;
    //         }
    //     }
    // }

    // update PI flag
    int sourceLen = (Abc_NtkPiNum(pAppNtk) >> 6) + 1;
    vector <tVec> sources(appSmlt.GetMaxId() + 1);
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
            CollectNodeLAC(pObj, appSmlt, isERInc, isERDec, sources, vMffcs, baseER, nodeLACs[pObj->Id]);
        }
    }
}


void SASIMI_Manager_t::CollectNodeLAC(IN Abc_Obj_t * pTS, IN Simulator_t & appSmlt, IN vector <tVec> & isERInc, IN vector <tVec> & isERDec, IN vector <tVec> & sources, IN vector <Vec_Ptr_t *> & vMffcs, IN int baseER, OUT LAC_t & nodeLAC)
{
    Abc_Ntk_t * pAppNtk = appSmlt.GetNetwork();
    DASSERT(pAppNtk == pTS->pNtk);
    double errorBoundFrame = errorBound * appSmlt.GetFrameNum();
    double invDelay = Mio_LibraryReadDelayInvMax((Mio_Library_t *)Abc_FrameReadLibGen()) + 0.1;
    int areaInv = Mio_LibraryReadAreaInv((Mio_Library_t *)Abc_FrameReadLibGen());
    int areaBuf = Mio_LibraryReadAreaBuf((Mio_Library_t *)Abc_FrameReadLibGen());
    nodeLAC.SetFOM(0.0);
    // consider constant replacement
    Abc_Obj_t * pConst0 = Ckt_GetConst(pAppNtk, 0);
    pair <int, int> dErrors;
    GetDError(appSmlt, pTS, pConst0, isERInc, isERDec, dErrors);
    if (baseER + dErrors.first <= errorBoundFrame) {
        double dArea = GetDArea(pTS, pConst0, vMffcs);
        double tempFOM = dArea / (dErrors.first + 1e-10);
        if ((dErrors.first < 0 && nodeLAC.GetFOM() > tempFOM) || (dErrors.first >= 0 && nodeLAC.GetFOM() >= 0 && nodeLAC.GetFOM() < tempFOM))
            nodeLAC.Update(pTS, pConst0, false, dErrors.first, dArea, tempFOM);
        // cout << Abc_ObjName(pTS) << ",0," << dErrors.first / static_cast <double>(appSmlt.GetFrameNum()) << ",," << dArea << endl;
    }
    Abc_Obj_t * pConst1 = Ckt_GetConst(pAppNtk, 1);
    GetDError(appSmlt, pTS, pConst1, isERInc, isERDec, dErrors);
    if (baseER + dErrors.first <= errorBoundFrame) {
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
        GetDError(appSmlt, pTS, pSS, isERInc, isERDec, dErrors);
        if (baseER + dErrors.first <= errorBoundFrame || (baseER + dErrors.second <= errorBoundFrame && Ckt_GetObjArrivalTime(pTS, 3) >= Ckt_GetObjArrivalTime(pSS, 3) +  invDelay * 1000)) {
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
            // cout << Abc_ObjName(pTS) << "," << Abc_ObjName(pSS) << "," << dErrors.first / static_cast <double>(appSmlt.GetFrameNum()) << "," << dErrors.second / static_cast <double>(appSmlt.GetFrameNum()) << "," << dArea << endl;
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
        GetDError(appSmlt, pTS, pSS, isERInc, isERDec, dErrors);
        if (baseER + dErrors.first <= errorBoundFrame || baseER + dErrors.second <= errorBoundFrame) {
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
            // cout << Abc_ObjName(pTS) << "," << Abc_ObjName(pSS) << "," << dErrors.first / static_cast <double>(appSmlt.GetFrameNum()) << "," << dErrors.second / static_cast <double>(appSmlt.GetFrameNum()) << "," << dArea << endl;
        }
    }
}


void SASIMI_Manager_t::SortCandLACs(IN std::vector <LAC_t> & nodeLACs, IN int nFrame, OUT std::vector <LAC_t> & candLACs)
{
    candLACs.clear();
    for (auto & nodeLAC: nodeLACs) {
        if (nodeLAC.GetFOM() != 0)
            candLACs.emplace_back(nodeLAC);
    }
    sort(candLACs.begin(), candLACs.end());
}


int SASIMI_Manager_t::ApplyBestLAC(Simulator_t & oriSmlt, Simulator_t & appSmlt, vector <LAC_t> & candLACs, int topNum, int cntRound, string outPrefix, unsigned seed)
{
    if (candLACs.empty())
        return 1;
    Abc_Ntk_t * pOriNtk = oriSmlt.GetNetwork();
    Abc_Ntk_t * pAppNtk = appSmlt.GetNetwork();
    Abc_Obj_t * pTS = nullptr;
    Abc_Obj_t * pSS = nullptr;
    bool isInv = false;
    double error = DBL_MAX;
    for (int i = 0; i < topNum && i < static_cast <int> (candLACs.size()); ++i) {
        pTS = candLACs[i].GetTS();
        pSS = candLACs[i].GetSS();
        isInv = candLACs[i].GetIsInv();
        DASSERT(pTS != nullptr);
        DASSERT(pSS != nullptr);
        DASSERT(pTS->pNtk == pAppNtk && pSS->pNtk == pAppNtk);
        error = MeasureSASIMIER(&oriSmlt, &appSmlt, pTS, pSS, isInv, true);
        if (error <= errorBound)
            break;
    }
    if (pTS == nullptr || pSS == nullptr)
        return 1;
    if (!isInv) {
        if (Abc_ObjIsNode(pSS) && Abc_NodeIsConst0(pSS))
            cout << Abc_ObjName(pTS) << " is replaced by zero with estimated error " << error << endl;
        else if (Abc_ObjIsNode(pSS) && Abc_NodeIsConst1(pSS))
            cout << Abc_ObjName(pTS) << " is replaced by one with estimated error " << error << endl;
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
    double accError = MeasureER(pOriNtk, pAppNtk, nFrame, seed);
    DASSERT(accError == error);
    cout << "error = " << accError << endl;
    cout << "area = " << Ckt_GetArea(pAppNtk) << endl;
    cout << "delay = " << Ckt_GetDelay(pAppNtk) << endl;
    cout << "#gates = " << Abc_NtkNodeNum(pAppNtk) << endl;

    ostringstream command;
    command << outPrefix << "_" << cntRound << "_" << accError << "_" << Ckt_GetArea(pAppNtk) << "_" << Ckt_GetDelay(pAppNtk) << ".blif";
    cout << "output circuit " << command.str() << endl;
    Ckt_WriteBlif(pAppNtk, command.str());
    return 0;
}


void SASIMI_Manager_t::GetDError(IN Simulator_t & appSmlt, IN Abc_Obj_t * pTS, IN Abc_Obj_t * pSS, IN vector <tVec> & isERInc, IN vector <tVec> & isERDec, OUT pair<int, int> & dErrors)
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
        // if (string(Abc_ObjName(pTS)) == "[39009]" && Abc_ObjIsNode(pSS) && Abc_NodeIsConst1(pSS) && decFlag && cntRound == 6) {
        //     cout << "Warning round " << cntRound << "," << endl;
        //     for (int k = 0; k < 64; ++k) {
        //         if (Ckt_GetBit(decFlag, k)) {
        //             cout << "Warning" << 64 * i + k << endl;
        //         }
        //     }
        // }
        if (!Abc_NodeIsConst(pSS)) {
            incFlag = isERInc[pTS->Id][i] & ~isChanged;
            dInvError += Ckt_CountOneNum(incFlag);
            decFlag = isERDec[pTS->Id][i] & ~isChanged;
            dInvError -= Ckt_CountOneNum(decFlag);
        }
    }
    // if (string(Abc_ObjName(pTS)) == "[39009]" && Abc_ObjIsNode(pSS) && Abc_NodeIsConst1(pSS)) {
    //     cout << "Warning round " << cntRound << "," << incEr << "," << decEr << "," << dError << endl;
    // }
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


void LAC_t::Print(int nFrame) const
{
    if (pTS == nullptr || pSS == nullptr) {
        cout << "(null)";
    }
    else {
        if (Abc_ObjIsNode(pSS) && Abc_NodeIsConst0(pSS))
            cout << "(" << Abc_ObjName(pTS) << "," << "zero" << "," << isInv << "," << dError / static_cast <double> (nFrame) << "," << dArea << "," << FOM * nFrame << ")";
        else if (Abc_ObjIsNode(pSS) && Abc_NodeIsConst1(pSS))
            cout << "(" << Abc_ObjName(pTS) << "," << "one" << "," << isInv << "," << dError / static_cast <double> (nFrame) << "," << dArea << "," << FOM * nFrame << ")";
        else
            cout << "(" << Abc_ObjName(pTS) << "," << Abc_ObjName(pSS) << "," << isInv << "," << dError / static_cast <double> (nFrame) << "," << dArea << "," << FOM * nFrame << ")";
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
