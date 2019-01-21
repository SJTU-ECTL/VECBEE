#include "cktSopNet.h"


using namespace std;
using namespace abc;
using namespace boost;


Ckt_Sop_Net_t::Ckt_Sop_Net_t(int nFrames)
{
    // allocate a new network
    pAbcNtk = Abc_NtkAlloc(ABC_NTK_LOGIC, ABC_FUNC_SOP, 1);

    // assign simulation clusters number
    nValueClusters = nFrames / 64;

    cktObjs.clear();
    pCktPis.clear();
    pCktPos.clear();
}


Ckt_Sop_Net_t::Ckt_Sop_Net_t(Abc_Ntk_t * p_abc_ntk, int nFrames)
{
    Abc_Obj_t * pAbcObj, * pFanin;
    int i;

    assert(Abc_NtkIsSopLogic(p_abc_ntk));

    // duplicate network
    pAbcNtk = Abc_NtkDup(p_abc_ntk);

    // assign simulation clusters number
    nValueClusters = nFrames / 64;

    // init circuit objects, use pTemp to temporarily store the reflection
    Abc_NtkForEachObj(pAbcNtk, pAbcObj, i) {
        cktObjs.emplace_back(Ckt_Sop_t(pAbcObj, this));
        pAbcObj->pTemp = static_cast <void *> (&(cktObjs.back()));
    }

    // get pointers of PI/PO/CONST0/CONST1
    for (auto & obj : cktObjs) {
        if (obj.IsPI())
            pCktPis.emplace_back(&obj);
        else if (obj.IsPO())
            pCktPos.emplace_back(&obj);
    }

    // add fanin/fanout information, use information saved in pTemp
    for (auto & obj : cktObjs) {
        Abc_ObjForEachFanin(obj.GetAbcObj(), pFanin, i)
            obj.AddFanin(static_cast <Ckt_Sop_t *> (pFanin->pTemp));
    }
}


Ckt_Sop_Net_t::Ckt_Sop_Net_t(const Ckt_Sop_Net_t & other)
{
    Abc_Obj_t * pAbcObj, * pFanin;
    int i;

    assert(Abc_NtkIsSopLogic(other.pAbcNtk));

    // deep copy
    pAbcNtk = Abc_NtkDup(other.pAbcNtk);

    // assign simulation clusters number
    nValueClusters = other.nValueClusters;

    // init circuit objects, use pTemp to temporarily store the reflection
    Abc_NtkForEachObj(pAbcNtk, pAbcObj, i) {
        cktObjs.emplace_back(Ckt_Sop_t(pAbcObj, this));
        pAbcObj->pTemp = static_cast <void *> (&(cktObjs.back()));
    }

    // get pointers of PI/PO/CONST0/CONST1
    for (auto & obj : cktObjs) {
        if (obj.IsPI())
            pCktPis.emplace_back(&obj);
        else if (obj.IsPO())
            pCktPos.emplace_back(&obj);
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
    int maxNLiterals = -INT_MAX;
    cout << "---------------- Network information ----------------" << endl;
    cout << "Name\tType\tSOP\t#liter" << endl;
    for (auto & obj : cktObjs) {
        cout << obj.GetName() << "\t" << obj.GetType() << "\t" << obj.GetSOP() << "\t" << obj.GetNLiterals() << endl;
        maxNLiterals = max(maxNLiterals, obj.GetNLiterals());
    }
    cout << "Max #literals = " << maxNLiterals << endl;
}


void Ckt_Sop_Net_t::PrintTopoOrder(void)
{
    cout << "---------------- Topological order   ----------------" << endl;
    vector <Ckt_Sop_t *> pOrderedObjs;
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


void Ckt_Sop_Net_t::GenInputDist(unsigned seed)
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
}


void Ckt_Sop_Net_t::PrintInput(void) const
{
    for (auto & pCktPi : pCktPis) {
        cout << pCktPi->GetName() << endl;
        pCktPi->PrintClusters();
        cout << endl;
    }
}


void Ckt_Sop_Net_t::PrintOutput(void) const
{
    for (auto & pCktPo : pCktPos) {
        cout << pCktPo->GetName() << endl;
        pCktPo->PrintClusters();
        cout << endl;
    }
}


void Ckt_Sop_Net_t::DFS(Ckt_Sop_t * pCktObj, vector <Ckt_Sop_t *> & pOrderedObjs)
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


void Ckt_Sop_Net_t::SetAllUnvisited(void)
{
    for (auto & pCktObj : cktObjs)
        pCktObj.ResetVisited();
}


void Ckt_Sop_Net_t::SetAllUnvisited2(void)
{
    for (auto & pCktObj : cktObjs)
        pCktObj.ResetVisited2();
}


void Ckt_Sop_Net_t::SortObjects(vector <Ckt_Sop_t *> & pOrderedObjs)
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


void Ckt_Sop_Net_t::FeedForward(void)
{
    // get topological order
    vector <Ckt_Sop_t *> pOrderedObjs;
    SortObjects(pOrderedObjs);
    // update
    for (auto & pCktObj : pOrderedObjs)
        pCktObj->UpdateClusters();
}


void Ckt_Sop_Net_t::FeedForward(vector <Ckt_Sop_t *> & pOrderedObjs)
{
    // update
    for (auto & pCktObj : pOrderedObjs)
        pCktObj->UpdateClusters();
}


void Ckt_Sop_Net_t::FeedForward(list <Ckt_Sop_t *> & subNtk, int i)
{
    for (auto & pCktObj : subNtk)
        pCktObj->UpdateCluster(i);
}


void Ckt_Sop_Net_t::CheckSimulator(void)
{
    // only for c6288
    assert(string(pAbcNtk->pName) == "c6288" || string(pAbcNtk->pName) == "c6288.blif");
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


void Ckt_Sop_Net_t::TestSimulatorSpeed(void)
{
    GenInputDist();
    vector <Ckt_Sop_t *> pOrderedObjs;
    SortObjects(pOrderedObjs);
    clock_t st = clock();
    FeedForward(pOrderedObjs);
    clock_t ed = clock();
    cout << "circuit = " << GetName() << endl;
    cout << "frame number = " << nValueClusters * 64 << endl;
    cout << "time = " << ed - st << " us" << endl;
}


int Ckt_Sop_Net_t::GetErrorRate(Ckt_Sop_Net_t & refNtk)
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


void Ckt_Sop_Net_t::Replace(Ckt_Sop_t & cktObj, vector <string> & newSOP, Ckt_Sop_Cat_t _type, Ckt_Sing_Sel_Info_t & info)
{
    assert(cktObj.GetAbcObj()->pNtk == GetAbcNtk());
    cktObj.ReplaceBy(newSOP, _type, info);
}


void Ckt_Sop_Net_t::RecoverFromRpl(Ckt_Sing_Sel_Info_t & info)
{
    info.pCktObj->SetSOP(info.SOP);
    info.pCktObj->SetType(info.type);
    // change SOP of ABC object
    string tmp("");
    if (info.type == Ckt_Sop_Cat_t::INTER) {
        for (auto & cube : info.SOP) {
            tmp += cube;
            tmp += " 1\n";
        }
        tmp += "\0";
    }
    else if (info.type == Ckt_Sop_Cat_t::CONST0)
        tmp = " 0\n\0";
    else if (info.type == Ckt_Sop_Cat_t::CONST1)
        tmp = " 1\n\0";
    else
        assert(0);
    memcpy(info.pCktObj->GetAbcObj()->pData, tmp.c_str(), tmp.length() + 1);
}


void Ckt_Sop_Net_t::CheckFanio(void) const
{
    for (auto & cktObj : cktObjs)
        cktObj.CheckFanio();
}


void Ckt_Sop_Net_t::UpdateFoCone(void)
{
    vector <Ckt_Sop_t *> pOrdObjs;
    SortObjects(pOrdObjs);
    for (auto & cktObj : cktObjs)
        cktObj.InitFoCone(GetPoNum());
    for (int i = 0; i < GetPoNum(); ++i)
        pCktPos[i]->SetFoCone(i);
    for (auto it = pOrdObjs.rbegin(); it != pOrdObjs.rend(); ++it) {
        for (int i = 0; i < (*it)->GetFanoutNum(); ++i) {
            Ckt_Sop_t * pCktFo = (*it)->GetFanout(i);
            assert((*it)->GetFoConeSize() == pCktFo->GetFoConeSize());
            (*it)->SelfOrFoCone(pCktFo);
        }
    }
}


void Ckt_Sop_Net_t::PrintSimRes(void) const
{
    for (auto & cktObj : cktObjs) {
        cout << cktObj.GetName() << ":";
        cktObj.PrintClusters();
        cout << endl;
    }
}
