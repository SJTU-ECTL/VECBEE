#include "cktSop.h"


using namespace std;
using namespace abc;


Ckt_Sop_t::Ckt_Sop_t(Abc_Obj_t * p_abc_obj, Ckt_Sop_Net_t * p_ckt_ntk)
    : pAbcObj(p_abc_obj), pCktNtk(p_ckt_ntk), isVisited(false), topoId(0)
{
    valueClusters.resize(pCktNtk->GetValClustersNum());
    foConeInfo.resize((Abc_NtkPoNum(pCktNtk->GetAbcNtk()) >> 6) + 1);
    BD.resize(Abc_NtkPoNum(pCktNtk->GetAbcNtk()));
}


Ckt_Sop_t::Ckt_Sop_t(const Ckt_Sop_t & other)
    : pAbcObj(other.pAbcObj), pCktNtk(other.pCktNtk), isVisited(other.isVisited), topoId(other.topoId)
{
    // shallow copy
    valueClusters.resize(other.pCktNtk->GetValClustersNum());
    foConeInfo.resize(other.foConeInfo.size());
    BD.resize(other.BD.size());
}


Ckt_Sop_t::~Ckt_Sop_t(void)
{
}


void Ckt_Sop_t::PrintFanios(void) const
{
    string temp = "";
    for (auto & pCktFanin : pCktFanins) {
        temp += pCktFanin->GetName();
        temp += ", ";
    }
    cout << setw(30) << setiosflags(ios::left) << temp;
    temp = "";
    for (auto & pCktFanout : pCktFanouts) {
        temp += pCktFanout->GetName();
        temp += ", ";
    }
    cout << setw(30) << setiosflags(ios::left) << temp;
}


void Ckt_Sop_t::PrintClusters(void) const
{
    for (auto & cluster : valueClusters) {
        for (int i = 0; i < 64; ++i) {
            cout << Ckt_GetBit(cluster, static_cast <uint64_t> (i));
        }
    }
}


void Ckt_Sop_t::UpdateClusters(void)
{
}


ostream & operator << (ostream & os, const Ckt_Sop_Cat_t & type)
{
    switch ( type ) {
        case Ckt_Sop_Cat_t::PI:
            cout << "PI";
        break;
        case Ckt_Sop_Cat_t::PO:
            cout << "PO";
        break;
        case Ckt_Sop_Cat_t::CONST0:
            cout << "CONST0";
        break;
        case Ckt_Sop_Cat_t::CONST1:
            cout << "CONST1";
        break;
        case Ckt_Sop_Cat_t::INTER:
            cout << "INTER";
        default:
            assert(0);
    }
    return os;
}

