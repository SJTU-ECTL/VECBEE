#include "cktNtk.h"


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


Ckt_Ntk_t::Ckt_Ntk_t(Abc_Ntk_t * p_abc_ntk, int nFrames)
    : nValueClusters(nFrames / 64)
{
    Abc_Obj_t * pAbcObj, * pFanin;
    int i;

    // duplicate network
    pAbcNtk = Abc_NtkDup(p_abc_ntk);

    // init circuit objects, use pTemp to temporarily store the reflection
    Abc_NtkForEachObj(p_abc_ntk, pAbcObj, i) {
        cktObjs.emplace_back(Ckt_Obj_t(pAbcObj));
        pAbcObj->pTemp = static_cast <void *> (&(cktObjs.back()));
    }

    // get pointers of PI/PO/CONST0/CONST1
    pCktConst0 = pCktConst1 = nullptr;
    for (auto & obj : cktObjs) {
        if (obj.GetType() == Ckt_Obj_Type_t::PI)
            pCktPis.emplace_back(&obj);
        else if (obj.GetType() == Ckt_Obj_Type_t::PO)
            pCktPos.emplace_back(&obj);
        else if (obj.GetType() == Ckt_Obj_Type_t::CONST0)
            pCktConst0 = &obj;
        else if (obj.GetType() == Ckt_Obj_Type_t::CONST1)
            pCktConst1 = &obj;
    }
    if (pCktConst0 == nullptr) {
        cktObjs.emplace_back(Ckt_Obj_t(Abc_NtkCreateNodeConst0(pAbcNtk)));
        pCktConst0 = &(cktObjs.back());
    }
    if (pCktConst1 == nullptr) {
        cktObjs.emplace_back(Ckt_Obj_t(Abc_NtkCreateNodeConst1(pAbcNtk)));
        pCktConst1 = &(cktObjs.back());
    }

    // resize value clusters
    for (auto & obj : cktObjs)
        obj.ResizeClusters(nValueClusters);

    // add fanin/fanout information, use information saved in pTemp
    for (auto & obj : cktObjs) {
        Abc_ObjForEachFanin(obj.GetAbcObj(), pFanin, i)
            obj.AddFanin(static_cast <Ckt_Obj_t *> (pFanin->pTemp));
    }
}


Ckt_Ntk_t::~Ckt_Ntk_t(void)
{
    // free network (includes ABC objects added by user)
    Abc_NtkDelete(pAbcNtk);
}


void Ckt_Ntk_t::PrintInfo(void) const
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


void Ckt_Ntk_t::PrintTopologicalOrder(void)
{
    cout << "---------------- Topological order   ----------------" << endl;
    vector <Ckt_Obj_t *> pOrderedObjs;
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


void Ckt_Ntk_t::GenInputDist(unsigned seed)
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


void Ckt_Ntk_t::PrintInputDistribution(void) const
{
    for (auto & pCktPi : pCktPis)
        pCktPi->PrintClusters();
}


void Ckt_Ntk_t::DFS(Ckt_Obj_t * pCktObj, vector <Ckt_Obj_t *> & pOrderedObjs)
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
}


void Ckt_Ntk_t::SortObjects(vector <Ckt_Obj_t *> & pOrderedObjs)
{
    // reset traversal flag
    for (auto & pCktObj : cktObjs)
        pCktObj.ResetVisited();
    // start a vector of nodes
    pOrderedObjs.reserve(GetObjNum());
    // topological sort
    for (auto & pCktPo : pCktPos)
        DFS(pCktPo, pOrderedObjs);
}


void Ckt_Ntk_t::FeedForward(void)
{
    // get topological order
    vector <Ckt_Obj_t *> pOrderedObjs;
    SortObjects(pOrderedObjs);
    // update
    for (auto & pCktObj : pOrderedObjs)
        pCktObj->UpdateClusters();
}


void Ckt_Ntk_t::SimulatorChecker(void)
{
    // only for c6288
    assert(string(pAbcNtk->pName) == "c6288");
    assert(pCktPis.size() == 32 && pCktPos.size() == 32);
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


float Ckt_Ntk_t::GetErrorRate(Ckt_Ntk_t & refNtk, Ckt_Bit_Cnt_t & table)
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
        ret += table.GetOneNum(temp);
    }
    return static_cast <float> (ret) / static_cast <float> (nValueClusters << 6);
}


Ckt_Obj_t * Ckt_Ntk_t::AddInverter(Ckt_Obj_t & cktObj)
{
    Ckt_Obj_Type_t type = cktObj.GetType();
    assert(type != Ckt_Obj_Type_t::PO     && type != Ckt_Obj_Type_t::INV &&
           type != Ckt_Obj_Type_t::CONST0 && type != Ckt_Obj_Type_t::CONST1);
    assert(pAbcNtk == cktObj.GetAbcObj()->pNtk);

    Abc_Obj_t * pAbcObjNew = Abc_NtkCreateNodeInv(pAbcNtk, cktObj.GetAbcObj());

    cktObjs.emplace_back(Ckt_Obj_t(pAbcObjNew));
    Ckt_Obj_t * pCktObjNew = &(cktObjs.back());
    pCktObjNew->AddFanin(&cktObj);

    return pCktObjNew;
}
