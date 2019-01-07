#include "cktGate.h"


using namespace std;
using namespace abc;


Ckt_Gate_t::Ckt_Gate_t(Abc_Obj_t * p_abc_obj, Ckt_Gate_Net_t * p_ckt_ntk)
    : pAbcObj(p_abc_obj), pCktNtk(p_ckt_ntk), type(Abc_GetGateType(p_abc_obj)), isVisited(false), isNewInv(false), topoId(0), pCktInv(nullptr)
{
    valueClusters.resize(pCktNtk->GetValClustersNum());
    valueClustersBak.resize(pCktNtk->GetValClustersNum());
    foConeInfo.resize((Abc_NtkPoNum(pCktNtk->GetAbcNtk()) >> 6) + 1);
    BD.resize(Abc_NtkPoNum(pCktNtk->GetAbcNtk()));
}


Ckt_Gate_t::Ckt_Gate_t(const Ckt_Gate_t & other)
    : pAbcObj(other.pAbcObj), pCktNtk(other.pCktNtk), type(other.GetType()), isVisited(other.isVisited), isNewInv(false), topoId(other.topoId), pCktInv(nullptr)
{
    // shallow copy
    valueClusters.resize(other.pCktNtk->GetValClustersNum());
    valueClustersBak.resize(other.pCktNtk->GetValClustersNum());
    foConeInfo.resize(other.foConeInfo.size());
    BD.resize(other.BD.size());
}


Ckt_Gate_t::~Ckt_Gate_t(void)
{
}


void Ckt_Gate_t::PrintFanios(void) const
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


void Ckt_Gate_t::PrintClusters(void) const
{
    for (auto & cluster : valueClusters) {
        for (int i = 0; i < 64; ++i) {
            cout << Ckt_GetBit(cluster, static_cast <uint64_t> (i));
        }
    }
}


void Ckt_Gate_t::UpdateClusters(void)
{
    switch ( type ) {
        case Ckt_Gate_Cat_t::PI:
        case Ckt_Gate_Cat_t::CONST0:
        case Ckt_Gate_Cat_t::CONST1:
        break;
        case Ckt_Gate_Cat_t::PO:
        case Ckt_Gate_Cat_t::BUF:
            for (int i = 0; i < static_cast <int> (valueClusters.size()); ++i)
                valueClusters[i] = pCktFanins[0]->valueClusters[i];
        break;
        case Ckt_Gate_Cat_t::INV:
            for (int i = 0; i < static_cast <int> (valueClusters.size()); ++i)
                valueClusters[i] = ~pCktFanins[0]->valueClusters[i];
        break;
        case Ckt_Gate_Cat_t::XOR:
            for (int i = 0; i < static_cast <int> (valueClusters.size()); ++i)
                valueClusters[i] = pCktFanins[0]->valueClusters[i] ^ pCktFanins[1]->valueClusters[i];
        break;
        case Ckt_Gate_Cat_t::XNOR:
            for (int i = 0; i < static_cast <int> (valueClusters.size()); ++i)
                valueClusters[i] = ~(pCktFanins[0]->valueClusters[i] ^ pCktFanins[1]->valueClusters[i]);
        break;
        case Ckt_Gate_Cat_t::AND2:
            for (int i = 0; i < static_cast <int> (valueClusters.size()); ++i)
                valueClusters[i] = pCktFanins[0]->valueClusters[i] & pCktFanins[1]->valueClusters[i];
        break;
        case Ckt_Gate_Cat_t::OR2:
            for (int i = 0; i < static_cast <int> (valueClusters.size()); ++i)
                valueClusters[i] = pCktFanins[0]->valueClusters[i] | pCktFanins[1]->valueClusters[i];
        break;
        case Ckt_Gate_Cat_t::NAND2:
            for (int i = 0; i < static_cast <int> (valueClusters.size()); ++i)
                valueClusters[i] = ~(pCktFanins[0]->valueClusters[i] & pCktFanins[1]->valueClusters[i]);
        break;
        case Ckt_Gate_Cat_t::NAND3:
            for (int i = 0; i < static_cast <int> (valueClusters.size()); ++i)
                valueClusters[i] = ~(pCktFanins[0]->valueClusters[i] & pCktFanins[1]->valueClusters[i] & pCktFanins[2]->valueClusters[i]);
        break;
        case Ckt_Gate_Cat_t::NAND4:
            for (int i = 0; i < static_cast <int> (valueClusters.size()); ++i)
                valueClusters[i] = ~(pCktFanins[0]->valueClusters[i] & pCktFanins[1]->valueClusters[i] & pCktFanins[2]->valueClusters[i] & pCktFanins[3]->valueClusters[i]);
        break;
        case Ckt_Gate_Cat_t::NOR2:
            for (int i = 0; i < static_cast <int> (valueClusters.size()); ++i)
                valueClusters[i] = ~(pCktFanins[0]->valueClusters[i] | pCktFanins[1]->valueClusters[i]);
        break;
        case Ckt_Gate_Cat_t::NOR3:
            for (int i = 0; i < static_cast <int> (valueClusters.size()); ++i)
                valueClusters[i] = ~(pCktFanins[0]->valueClusters[i] | pCktFanins[1]->valueClusters[i] | pCktFanins[2]->valueClusters[i]);
        break;
        case Ckt_Gate_Cat_t::NOR4:
            for (int i = 0; i < static_cast <int> (valueClusters.size()); ++i)
                valueClusters[i] = ~(pCktFanins[0]->valueClusters[i] | pCktFanins[1]->valueClusters[i] | pCktFanins[2]->valueClusters[i] | pCktFanins[3]->valueClusters[i]);
        break;
        case Ckt_Gate_Cat_t::AOI21:
            for (int i = 0; i < static_cast <int> (valueClusters.size()); ++i)
                valueClusters[i] = ~((pCktFanins[0]->valueClusters[i] & pCktFanins[1]->valueClusters[i]) | pCktFanins[2]->valueClusters[i]);
        break;
        case Ckt_Gate_Cat_t::AOI22:
            for (int i = 0; i < static_cast <int> (valueClusters.size()); ++i)
                valueClusters[i] = ~((pCktFanins[0]->valueClusters[i] & pCktFanins[1]->valueClusters[i]) | (pCktFanins[2]->valueClusters[i] & pCktFanins[3]->valueClusters[i]));
        break;
        case Ckt_Gate_Cat_t::OAI21:
            for (int i = 0; i < static_cast <int> (valueClusters.size()); ++i)
                valueClusters[i] = ~((pCktFanins[0]->valueClusters[i] | pCktFanins[1]->valueClusters[i]) & pCktFanins[2]->valueClusters[i]);
        break;
        case Ckt_Gate_Cat_t::OAI22:
            for (int i = 0; i < static_cast <int> (valueClusters.size()); ++i)
                valueClusters[i] = ~((pCktFanins[0]->valueClusters[i] | pCktFanins[1]->valueClusters[i]) & (pCktFanins[2]->valueClusters[i] | pCktFanins[3]->valueClusters[i]));
        break;
        default:
            assert(0);
    }
}


void Ckt_Gate_t::UpdateCluster(int i)
{
    switch ( type ) {
        case Ckt_Gate_Cat_t::PI:
        case Ckt_Gate_Cat_t::CONST0:
        case Ckt_Gate_Cat_t::CONST1:
        break;
        case Ckt_Gate_Cat_t::PO:
        case Ckt_Gate_Cat_t::BUF:
            valueClusters[i] = pCktFanins[0]->valueClusters[i];
        break;
        case Ckt_Gate_Cat_t::INV:
            valueClusters[i] = ~pCktFanins[0]->valueClusters[i];
        break;
        case Ckt_Gate_Cat_t::XOR:
            valueClusters[i] = pCktFanins[0]->valueClusters[i] ^ pCktFanins[1]->valueClusters[i];
        break;
        case Ckt_Gate_Cat_t::XNOR:
            valueClusters[i] = ~(pCktFanins[0]->valueClusters[i] ^ pCktFanins[1]->valueClusters[i]);
        break;
        case Ckt_Gate_Cat_t::AND2:
            valueClusters[i] = pCktFanins[0]->valueClusters[i] & pCktFanins[1]->valueClusters[i];
        break;
        case Ckt_Gate_Cat_t::OR2:
            valueClusters[i] = pCktFanins[0]->valueClusters[i] | pCktFanins[1]->valueClusters[i];
        break;
        case Ckt_Gate_Cat_t::NAND2:
            valueClusters[i] = ~(pCktFanins[0]->valueClusters[i] & pCktFanins[1]->valueClusters[i]);
        break;
        case Ckt_Gate_Cat_t::NAND3:
            valueClusters[i] = ~(pCktFanins[0]->valueClusters[i] & pCktFanins[1]->valueClusters[i] & pCktFanins[2]->valueClusters[i]);
        break;
        case Ckt_Gate_Cat_t::NAND4:
            valueClusters[i] = ~(pCktFanins[0]->valueClusters[i] & pCktFanins[1]->valueClusters[i] & pCktFanins[2]->valueClusters[i] & pCktFanins[3]->valueClusters[i]);
        break;
        case Ckt_Gate_Cat_t::NOR2:
            valueClusters[i] = ~(pCktFanins[0]->valueClusters[i] | pCktFanins[1]->valueClusters[i]);
        break;
        case Ckt_Gate_Cat_t::NOR3:
            valueClusters[i] = ~(pCktFanins[0]->valueClusters[i] | pCktFanins[1]->valueClusters[i] | pCktFanins[2]->valueClusters[i]);
        break;
        case Ckt_Gate_Cat_t::NOR4:
            valueClusters[i] = ~(pCktFanins[0]->valueClusters[i] | pCktFanins[1]->valueClusters[i] | pCktFanins[2]->valueClusters[i] | pCktFanins[3]->valueClusters[i]);
        break;
        case Ckt_Gate_Cat_t::AOI21:
            valueClusters[i] = ~((pCktFanins[0]->valueClusters[i] & pCktFanins[1]->valueClusters[i]) | pCktFanins[2]->valueClusters[i]);
        break;
        case Ckt_Gate_Cat_t::AOI22:
            valueClusters[i] = ~((pCktFanins[0]->valueClusters[i] & pCktFanins[1]->valueClusters[i]) | (pCktFanins[2]->valueClusters[i] & pCktFanins[3]->valueClusters[i]));
        break;
        case Ckt_Gate_Cat_t::OAI21:
            valueClusters[i] = ~((pCktFanins[0]->valueClusters[i] | pCktFanins[1]->valueClusters[i]) & pCktFanins[2]->valueClusters[i]);
        break;
        case Ckt_Gate_Cat_t::OAI22:
            valueClusters[i] = ~((pCktFanins[0]->valueClusters[i] | pCktFanins[1]->valueClusters[i]) & (pCktFanins[2]->valueClusters[i] | pCktFanins[3]->valueClusters[i]));
        break;
        default:
            assert(0);
    }
}


void Ckt_Gate_t::ReplaceBy(Ckt_Gate_t & cktNewObj, vector <Ckt_Rpl_Info_t> & info)
{
    info.clear();
    assert(!IsPO() && !IsPI() && !IsConst() && !cktNewObj.IsPO() && this != &cktNewObj);

    Abc_ObjTransferFanout(pAbcObj, cktNewObj.pAbcObj);

    // clone fanouts
    vector <Ckt_Gate_t *> pCktOldFanouts(pCktFanouts);
    // patch fanin
    for (auto & pCktOldFanout : pCktOldFanouts) {
        // replace fanin
        auto itCktFanin = find( pCktOldFanout->pCktFanins.begin(),
                                pCktOldFanout->pCktFanins.end(), this );
        assert(itCktFanin != pCktOldFanout->pCktFanins.end());
        *itCktFanin = &cktNewObj;

        // remove fanout
        auto itCktFanout = find( pCktFanouts.begin(),
                                 pCktFanouts.end(), pCktOldFanout );
        assert(itCktFanout != pCktFanouts.end());
        pCktFanouts.erase(itCktFanout);

        // add fanout
        cktNewObj.pCktFanouts.emplace_back(pCktOldFanout);

        // record history
        info.emplace_back(Ckt_Rpl_Info_t(this,          itCktFanin - pCktOldFanout->pCktFanins.begin(),
                                         pCktOldFanout, itCktFanout - pCktFanouts.begin()));
    }
}


void Ckt_Gate_t::CheckFanio(void) const
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


void Ckt_Gate_t::PrintBD(void) const
{
    cout << GetName() << ":" << endl;
    for (int i = 0; i < pCktNtk->GetPoNum(); ++i) {
        cout << pCktNtk->GetPo(i)->GetName() << ",";
        for (int j = 0; j < 64; ++j)
            cout << Ckt_GetBit(BD[i], static_cast <uint64_t> (j));
        cout << endl;
    }
}


Ckt_Rpl_Info_t::Ckt_Rpl_Info_t(Ckt_Gate_t * pObj1, int iFanin, Ckt_Gate_t * pObj2, int iFanout)
    : pCktObjFrom(pObj1), iCktFanin(iFanin), pCktObjTo(pObj2), iCktFanout(iFanout)
{
}


Ckt_Rpl_Info_t::~Ckt_Rpl_Info_t(void)
{
}


ostream & operator << (ostream & os, const Ckt_Gate_Cat_t & type)
{
    switch ( type ) {
        case Ckt_Gate_Cat_t::PI:
            cout << "PI";
        break;
        case Ckt_Gate_Cat_t::PO:
            cout << "PO";
        break;
        case Ckt_Gate_Cat_t::CONST0:
            cout << "CONST0";
        break;
        case Ckt_Gate_Cat_t::CONST1:
            cout << "CONST1";
        break;
        case Ckt_Gate_Cat_t::BUF:
            cout << "BUF";
        break;
        case Ckt_Gate_Cat_t::INV:
            cout << "INV";
        break;
        case Ckt_Gate_Cat_t::XOR:
            cout << "XOR2";
        break;
        case Ckt_Gate_Cat_t::XNOR:
            cout << "XNOR2";
        break;
        case Ckt_Gate_Cat_t::AND2:
            cout << "AND2";
        break;
        case Ckt_Gate_Cat_t::OR2:
            cout << "OR2";
        break;
        case Ckt_Gate_Cat_t::NAND2:
            cout << "NAND2";
        break;
        case Ckt_Gate_Cat_t::NAND3:
            cout << "NAND3";
        break;
        case Ckt_Gate_Cat_t::NAND4:
            cout << "NAND4";
        break;
        case Ckt_Gate_Cat_t::NOR2:
            cout << "NOR2";
        break;
        case Ckt_Gate_Cat_t::NOR3:
            cout << "NOR3";
        break;
        case Ckt_Gate_Cat_t::NOR4:
            cout << "NOR4";
        break;
        case Ckt_Gate_Cat_t::AOI21:
            cout << "AOI21";
        break;
        case Ckt_Gate_Cat_t::AOI22:
            cout << "AOI22";
        break;
        case Ckt_Gate_Cat_t::OAI21:
            cout << "OAI21";
        break;
        case Ckt_Gate_Cat_t::OAI22:
            cout << "OAI22";
        break;
        default:
            assert(0);
    }
    return os;
}


ostream & operator << (ostream & os, const Ckt_Rpl_Info_t & info)
{
    cout << info.pCktObjFrom->GetName() << "\t";
    cout << info.pCktObjTo->GetName() << "\t";
    cout << info.iCktFanin << "\t";
    cout << info.iCktFanout;
    return os;
}


Ckt_Gate_Cat_t Abc_GetGateType( Abc_Obj_t * pObj )
{
    if (Abc_ObjIsPi(pObj))
        return Ckt_Gate_Cat_t::PI;
    if (Abc_ObjIsPo(pObj))
        return Ckt_Gate_Cat_t::PO;
    assert(Abc_ObjIsNode(pObj));
    if ( Abc_GateIsConst0(pObj) )
        return Ckt_Gate_Cat_t::CONST0;
    else if ( Abc_GateIsConst1(pObj) )
        return Ckt_Gate_Cat_t::CONST1;
    else if ( Abc_GateIsBuf(pObj) )
        return Ckt_Gate_Cat_t::BUF;
    else if ( Abc_GateIsInv(pObj) )
        return Ckt_Gate_Cat_t::INV;
    else if ( Abc_GateIsXor(pObj) )
        return Ckt_Gate_Cat_t::XOR;
    else if ( Abc_GateIsXnor(pObj) )
        return Ckt_Gate_Cat_t::XNOR;
    else if ( Abc_GateIsAnd(pObj) ) {
        assert( Abc_ObjFaninNum(pObj) == 2 );
        return Ckt_Gate_Cat_t::AND2;
    }
    else if ( Abc_GateIsOr(pObj) ) {
        assert( Abc_ObjFaninNum(pObj) == 2 );
        return Ckt_Gate_Cat_t::OR2;
    }
    else if ( Abc_GateIsNand(pObj) ) {
        assert( Abc_ObjFaninNum(pObj) <= 4 );
        return (Ckt_Gate_Cat_t)( (int)Ckt_Gate_Cat_t::NAND2 + Abc_ObjFaninNum(pObj) - 2 );
    }
    else if ( Abc_GateIsNor(pObj) ) {
        assert( Abc_ObjFaninNum( pObj ) <= 4 );
        return (Ckt_Gate_Cat_t)( (int)Ckt_Gate_Cat_t::NOR2 + Abc_ObjFaninNum( pObj ) - 2 );
    }
    else if ( Abc_GateIsAOI21(pObj) )
        return Ckt_Gate_Cat_t::AOI21;
    else if ( Abc_GateIsAOI22(pObj) )
        return Ckt_Gate_Cat_t::AOI22;
    else if ( Abc_GateIsOAI21(pObj) )
        return Ckt_Gate_Cat_t::OAI21;
    else if ( Abc_GateIsOAI22(pObj) )
        return Ckt_Gate_Cat_t::OAI22;
    else
        assert( 0 );
}


bool Abc_GateIsConst0(  Abc_Obj_t * pObj )
{
    char * pSop  = Mio_GateReadSop( (Mio_Gate_t *)pObj->pData );
    return Abc_SopIsConst0(pSop);
}


bool Abc_GateIsConst1(  Abc_Obj_t * pObj )
{
    char * pSop  = Mio_GateReadSop( (Mio_Gate_t *)pObj->pData );
    return Abc_SopIsConst1(pSop);
}


bool Abc_GateIsBuf(  Abc_Obj_t * pObj )
{
    char * pSop  = Mio_GateReadSop( (Mio_Gate_t *)pObj->pData );
    return Abc_SopIsBuf(pSop);
}


bool Abc_GateIsInv(  Abc_Obj_t * pObj )
{
    char * pSop  = Mio_GateReadSop( (Mio_Gate_t *)pObj->pData );
    return Abc_SopIsInv(pSop);
}


bool Abc_GateIsAnd(  Abc_Obj_t * pObj )
{
    char * pSop  = Mio_GateReadSop( (Mio_Gate_t *)pObj->pData );
    if ( !Abc_SopIsComplement(pSop) ) {    //111 1
        char * pCur;
        if ( Abc_SopGetCubeNum(pSop) != 1 )
            return 0;
        for ( pCur = pSop; *pCur != ' '; pCur++ )
            if ( *pCur != '1' )
                return 0;
    }
    else {    //0-- 0\n-0- 0\n--0 0\n
        char * pCube, * pCur;
        int nVars, nLits;
        nVars = Abc_SopGetVarNum( pSop );
        if ( nVars != Abc_SopGetCubeNum(pSop) )
            return 0;
        Abc_SopForEachCube( pSop, nVars, pCube )
        {
            nLits = 0;
            for ( pCur = pCube; *pCur != ' '; pCur++ ) {
                if ( *pCur ==  '0' )
                    nLits ++;
                else if ( *pCur == '-' )
                    continue;
                else
                    return 0;
            }
            if ( nLits != 1 )
                return 0;
        }
    }
    return 1;
}


bool Abc_GateIsOr(  Abc_Obj_t * pObj )
{
    char * pSop  = Mio_GateReadSop( (Mio_Gate_t *)pObj->pData );
    if ( Abc_SopIsComplement(pSop) ) {    //000 0
        char * pCur;
        if ( Abc_SopGetCubeNum(pSop) != 1 )
            return 0;
        for ( pCur = pSop; *pCur != ' '; pCur++ )
            if ( *pCur != '0' )
                return 0;
    }
    else {    //1-- 1\n-1- 1\n--1 1\n
        char * pCube, * pCur;
        int nVars, nLits;
        nVars = Abc_SopGetVarNum( pSop );
        if ( nVars != Abc_SopGetCubeNum(pSop) )
            return 0;
        Abc_SopForEachCube( pSop, nVars, pCube )
        {
            nLits = 0;
            for ( pCur = pCube; *pCur != ' '; pCur++ ) {
                if ( *pCur ==  '1' )
                    nLits ++;
                else if ( *pCur == '-' )
                    continue;
                else
                    return 0;
            }
            if ( nLits != 1 )
                return 0;
        }
    }
    return 1;
}


bool Abc_GateIsNand(  Abc_Obj_t * pObj )
{
    char * pSop  = Mio_GateReadSop( (Mio_Gate_t *)pObj->pData );
    if ( Abc_SopIsComplement(pSop) ) {    //111 0
        char * pCur;
        if ( Abc_SopGetCubeNum(pSop) != 1 )
            return 0;
        for ( pCur = pSop; *pCur != ' '; pCur++ )
            if ( *pCur != '1' )
                return 0;
    }
    else {    //0-- 1\n-0- 1\n--0 1\n
        char * pCube, * pCur;
        int nVars, nLits;
        nVars = Abc_SopGetVarNum( pSop );
        if ( nVars != Abc_SopGetCubeNum(pSop) )
            return 0;
        Abc_SopForEachCube( pSop, nVars, pCube )
        {
            nLits = 0;
            for ( pCur = pCube; *pCur != ' '; pCur++ ) {
                if ( *pCur ==  '0' )
                    nLits ++;
                else if ( *pCur == '-' )
                    continue;
                else
                    return 0;
            }
            if ( nLits != 1 )
                return 0;
        }
    }
    return 1;
}


bool Abc_GateIsNor(  Abc_Obj_t * pObj )
{
    char * pSop  = Mio_GateReadSop( (Mio_Gate_t *)pObj->pData );
    if ( !Abc_SopIsComplement(pSop) ) {    //000 1
        char * pCur;
        if ( Abc_SopGetCubeNum(pSop) != 1 )
            return 0;
        for ( pCur = pSop; *pCur != ' '; pCur++ )
            if ( *pCur != '0' )
                return 0;
    }
    else {    //1-- 0\n-1- 0\n--1 0\n
        char * pCube, * pCur;
        int nVars, nLits;
        nVars = Abc_SopGetVarNum( pSop );
        if ( nVars != Abc_SopGetCubeNum(pSop) )
            return 0;
        Abc_SopForEachCube( pSop, nVars, pCube )
        {
            nLits = 0;
            for ( pCur = pCube; *pCur != ' '; pCur++ ) {
                if ( *pCur ==  '1' )
                    nLits ++;
                else if ( *pCur == '-' )
                    continue;
                else
                    return 0;
            }
            if ( nLits != 1 )
                return 0;
        }
    }
    return 1;
}


bool Abc_GateIsXor(  Abc_Obj_t * pObj )
{
    char * pSop  = Mio_GateReadSop( (Mio_Gate_t *)pObj->pData );
    if ( !Abc_SopIsComplement(pSop) ) {    //01 1\n10 1\n
        char * pCube, * pCur;
        int nVars, nLits;
        nVars = Abc_SopGetVarNum( pSop );
        if ( nVars != 2 )
            return 0;
        Abc_SopForEachCube( pSop, nVars, pCube )
        {
            nLits = 0;
            for ( pCur = pCube; *pCur != ' '; pCur++ ) {
                if ( *pCur ==  '1' )
                    nLits ++;
                else if ( *pCur == '-' )
                    return 0;
            }
            if ( nLits != 1 )
                return 0;
        }
    }
    else
        return 0;
    return 1;
}


bool Abc_GateIsXnor(  Abc_Obj_t * pObj )
{
    char * pSop  = Mio_GateReadSop( (Mio_Gate_t *)pObj->pData );
    if ( strcmp( pSop, "00 1\n11 1\n" ) == 0 )
        return 1;
    else if ( strcmp( pSop, "11 1\n00 1\n" ) == 0 )
        return 1;
    else
        return 0;
}


bool Abc_GateIsAOI21(  Abc_Obj_t * pObj )
{
    char * pSop  = Mio_GateReadSop( (Mio_Gate_t *)pObj->pData );
    if ( strcmp( pSop, "-00 1\n0-0 1\n" ) == 0 )
        return 1;
    else
        return 0;
}


bool Abc_GateIsAOI22(  Abc_Obj_t * pObj )
{
    char * pSop  = Mio_GateReadSop( (Mio_Gate_t *)pObj->pData );
    if ( strcmp( pSop, "--11 0\n11-- 0\n" ) == 0 )
        return 1;
    else
        return 0;
}


bool Abc_GateIsOAI21(  Abc_Obj_t * pObj )
{
    char * pSop  = Mio_GateReadSop( (Mio_Gate_t *)pObj->pData );
    if ( strcmp( pSop, "--0 1\n00- 1\n" ) == 0 )
        return 1;
    else
        return 0;
}


bool Abc_GateIsOAI22(  Abc_Obj_t * pObj )
{
    char * pSop  = Mio_GateReadSop( (Mio_Gate_t *)pObj->pData );
    if ( strcmp( pSop, "--00 1\n00-- 1\n" ) == 0 )
        return 1;
    else
        return 0;
}
