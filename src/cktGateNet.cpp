#include "cktGateNet.h"


using namespace std;
using namespace abc;
using namespace boost;


Ckt_Bit_Cnt_t::Ckt_Bit_Cnt_t(void)
{
    for (int i = 0; i < 65536; ++i) {
        int x = i;
        table[i] = 0;
        while (x) {
            x = x & (x - 1);
            table[i]++;
        }
    }
}


Ckt_Bit_Cnt_t::~Ckt_Bit_Cnt_t(void)
{
}


Ckt_Gate_Net_t::Ckt_Gate_Net_t(Abc_Ntk_t * p_abc_ntk, int nFrames)
    : nValueClusters(nFrames / 64)
{
    Abc_Obj_t * pAbcObj, * pFanin;
    int i;

    assert(Abc_NtkIsMappedLogic(p_abc_ntk));

    // duplicate network
    pAbcNtk = Abc_NtkDup(p_abc_ntk);

    // init circuit objects, use pTemp to temporarily store the reflection
    Abc_NtkForEachObj(pAbcNtk, pAbcObj, i) {
        cktObjs.emplace_back(Ckt_Gate_t(pAbcObj, this));
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
        cktObjs.emplace_back(Ckt_Gate_t(Abc_NtkCreateNodeConst0(pAbcNtk), this));
        pCktConst0 = &(cktObjs.back());
    }
    if (pCktConst1 == nullptr) {
        cktObjs.emplace_back(Ckt_Gate_t(Abc_NtkCreateNodeConst1(pAbcNtk), this));
        pCktConst1 = &(cktObjs.back());
    }

    // add fanin/fanout information, use information saved in pTemp
    for (auto & obj : cktObjs) {
        Abc_ObjForEachFanin(obj.GetAbcObj(), pFanin, i)
            obj.AddFanin(static_cast <Ckt_Gate_t *> (pFanin->pTemp));
    }
}


Ckt_Gate_Net_t::Ckt_Gate_Net_t(const Ckt_Gate_Net_t & other)
    : nValueClusters(other.nValueClusters)
{
    Abc_Obj_t * pAbcObj, * pFanin;
    int i;

    // deep copy
    pAbcNtk = Abc_NtkDup(other.pAbcNtk);

    // init circuit objects, use pTemp to temporarily store the reflection
    Abc_NtkForEachObj(pAbcNtk, pAbcObj, i) {
        cktObjs.emplace_back(Ckt_Gate_t(pAbcObj, this));
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
        cktObjs.emplace_back(Ckt_Gate_t(Abc_NtkCreateNodeConst0(pAbcNtk), this));
        pCktConst0 = &(cktObjs.back());
    }
    if (pCktConst1 == nullptr) {
        cktObjs.emplace_back(Ckt_Gate_t(Abc_NtkCreateNodeConst1(pAbcNtk), this));
        pCktConst1 = &(cktObjs.back());
    }

    // add fanin/fanout information, use information saved in pTemp
    for (auto & obj : cktObjs) {
        Abc_ObjForEachFanin(obj.GetAbcObj(), pFanin, i)
            obj.AddFanin(static_cast <Ckt_Gate_t *> (pFanin->pTemp));
    }
}


Ckt_Gate_Net_t::~Ckt_Gate_Net_t(void)
{
    // free network (includes ABC objects added by user)
    Abc_NtkDelete(pAbcNtk);
}


void Ckt_Gate_Net_t::PrintInfo(void) const
{
    cout << "---------------- Network information ----------------" << endl;
    cout << "Name\t\tType\t";
    cout << setw(30) << setiosflags(ios::left) << "Fanin";
    cout << setw(30) << setiosflags(ios::left) << "Fanout";
    cout << endl;
    for (auto & obj : cktObjs) {
        cout << obj.GetName() << "\t\t" << obj.GetType() << "\t";
        obj.PrintFanios();
        cout << endl;
    }
}


void Ckt_Gate_Net_t::PrintTopoOrder(void)
{
    cout << "---------------- Topological order   ----------------" << endl;
    vector <Ckt_Gate_t *> pOrderedObjs;
    SortObjects(pOrderedObjs);
    cout << "Name\t\tType\t";
    cout << setw(30) << setiosflags(ios::left) << "Fanin";
    cout << setw(30) << setiosflags(ios::left) << "Fanout";
    cout << endl;
    for (auto & pObj : pOrderedObjs) {
        cout << pObj->GetName() << "\t\t" << pObj->GetType() << "\t";
        pObj->PrintFanios();
        cout << endl;
    }
}


void Ckt_Gate_Net_t::GenInputDist(unsigned seed)
{
    uniform_int <> distribution(0, 1);
    boost::random::mt19937 engine(seed);
    variate_generator<boost::random::mt19937, uniform_int <> > randomPI(engine, distribution);
    for (auto & pCktPi : pCktPis) {
        assert(pCktPi->GetClustersSize() == nValueClusters);
        for (int i = 0; i < nValueClusters; ++i) {
            uint64_t value = 0;
            for (int j = 0; j < 64; ++j) {
                if (randomPI())
                    Ckt_SetBit(value, static_cast <uint64_t> (j));
            }
            pCktPi->SetCluster(i, value);
        }
    }
    for (int i = 0; i < nValueClusters; ++i) {
        pCktConst0->SetCluster(i, static_cast <uint64_t> (0));
        pCktConst1->SetCluster(i, static_cast <uint64_t> (ULLONG_MAX));
    }
}


void Ckt_Gate_Net_t::PrintInput(void) const
{
    for (auto & pCktPi : pCktPis) {
        cout << pCktPi->GetName() << endl;
        pCktPi->PrintClusters();
        cout << endl;
    }
}


void Ckt_Gate_Net_t::PrintOutput(void) const
{
    for (auto & pCktPo : pCktPos) {
        cout << pCktPo->GetName() << endl;
        pCktPo->PrintClusters();
        cout << endl;
    }
}


void Ckt_Gate_Net_t::DFS(Ckt_Gate_t * pCktObj, vector <Ckt_Gate_t *> & pOrderedObjs)
{
    // if this node is already visited, skip
    if (pCktObj->GetVisited())
        return;
    // mark the node as visited
    pCktObj->SetVisited();
    // visit the transitive fanin of the node
    for (int i = 0; i < pCktObj->GetFaninNum(); ++i)
        DFS(pCktObj->GetFanin(i), pOrderedObjs);
    // add the node after the fanins have been added
    pOrderedObjs.emplace_back(pCktObj);
    pCktObj->SetTopoId(static_cast <int> (pOrderedObjs.size()));
}


void Ckt_Gate_Net_t::SetAllUnvisited(void)
{
    for (auto & pCktObj : cktObjs)
        pCktObj.ResetVisited();
}


void Ckt_Gate_Net_t::SetAllUnvisited2(void)
{
    for (auto & pCktObj : cktObjs)
        pCktObj.ResetVisited2();
}


void Ckt_Gate_Net_t::SortObjects(vector <Ckt_Gate_t *> & pOrderedObjs)
{
    pOrderedObjs.clear();
    // reset traversal flag
    SetAllUnvisited();
    // start a vector of nodes
    pOrderedObjs.reserve(GetObjNum());
    // topological sort
    for (auto & pCktPo : pCktPos)
        DFS(pCktPo, pOrderedObjs);
}


void Ckt_Gate_Net_t::FeedForward(void)
{
    // get topological order
    vector <Ckt_Gate_t *> pOrderedObjs;
    SortObjects(pOrderedObjs);
    // update
    for (auto & pCktObj : pOrderedObjs)
        pCktObj->UpdateClusters();
}


void Ckt_Gate_Net_t::FeedForward(vector <Ckt_Gate_t *> & pOrderedObjs)
{
    // update
    for (auto & pCktObj : pOrderedObjs)
        pCktObj->UpdateClusters();
}


void Ckt_Gate_Net_t::FeedForward(list <Ckt_Gate_t *> & subNtk, int i)
{
    for (auto & pCktObj : subNtk)
        pCktObj->UpdateCluster(i);
}


void Ckt_Gate_Net_t::BackupSimRes(void)
{
    for (auto & cktObj : cktObjs)
        cktObj.BackupClusters();
}


void Ckt_Gate_Net_t::CheckSimulator(void)
{
    // only for c6288
    assert(string(pAbcNtk->pName) == "c6288");
    assert(pCktPis.size() == 32 && pCktPos.size() == 32);
    GenInputDist();
    FeedForward();
    for (int i = 0; i < nValueClusters; ++i) {
        for (int j = 0; j < 64; ++j) {
            uint64_t f1 = 0;
            for (int k = 15; k >= 0; --k) {
                if (Ckt_GetBit(pCktPis[k]->GetCluster(i), static_cast <uint64_t> (j)))
                    Ckt_SetBit(f1, static_cast <uint64_t> (k));
            }
            uint64_t f2 = 0;
            for (int k = 31; k >= 16; --k) {
                if (Ckt_GetBit(pCktPis[k]->GetCluster(i), static_cast <uint64_t> (j)))
                    Ckt_SetBit(f2, static_cast <uint64_t> (k - 16));
            }
            uint64_t res = 0;
            for (int k = 29; k >= 0; --k) {
                if (Ckt_GetBit(pCktPos[k]->GetCluster(i), static_cast <uint64_t> (j)))
                    Ckt_SetBit(res, static_cast <uint64_t> (k));
            }
            if (Ckt_GetBit(pCktPos[30]->GetCluster(i), static_cast <uint64_t> (j)))
                Ckt_SetBit(res, static_cast <uint64_t> (31));
            if (Ckt_GetBit(pCktPos[31]->GetCluster(i), static_cast <uint64_t> (j)))
                Ckt_SetBit(res, static_cast <uint64_t> (30));
            cout << f1 << "*" << f2 << "=" << res << endl;
            assert(f1 * f2 == res);
        }
    }
}


void Ckt_Gate_Net_t::TestSimulatorSpeed(void)
{
    GenInputDist();
    vector <Ckt_Gate_t *> pOrderedObjs;
    SortObjects(pOrderedObjs);
    clock_t st = clock();
    FeedForward(pOrderedObjs);
    clock_t ed = clock();
    cout << "circuit = " << GetName() << endl;
    cout << "frame number = " << nValueClusters * 64 << endl;
    cout << "time = " << ed - st << " us" << endl;
}


int Ckt_Gate_Net_t::GetErrorRate(Ckt_Gate_Net_t & refNtk)
{
    // make sure POs are same
    assert(pCktPos.size() == refNtk.pCktPos.size());
    int poNum = static_cast <int> (pCktPos.size());
    for (int i = 0; i < poNum; ++i)
        assert(pCktPos[i]->GetName() == refNtk.pCktPos[i]->GetName());
    // compare value cluster of POs
    int ret = 0;
    for (int k = 0; k < nValueClusters; ++k) {
        uint64_t temp = 0;
        for (int i = 0; i < poNum; ++i)
            temp |= pCktPos[i]->GetCluster(k) ^ refNtk.pCktPos[i]->GetCluster(k);
        ret += Ckt_CountOneNum(temp);
    }
    return ret;
}


Ckt_Gate_t * Ckt_Gate_Net_t::AddInverter(Ckt_Gate_t & cktObj)
{
    assert(!cktObj.IsPO() && !cktObj.IsInv() && !cktObj.IsConst());
    assert(pAbcNtk == cktObj.GetAbcObj()->pNtk);

    Abc_Obj_t * pAbcObjNew = Abc_NtkCreateNodeInv(pAbcNtk, cktObj.GetAbcObj());

    cktObjs.emplace_back(Ckt_Gate_t(pAbcObjNew, this));
    Ckt_Gate_t * pCktObjNew = &(cktObjs.back());
    pCktObjNew->AddFanin(&cktObj);
    cktObj.SetAddedInv(*pCktObjNew);

    return pCktObjNew;
}


Ckt_Gate_t * Ckt_Gate_Net_t::GetInverter(Ckt_Gate_t & cktObj)
{
    if (cktObj.HasAddedInv())
        return cktObj.GetAddedInv();
    else
        return AddInverter(cktObj);
}


Ckt_Gate_t * Ckt_Gate_Net_t::GetInverter2(Ckt_Gate_t & cktObj)
{
    Ckt_Gate_t * pRet = nullptr;
    if (cktObj.HasAddedInv())
        pRet = cktObj.GetAddedInv();
    else {
        pRet = AddInverter(cktObj);
        pRet->UpdateClusters();
    }
    return pRet;
}


void Ckt_Gate_Net_t::Replace(Ckt_Gate_t & cktOldObj, Ckt_Gate_t & cktNewObj, vector <Ckt_Rpl_Info_t> & info, bool isInv)
{
    assert(cktOldObj.GetAbcObj()->pNtk == GetAbcNtk());
    assert(cktNewObj.GetAbcObj()->pNtk == GetAbcNtk());
    if (!isInv)
        cktOldObj.ReplaceBy(cktNewObj, info);
    else {
        assert(!cktOldObj.IsInv() && !cktNewObj.IsInv());
        assert(!cktNewObj.IsConst());
        Ckt_Gate_t * pCktNewInv = GetInverter(cktNewObj);
        cktOldObj.ReplaceBy(*pCktNewInv, info);
    }
}


void Ckt_Gate_Net_t::ReplaceByName(string oldName, string newName, vector <Ckt_Rpl_Info_t> & info)
{
    auto itCktOldObj = find_if(cktObjs.begin(), cktObjs.end(),
                [&oldName](Ckt_Gate_t & obj) {
                    return obj.GetName() == oldName;
                }
            );
    assert(itCktOldObj != cktObjs.end());

    auto itCktNewObj = find_if(cktObjs.begin(), cktObjs.end(),
                [&newName](Ckt_Gate_t & obj) {
                    return obj.GetName() == newName;
                }
            );
    assert(itCktNewObj != cktObjs.end());

    assert(itCktOldObj->GetAbcObj()->pNtk == pAbcNtk);
    itCktOldObj->ReplaceBy(*itCktNewObj, info);
}


void Ckt_Gate_Net_t::RecoverFromRpl(vector <Ckt_Rpl_Info_t> & info)
{
    for (auto it = info.rbegin(); it != info.rend(); ++it) {
        // recover ABC
        Vec_IntPop(&(it->pCktObjTo->GetFanin(it->iCktFanin)->GetAbcObj())->vFanouts);
        Vec_IntInsert(
                &(it->pCktObjFrom->GetAbcObj())->vFanouts,
                it->iCktFanout,
                it->pCktObjTo->GetAbcObj()->Id
        );
        Vec_IntWriteEntry(
                &(it->pCktObjTo->GetAbcObj())->vFanins,
                it->iCktFanin,
                it->pCktObjFrom->GetAbcObj()->Id
        );
        // recover circuit
        it->pCktObjTo->GetFanin(it->iCktFanin)->PopBackFanout();
        it->pCktObjFrom->InsertFanout(it->iCktFanout, it->pCktObjTo);
        it->pCktObjTo->WriteFanin(it->iCktFanin, it->pCktObjFrom);
    }
}


void Ckt_Gate_Net_t::CheckFanio(void) const
{
    for (auto & cktObj : cktObjs)
        cktObj.CheckFanio();
}


void Ckt_Gate_Net_t::UpdateFoCone(void)
{
    vector <Ckt_Gate_t *> pOrdObjs;
    SortObjects(pOrdObjs);
    for (auto & cktObj : cktObjs)
        cktObj.InitFoCone(GetPoNum());
    for (int i = 0; i < GetPoNum(); ++i)
        pCktPos[i]->SetFoCone(i);
    for (auto it = pOrdObjs.rbegin(); it != pOrdObjs.rend(); ++it) {
        for (int i = 0; i < (*it)->GetFanoutNum(); ++i) {
            Ckt_Gate_t * pCktFo = (*it)->GetFanout(i);
            assert((*it)->GetFoConeSize() == pCktFo->GetFoConeSize());
            (*it)->SelfOrFoCone(pCktFo);
        }
    }
}


void Ckt_Gate_Net_t::PrintCut(void) const
{
    for (auto & cktObj : cktObjs) {
        cout << cktObj.GetName() << ":";
        for (auto & pCktFg : cktObj.cut) {
            cout << pCktFg->GetName() << "\t";
        }
        cout << endl;
    }
}


void Ckt_Gate_Net_t::PrintCutNtk(void) const
{
    for (auto & cktObj : cktObjs) {
        cout << cktObj.GetName() << ":";
        for (auto & pCktObj: cktObj.cutNtk) {
            cout << pCktObj->GetName() << "\t";
        }
        cout << endl;
    }
}


void Ckt_Gate_Net_t::PrintSimRes(void) const
{
    for (auto & cktObj : cktObjs) {
        cout << cktObj.GetName() << ":";
        cktObj.PrintClusters();
        cout << endl;
    }
}


void Ckt_Gate_Net_t::PrintBD(void) const
{
    for (auto & cktObj : cktObjs)
        cktObj.PrintBD();
}
