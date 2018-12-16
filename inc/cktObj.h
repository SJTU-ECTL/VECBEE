#ifndef CKT_OBJ_H
#define CKT_OBJ_H


#include <bits/stdc++.h>
#include "abcApi.h"


enum class Ckt_Obj_Type_t
{
    PI,   PO,   CONST0, CONST1, BUF,   INV,   XOR,
    XNOR, AND2, OR2,    NAND2,  NAND3, NAND4, NOR2,
    NOR3, NOR4, AOI21,  AOI22,  OAI21, OAI22
};


class Ckt_Obj_t
{
private:
    abc::Abc_Obj_t *       pAbcObj;       // the corresponding ABC object
    Ckt_Obj_Type_t         type;          // object type
    std::vector <uint64_t> valueClusters; // temporary simluation value clusters

    Ckt_Obj_t & operator = (const Ckt_Obj_t & other);

public:
    explicit Ckt_Obj_t(abc::Abc_Obj_t * p_abc_obj);
    Ckt_Obj_t(const Ckt_Obj_t & other);
    ~Ckt_Obj_t(void);
    inline Ckt_Obj_Type_t GetObjType(void) const { return type; }
    inline float GetArrivalTime(void) const { return ((abc::Abc_Time_t *)pAbcObj->pNtk->pManTime->vArrs->pArray[pAbcObj->Id])->Rise; }
    inline std::string GetName(void) const { return std::string(Abc_ObjName(pAbcObj)); }
};


std::ostream & operator << (std::ostream & os, const Ckt_Obj_Type_t & type);
Ckt_Obj_Type_t Ckt_GetObjType(abc::Abc_Obj_t * pAbcObj);
bool Ckt_SopIsConst0( char * pSop );
bool Ckt_SopIsConst1( char * pSop );
bool Ckt_SopIsBuf( char * pSop );
bool Ckt_SopIsInvGate( char * pSop );
bool Ckt_SopIsAndGate( char * pSop );
bool Ckt_SopIsOrGate( char * pSop );
bool Ckt_SopIsNandGate( char * pSop );
bool Ckt_SopIsNorGate( char * pSop );
bool Ckt_SopIsXorGate( char * pSop );
bool Ckt_SopIsXnorGate( char * pSop );
bool Ckt_SopIsAOI21Gate( char * pSop );
bool Ckt_SopIsAOI22Gate( char * pSop );
bool Ckt_SopIsOAI21Gate( char * pSop );
bool Ckt_SopIsOAI22Gate( char * pSop );


#endif
