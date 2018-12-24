#include "cktObj.h"


using namespace std;
using namespace abc;


Ckt_Obj_t::Ckt_Obj_t(Abc_Obj_t * p_abc_obj)
    : pAbcObj(p_abc_obj), type(Ckt_GetObjType(p_abc_obj)), isVisited(false)
{
}


Ckt_Obj_t::Ckt_Obj_t(const Ckt_Obj_t & other)
    : pAbcObj(other.pAbcObj), type(other.GetType()), isVisited(other.isVisited)
{
    // shallow copy
}


Ckt_Obj_t::~Ckt_Obj_t(void)
{
}


void Ckt_Obj_t::PrintFanios(void) const
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


void Ckt_Obj_t::PrintClusters(void) const
{
    for (auto & cluster : valueClusters) {
        for (int i = 0; i < 64; ++i) {
            cout << Ckt_GetBit(cluster, static_cast <uint64_t> (i));
        }
    }
}


void Ckt_Obj_t::UpdateClusters(void)
{
    switch ( type ) {
        case Ckt_Obj_Type_t::PI:
        case Ckt_Obj_Type_t::CONST0:
        case Ckt_Obj_Type_t::CONST1:
        break;
        case Ckt_Obj_Type_t::PO:
        case Ckt_Obj_Type_t::BUF:
            for (int i = 0; i < static_cast <int> (valueClusters.size()); ++i)
                valueClusters[i] = pCktFanins[0]->valueClusters[i];
        break;
        case Ckt_Obj_Type_t::INV:
            for (int i = 0; i < static_cast <int> (valueClusters.size()); ++i)
                valueClusters[i] = ~pCktFanins[0]->valueClusters[i];
        break;
        case Ckt_Obj_Type_t::XOR:
            for (int i = 0; i < static_cast <int> (valueClusters.size()); ++i)
                valueClusters[i] = pCktFanins[0]->valueClusters[i] ^ pCktFanins[1]->valueClusters[i];
        break;
        case Ckt_Obj_Type_t::XNOR:
            for (int i = 0; i < static_cast <int> (valueClusters.size()); ++i)
                valueClusters[i] = ~(pCktFanins[0]->valueClusters[i] ^ pCktFanins[1]->valueClusters[i]);
        break;
        case Ckt_Obj_Type_t::AND2:
            for (int i = 0; i < static_cast <int> (valueClusters.size()); ++i)
                valueClusters[i] = pCktFanins[0]->valueClusters[i] & pCktFanins[1]->valueClusters[i];
        break;
        case Ckt_Obj_Type_t::OR2:
            for (int i = 0; i < static_cast <int> (valueClusters.size()); ++i)
                valueClusters[i] = pCktFanins[0]->valueClusters[i] | pCktFanins[1]->valueClusters[i];
        break;
        case Ckt_Obj_Type_t::NAND2:
            for (int i = 0; i < static_cast <int> (valueClusters.size()); ++i)
                valueClusters[i] = ~(pCktFanins[0]->valueClusters[i] & pCktFanins[1]->valueClusters[i]);
        break;
        case Ckt_Obj_Type_t::NAND3:
            for (int i = 0; i < static_cast <int> (valueClusters.size()); ++i)
                valueClusters[i] = ~(pCktFanins[0]->valueClusters[i] & pCktFanins[1]->valueClusters[i] & pCktFanins[2]->valueClusters[i]);
        break;
        case Ckt_Obj_Type_t::NAND4:
            for (int i = 0; i < static_cast <int> (valueClusters.size()); ++i)
                valueClusters[i] = ~(pCktFanins[0]->valueClusters[i] & pCktFanins[1]->valueClusters[i] & pCktFanins[2]->valueClusters[i] & pCktFanins[3]->valueClusters[i]);
        break;
        case Ckt_Obj_Type_t::NOR2:
            for (int i = 0; i < static_cast <int> (valueClusters.size()); ++i)
                valueClusters[i] = ~(pCktFanins[0]->valueClusters[i] | pCktFanins[1]->valueClusters[i]);
        break;
        case Ckt_Obj_Type_t::NOR3:
            for (int i = 0; i < static_cast <int> (valueClusters.size()); ++i)
                valueClusters[i] = ~(pCktFanins[0]->valueClusters[i] | pCktFanins[1]->valueClusters[i] | pCktFanins[2]->valueClusters[i]);
        break;
        case Ckt_Obj_Type_t::NOR4:
            for (int i = 0; i < static_cast <int> (valueClusters.size()); ++i)
                valueClusters[i] = ~(pCktFanins[0]->valueClusters[i] | pCktFanins[1]->valueClusters[i] | pCktFanins[2]->valueClusters[i] | pCktFanins[3]->valueClusters[i]);
        break;
        case Ckt_Obj_Type_t::AOI21:
            for (int i = 0; i < static_cast <int> (valueClusters.size()); ++i)
                valueClusters[i] = ~((pCktFanins[0]->valueClusters[i] & pCktFanins[1]->valueClusters[i]) | pCktFanins[2]->valueClusters[i]);
        break;
        case Ckt_Obj_Type_t::AOI22:
            for (int i = 0; i < static_cast <int> (valueClusters.size()); ++i)
                valueClusters[i] = ~((pCktFanins[0]->valueClusters[i] & pCktFanins[1]->valueClusters[i]) | (pCktFanins[2]->valueClusters[i] & pCktFanins[3]->valueClusters[i]));
        break;
        case Ckt_Obj_Type_t::OAI21:
            for (int i = 0; i < static_cast <int> (valueClusters.size()); ++i)
                valueClusters[i] = ~((pCktFanins[0]->valueClusters[i] | pCktFanins[1]->valueClusters[i]) & pCktFanins[2]->valueClusters[i]);
        break;
        case Ckt_Obj_Type_t::OAI22:
            for (int i = 0; i < static_cast <int> (valueClusters.size()); ++i)
                valueClusters[i] = ~((pCktFanins[0]->valueClusters[i] | pCktFanins[1]->valueClusters[i]) & (pCktFanins[2]->valueClusters[i] | pCktFanins[3]->valueClusters[i]));
        break;
        default:
            assert(0);
    }
}


ostream & operator << (ostream & os, const Ckt_Obj_Type_t & type)
{
    switch ( type ) {
        case Ckt_Obj_Type_t::PI:
            cout << "PI";
        break;
        case Ckt_Obj_Type_t::PO:
            cout << "PO";
        break;
        case Ckt_Obj_Type_t::CONST0:
            cout << "CONST0";
        break;
        case Ckt_Obj_Type_t::CONST1:
            cout << "CONST1";
        break;
        case Ckt_Obj_Type_t::BUF:
            cout << "BUF";
        break;
        case Ckt_Obj_Type_t::INV:
            cout << "INV";
        break;
        case Ckt_Obj_Type_t::XOR:
            cout << "XOR2";
        break;
        case Ckt_Obj_Type_t::XNOR:
            cout << "XNOR2";
        break;
        case Ckt_Obj_Type_t::AND2:
            cout << "AND2";
        break;
        case Ckt_Obj_Type_t::OR2:
            cout << "OR2";
        break;
        case Ckt_Obj_Type_t::NAND2:
            cout << "NAND2";
        break;
        case Ckt_Obj_Type_t::NAND3:
            cout << "NAND3";
        break;
        case Ckt_Obj_Type_t::NAND4:
            cout << "NAND4";
        break;
        case Ckt_Obj_Type_t::NOR2:
            cout << "NOR2";
        break;
        case Ckt_Obj_Type_t::NOR3:
            cout << "NOR3";
        break;
        case Ckt_Obj_Type_t::NOR4:
            cout << "NOR4";
        break;
        case Ckt_Obj_Type_t::AOI21:
            cout << "AOI21";
        break;
        case Ckt_Obj_Type_t::AOI22:
            cout << "AOI22";
        break;
        case Ckt_Obj_Type_t::OAI21:
            cout << "OAI21";
        break;
        case Ckt_Obj_Type_t::OAI22:
            cout << "OAI22";
        break;
        default:
            assert(0);
    }
    return os;
}


Ckt_Obj_Type_t Ckt_GetObjType( Abc_Obj_t * pObj )
{
    if (Abc_ObjIsPi(pObj))
        return Ckt_Obj_Type_t::PI;
    if (Abc_ObjIsPo(pObj))
        return Ckt_Obj_Type_t::PO;
    assert(Abc_ObjIsNode(pObj));
    char * pSop  = Mio_GateReadSop( (Mio_Gate_t *)pObj->pData );
    if ( Ckt_SopIsConst0(pSop) )
        return Ckt_Obj_Type_t::CONST0;
    else if ( Ckt_SopIsConst1(pSop) )
        return Ckt_Obj_Type_t::CONST1;
    else if ( Ckt_SopIsBuf(pSop) )
        return Ckt_Obj_Type_t::BUF;
    else if ( Ckt_SopIsInvGate(pSop) )
        return Ckt_Obj_Type_t::INV;
    else if ( Ckt_SopIsXorGate(pSop) )
        return Ckt_Obj_Type_t::XOR;
    else if ( Ckt_SopIsXnorGate(pSop) )
        return Ckt_Obj_Type_t::XNOR;
    else if ( Ckt_SopIsAndGate(pSop) ) {
        assert( Abc_SopGetVarNum( pSop ) == 2 );
        return Ckt_Obj_Type_t::AND2;
    }
    else if ( Ckt_SopIsOrGate(pSop) ) {
        assert( Abc_SopGetVarNum( pSop ) == 2 );
        return Ckt_Obj_Type_t::OR2;
    }
    else if ( Ckt_SopIsNandGate(pSop) ) {
        assert( Abc_SopGetVarNum( pSop ) <= 4 );
        return (Ckt_Obj_Type_t)( (int)Ckt_Obj_Type_t::NAND2 + Abc_SopGetVarNum( pSop ) - 2 );
    }
    else if ( Ckt_SopIsNorGate(pSop) ) {
        assert( Abc_SopGetVarNum( pSop ) <= 4 );
        return (Ckt_Obj_Type_t)( (int)Ckt_Obj_Type_t::NOR2 + Abc_SopGetVarNum( pSop ) - 2 );
    }
    else if ( Ckt_SopIsAOI21Gate(pSop) )
        return Ckt_Obj_Type_t::AOI21;
    else if ( Ckt_SopIsAOI22Gate(pSop) )
        return Ckt_Obj_Type_t::AOI22;
    else if ( Ckt_SopIsOAI21Gate(pSop) )
        return Ckt_Obj_Type_t::OAI21;
    else if ( Ckt_SopIsOAI22Gate(pSop) )
        return Ckt_Obj_Type_t::OAI22;
    else
        assert( 0 );
}


bool Ckt_SopIsConst0( char * pSop )
{
    return Abc_SopIsConst0(pSop);
}


bool Ckt_SopIsConst1( char * pSop )
{
    return Abc_SopIsConst1(pSop);
}


bool Ckt_SopIsBuf( char * pSop )
{
    return Abc_SopIsBuf(pSop);
}


bool Ckt_SopIsInvGate( char * pSop )
{
    return Abc_SopIsInv(pSop);
}


bool Ckt_SopIsAndGate( char * pSop )
{
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


bool Ckt_SopIsOrGate( char * pSop )
{
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


bool Ckt_SopIsNandGate( char * pSop )
{
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


bool Ckt_SopIsNorGate( char * pSop )
{
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


bool Ckt_SopIsXorGate( char * pSop )
{
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


bool Ckt_SopIsXnorGate( char * pSop )
{
    if ( strcmp( pSop, "00 1\n11 1\n" ) == 0 )
        return 1;
    else if ( strcmp( pSop, "11 1\n00 1\n" ) == 0 )
        return 1;
    else
        return 0;
}


bool Ckt_SopIsAOI21Gate( char * pSop )
{
    if ( strcmp( pSop, "-00 1\n0-0 1\n" ) == 0 )
        return 1;
    else
        return 0;
}


bool Ckt_SopIsAOI22Gate( char * pSop )
{
    if ( strcmp( pSop, "--11 0\n11-- 0\n" ) == 0 )
        return 1;
    else
        return 0;
}


bool Ckt_SopIsOAI21Gate( char * pSop )
{
    if ( strcmp( pSop, "--0 1\n00- 1\n" ) == 0 )
        return 1;
    else
        return 0;
}


bool Ckt_SopIsOAI22Gate( char * pSop )
{
    if ( strcmp( pSop, "--00 1\n00-- 1\n" ) == 0 )
        return 1;
    else
        return 0;
}