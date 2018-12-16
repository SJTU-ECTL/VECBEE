#include "cktNtk.h"


using namespace std;
using namespace abc;
using namespace boost;


Ckt_Ntk_t::Ckt_Ntk_t(Abc_Ntk_t * p_abc_ntk)
{
    pAbcNtk = Abc_NtkDup(p_abc_ntk);
    Abc_Obj_t * pAbcObj;
    int i;
    Abc_NtkForEachObj(p_abc_ntk, pAbcObj, i) {
        cktObjs.emplace_back(Ckt_Obj_t(pAbcObj));
        if (Abc_ObjIsPi(pAbcObj))
            cktPis.emplace_back(Ckt_Obj_t(pAbcObj));
        else if (Abc_ObjIsPo(pAbcObj))
            cktPos.emplace_back(Ckt_Obj_t(pAbcObj));
        else if (!Abc_ObjIsNode(pAbcObj))
            assert(0);
    }
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
        cout << obj.GetName() << "\t" << obj.GetObjType() << endl;
    }
}


void Ckt_Ntk_t::GenerateInputDistribution(void)
{
    uniform_int <> distribution(0, 1);
    boost::random::mt19937 engine(314);
    variate_generator<boost::random::mt19937, uniform_int <> > myRandom(engine, distribution);
    for (int i = 0; i < 100; ++i)
        cout << myRandom() << endl;
}
