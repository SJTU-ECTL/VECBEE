#include "cktSopNet.h"


using namespace std;
using namespace abc;
using namespace boost;


Ckt_Sop_Net_t::Ckt_Sop_Net_t(Abc_Ntk_t * p_abc_ntk, int nFrames)
    : nValueClusters(nFrames / 64)
{
    Abc_Obj_t * pAbcObj, * pFanin;
    int i;

    assert(Abc_NtkIsSopLogic(p_abc_ntk));

    // duplicate network
    pAbcNtk = Abc_NtkDup(p_abc_ntk);

    // init circuit objects, use pTemp to temporarily store the reflection
    Abc_NtkForEachObj(pAbcNtk, pAbcObj, i) {
        cktObjs.emplace_back(Ckt_Sop_t(pAbcObj, this));
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
        cktObjs.emplace_back(Ckt_Sop_t(Abc_NtkCreateNodeConst0(pAbcNtk), this));
        pCktConst0 = &(cktObjs.back());
    }
    if (pCktConst1 == nullptr) {
        cktObjs.emplace_back(Ckt_Sop_t(Abc_NtkCreateNodeConst1(pAbcNtk), this));
        pCktConst1 = &(cktObjs.back());
    }

    // add fanin/fanout information, use information saved in pTemp
    for (auto & obj : cktObjs) {
        Abc_ObjForEachFanin(obj.GetAbcObj(), pFanin, i)
            obj.AddFanin(static_cast <Ckt_Sop_t *> (pFanin->pTemp));
    }
}


Ckt_Sop_Net_t::Ckt_Sop_Net_t(const Ckt_Sop_Net_t & other)
    : nValueClusters(other.nValueClusters)
{
    Abc_Obj_t * pAbcObj, * pFanin;
    int i;

    // deep copy
    pAbcNtk = Abc_NtkDup(other.pAbcNtk);

    // init circuit objects, use pTemp to temporarily store the reflection
    Abc_NtkForEachObj(pAbcNtk, pAbcObj, i) {
        cktObjs.emplace_back(Ckt_Sop_t(pAbcObj, this));
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
        cktObjs.emplace_back(Ckt_Sop_t(Abc_NtkCreateNodeConst0(pAbcNtk), this));
        pCktConst0 = &(cktObjs.back());
    }
    if (pCktConst1 == nullptr) {
        cktObjs.emplace_back(Ckt_Sop_t(Abc_NtkCreateNodeConst1(pAbcNtk), this));
        pCktConst1 = &(cktObjs.back());
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
        cout << obj.GetName() << "\t" << obj.GetType() << "\t";
        obj.PrintSOP();
        cout << "\t" << obj.GetNLiterals() << endl;
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
    for (int i = 0; i < nValueClusters; ++i) {
        pCktConst0->SetCluster(i, static_cast <uint64_t> (0));
        pCktConst1->SetCluster(i, static_cast <uint64_t> (ULLONG_MAX));
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


void Ckt_Sop_Net_t::BackupSimRes(void)
{
    for (auto & cktObj : cktObjs)
        cktObj.BackupClusters();
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
