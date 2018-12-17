#include "cktNtk.h"


using namespace std;
using namespace abc;
using namespace boost;


Ckt_Ntk_t::Ckt_Ntk_t(Abc_Ntk_t * p_abc_ntk, int nFrames)
    : nValueClusters(nFrames / 64)
{
    pAbcNtk = Abc_NtkDup(p_abc_ntk);
    Abc_Obj_t * pAbcObj;
    int i;
    Abc_NtkForEachObj(p_abc_ntk, pAbcObj, i)
        cktObjs.emplace_back(Ckt_Obj_t(pAbcObj));
    for (auto & obj : cktObjs) {
        obj.ClustersPreAlloc(nValueClusters);
        if (obj.GetType() == Ckt_Obj_Type_t::PI)
            pCktPis.emplace_back(&obj);
        else if (obj.GetType() == Ckt_Obj_Type_t::PO)
            pCktPos.emplace_back(&obj);
    }
    GenerateInputDistribution();
}


Ckt_Ntk_t::~Ckt_Ntk_t(void)
{
    Abc_NtkDelete(pAbcNtk);
}


void Ckt_Ntk_t::PrintInfo(void) const
{
    cout << "---------------- Network information ----------------" << endl;
    cout << "Name\tType" << endl;
    for (auto & obj : cktObjs) {
        cout << obj.GetName() << "\t" << obj.GetType() << endl;
    }
}


void Ckt_Ntk_t::GenerateInputDistribution(unsigned seed)
{
    uniform_int <> distribution(0, 1);
    boost::random::mt19937 engine(seed);
    variate_generator<boost::random::mt19937, uniform_int <> > randomPI(engine, distribution);
    for (auto & pi : pCktPis) {
        assert(pi->GetClustersCap() == nValueClusters);
        for (int i = 0; i < nValueClusters; ++i) {
            uint64_t value = 0;
            for (int j = 0; j < 64; ++j) {
                if (randomPI())
                    SetBit(value, static_cast <uint64_t> (j));
            }
            pi->AddCluster(value);
        }
    }
}
