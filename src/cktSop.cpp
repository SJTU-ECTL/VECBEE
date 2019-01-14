#include "cktSop.h"


using namespace std;
using namespace abc;


Ckt_Sop_t::Ckt_Sop_t(Abc_Obj_t * p_abc_obj, Ckt_Sop_Net_t * p_ckt_ntk)
    : pAbcObj(p_abc_obj), pCktNtk(p_ckt_ntk), type(Abc_GetSopType(p_abc_obj)), isVisited(false), topoId(0)
{
    valueClusters.resize(pCktNtk->GetValClustersNum());
    valueClustersBak.resize(pCktNtk->GetValClustersNum());
    CollectSOP();
    GetLiteralsNum();
    GenerateALCs();
    foConeInfo.resize((Abc_NtkPoNum(pCktNtk->GetAbcNtk()) >> 6) + 1);
    BD.resize(Abc_NtkPoNum(pCktNtk->GetAbcNtk()));
}


Ckt_Sop_t::Ckt_Sop_t(const Ckt_Sop_t & other)
    : pAbcObj(other.pAbcObj), pCktNtk(other.pCktNtk), type(other.GetType()), isVisited(other.isVisited), topoId(other.topoId)
{
    valueClusters.resize(other.pCktNtk->GetValClustersNum());
    valueClustersBak.resize(other.pCktNtk->GetValClustersNum());
    SOP.assign(other.SOP.begin(), other.SOP.end());
    nLiterals = other.nLiterals;
    ALCs.assign(other.ALCs.begin(), other.ALCs.end());
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


void Ckt_Sop_t::PrintSOP(void) const
{
    for (auto & s : SOP)
        cout << s << " ";
}


void Ckt_Sop_t::PrintSOP(vector <string> & strs) const
{
    for (auto & s : strs)
        cout << s << " ";
}


void Ckt_Sop_t::CollectSOP(void)
{
    if (IsPI() || IsPO() || IsConst())
        return;
    char * pCube, * pSop = (char *)pAbcObj->pData;
    int Value, v;
    assert(pSop && !Abc_SopIsExorType(pSop));
    int nVars = Abc_SopGetVarNum(pSop);
    SOP.clear();
    Abc_SopForEachCube(pSop, nVars, pCube) {
        string s = "";
        Abc_CubeForEachVar(pCube, Value, v)
            if (Value == '0' || Value == '1' || Value == '-')
                s += static_cast<char>(Value);
            else
                continue;
        SOP.emplace_back(s);
    }
}


void Ckt_Sop_t::GenerateALCs(int maxNLiterals)
{
    ALCs.clear();
    GenerateALCsRecur(0, 0, 0, maxNLiterals);
}


void Ckt_Sop_t::GenerateALCsRecur(int i, int j, int n, int maxNLiterals)
{
    cout << i << "\t" << j << "\t" << n << endl;
    PrintSOP();
    cout << endl;
    if (n > maxNLiterals)
        return;
    else if (i >= static_cast<int>(SOP.size())) {
        vector <string> ALC(SOP);
        for (auto it = ALC.begin(); it != ALC.end(); ++it) {
            bool isAllDC = true;
            for (auto & ch : *it) {
                if (ch != '-') {
                    isAllDC = false;
                    break;
                }
            }
            if (isAllDC) {
                ALC.erase(it);
                --it;
            }
        }
        if (!ALC.empty() && n)
            ALCs.emplace_back(ALC);
        return;
    }
    else if (j >= static_cast<int>(SOP[i].size()))
        GenerateALCsRecur(i + 1, 0, n);
    else {
        // do not change
        GenerateALCsRecur(i, j + 1, n);
        // change
        if (SOP[i][j] != '-') {
            char bak = SOP[i][j];
            SOP[i][j] = '-';
            GenerateALCsRecur(i, j + 1, n + 1);
            SOP[i][j] = bak;
        }
    }
}


void Ckt_Sop_t::PrintALCs(void) const
{
    for (auto ALC : ALCs) {
        PrintSOP(ALC);
        cout << ";";
    }
}


int Ckt_Sop_t::GetLiteralsNum(void)
{
    int ret = 0;
    for (auto & s : SOP)
        for (auto & ch : s)
            if (ch != '-')
                ++ret;
    nLiterals = ret;
    return ret;
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
    switch (type) {
        case Ckt_Sop_Cat_t::PI:
        case Ckt_Sop_Cat_t::CONST0:
        case Ckt_Sop_Cat_t::CONST1:
        break;
        case Ckt_Sop_Cat_t::PO:
            for (int i = 0; i < static_cast <int> (valueClusters.size()); ++i)
                valueClusters[i] = pCktFanins[0]->valueClusters[i];
        break;
        case Ckt_Sop_Cat_t::INTER:
            for (int i = 0; i < static_cast <int> (valueClusters.size()); ++i) {
                valueClusters[i] = 0;
                for (auto & cube : SOP) {
                    uint64_t product = static_cast <uint64_t> (ULLONG_MAX);
                    for (int j = 0; j < static_cast <int> (cube.length()); ++j) {
                        if (cube[j] == '0')
                            product &= ~(pCktFanins[j]->valueClusters[i]);
                        else if (cube[j] == '1')
                            product &= pCktFanins[j]->valueClusters[i];
                    }
                    valueClusters[i] |= product;
                }
            }
        break;
    }
}


void Ckt_Sop_t::UpdateCluster(int i)
{
    switch (type) {
        case Ckt_Sop_Cat_t::PI:
        case Ckt_Sop_Cat_t::CONST0:
        case Ckt_Sop_Cat_t::CONST1:
        break;
        case Ckt_Sop_Cat_t::PO:
            valueClusters[i] = pCktFanins[0]->valueClusters[i];
        break;
        case Ckt_Sop_Cat_t::INTER:
            valueClusters[i] = 0;
            for (auto & cube : SOP) {
                uint64_t product = static_cast <uint64_t> (ULLONG_MAX);
                for (int j = 0; j < static_cast <int> (cube.length()); ++j) {
                    if (cube[j] == '0')
                        product &= ~(pCktFanins[j]->valueClusters[i]);
                    else if (cube[j] == '1')
                        product &= pCktFanins[j]->valueClusters[i];
                }
                valueClusters[i] |= product;
            }
        break;
    }
}


void Ckt_Sop_t::CheckFanio(void) const
{
    Abc_Obj_t * pObj;
    int i;
    assert(Abc_ObjFaninNum(pAbcObj) == static_cast <int> (pCktFanins.size()));
    assert(Abc_ObjFanoutNum(pAbcObj) == static_cast <int> (pCktFanouts.size()));
    Abc_ObjForEachFanout(pAbcObj, pObj, i)
        assert(static_cast <string> (Abc_ObjName(pObj)) == pCktFanouts[i]->GetName());
    Abc_ObjForEachFanin(pAbcObj, pObj, i)
        assert(static_cast <string> (Abc_ObjName(pObj)) == pCktFanins[i]->GetName());
}


void Ckt_Sop_t::PrintBD(void) const
{
    cout << GetName() << ":" << endl;
    for (int i = 0; i < pCktNtk->GetPoNum(); ++i) {
        cout << pCktNtk->GetPo(i)->GetName() << ",";
        for (int j = 0; j < 64; ++j)
            cout << Ckt_GetBit(BD[i], static_cast <uint64_t> (j));
        cout << endl;
    }
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
        break;
        default:
            assert(0);
    }
    return os;
}


Ckt_Sop_Cat_t Abc_GetSopType( Abc_Obj_t * pObj )
{
    if (Abc_ObjIsPi(pObj))
        return Ckt_Sop_Cat_t::PI;
    if (Abc_ObjIsPo(pObj))
        return Ckt_Sop_Cat_t::PO;
    assert(Abc_ObjIsNode(pObj));
    if ( Abc_NodeIsConst0(pObj) )
        return Ckt_Sop_Cat_t::CONST0;
    else if ( Abc_NodeIsConst1(pObj) )
        return Ckt_Sop_Cat_t::CONST1;
    else
        return Ckt_Sop_Cat_t::INTER;
}
