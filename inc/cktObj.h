#ifndef CKT_OBJ_H
#define CKT_OBJ_H


#include <bits/stdc++.h>
#include "abcApi.h"
#include "cktNtk.h"


class Ckt_Rpl_Info_t;
class Ckt_Ntk_t;


static inline void            Ckt_SetBit        (uint64_t & x, uint64_t f) { x |= ((uint64_t)1 << (f & (uint64_t)63)); }
static inline void            Ckt_ResetBit      (uint64_t & x, uint64_t f) { x &= ~((uint64_t)1 << (f & (uint64_t)63)); }
static inline bool            Ckt_GetBit        (uint64_t x, uint64_t f) { return (bool)((x >> f) & (uint64_t)1); }


enum class Ckt_Obj_Type_t
{
    PI,   PO,   CONST0, CONST1, BUF,   INV,   XOR,
    XNOR, AND2, OR2,    NAND2,  NAND3, NAND4, NOR2,
    NOR3, NOR4, AOI21,  AOI22,  OAI21, OAI22
};


class Ckt_Obj_t
{
private:
    abc::Abc_Obj_t *          pAbcObj;          // the corresponding ABC object
    Ckt_Ntk_t *               pCktNtk;          // the corresponding CKT network
    Ckt_Obj_Type_t            type;             // object type
    bool                      isVisited;        // whether the object is visited
    bool                      isNewInv;         // whether the object is a new added inverter
    int                       topoId;           // mark the index in the topological sequence
    std::vector <uint64_t>    valueClusters;    // simluation value clusters
    std::vector <uint64_t>    valueClustersBak; // simluation value clusters backup
    std::vector <uint64_t>    foConeInfo;       // mark whether POs are in the objects' fanout cone, each bit corresponds a PO
    std::vector <uint64_t>    BD;               // partial boolean difference
    Ckt_Obj_t *               pCktInv;          // pointer to its inverter
    std::vector <Ckt_Obj_t *> pCktFanins;       // fanin pointers
    std::vector <Ckt_Obj_t *> pCktFanouts;      // fanout pointers

    Ckt_Obj_t &               operator =        (const Ckt_Obj_t & other);

public:
    std::list <Ckt_Obj_t *>   cut;              // the minimum cut in which objects' fanout cone are disjoint
    std::list <Ckt_Obj_t *>   cutNtk;           // nodes among itself and its cut arranged in topological order
    uint64_t                  BDPlus;           // temporary
    uint64_t                  BDMinus;          // temporary

    explicit                  Ckt_Obj_t         (abc::Abc_Obj_t * p_abc_obj, Ckt_Ntk_t * p_ckt_ntk);
                              Ckt_Obj_t         (const Ckt_Obj_t & other);
                              ~Ckt_Obj_t        (void);
    void                      PrintFanios       (void) const;
    void                      PrintClusters     (void) const;
    void                      UpdateClusters    (void);
    void                      UpdateCluster     (int i);
    void                      ReplaceBy         (Ckt_Obj_t & cktNewObj, std::vector <Ckt_Rpl_Info_t> & info);
    void                      CheckFanio        (void) const;

    inline abc::Abc_Obj_t *   GetAbcObj         (void) const                    { return pAbcObj; }
    inline Ckt_Obj_Type_t     GetType           (void) const                    { return type; }
    inline bool               GetVisited        (void) const                    { return isVisited; }
    inline void               SetVisited        (void)                          { isVisited = true; }
    inline void               ResetVisited      (void)                          { isVisited = false; topoId = 0; }
    inline void               ResetVisited2     (void)                          { isVisited = false; }
    inline void               SetTopoId         (int i)                         { topoId = i; }
    inline int                GetTopoId         (void) const                    { return topoId; }
    inline int                GetClustersSize   (void) const                    { return static_cast <int> (valueClusters.size()); }
    inline void               ResizeClusters    (int len)                       { valueClusters.resize(len); }
    inline void               BackupClusters    (void)                          { valueClustersBak.assign(valueClusters.begin(), valueClusters.end()); }
    inline void               RecoverCluster    (int i)                         { valueClusters[i] = valueClustersBak[i]; }
    inline void               SetCluster        (int i, uint64_t value)         { valueClusters[i] = value; }
    inline void               FlipCluster       (int i)                         { valueClusters[i] = ~valueClusters[i]; }
    inline uint64_t           GetCluster        (int i) const                   { return valueClusters[i]; }
    inline uint64_t           XorClusterBak     (int i) const                   { return valueClusters[i] ^ valueClustersBak[i]; }
    inline void               InitFoCone        (int f)                         { foConeInfo.resize((f >> 6) + 1, 0); }
    inline int                GetFoConeSize     (void) const                    { return static_cast <int> (foConeInfo.size()); }
    inline uint64_t           GetFoCone         (int i) const                   { return foConeInfo[i]; }
    inline void               SetFoCone         (int f)                         { Ckt_SetBit(foConeInfo[f >> 6], f); }
    inline void               SelfOrFoCone      (Ckt_Obj_t * pCktObj)           { for (int i = 0; i < GetFoConeSize(); ++i) foConeInfo[i] |= pCktObj->foConeInfo[i]; }
    inline void               SetBD             (int i, uint64_t value)         { BD[i] = value; }
    inline uint64_t           GetBD             (int i) const                   { return BD[i]; }
    inline void               SelfOrBD          (int i, uint64_t value)         { BD[i] |= value; }
    inline void               AddFanin          (Ckt_Obj_t * pCktFanin)         { pCktFanins.emplace_back(pCktFanin); pCktFanin->pCktFanouts.emplace_back(this); }
    inline Ckt_Obj_t *        GetFanin          (int i = 0) const               { return pCktFanins[i]; }
    inline int                GetFaninNum       (void) const                    { return static_cast <int> (pCktFanins.size()); }
    inline void               WriteFanin        (int i, Ckt_Obj_t * pCktFanin)  { pCktFanins[i] = pCktFanin; }
    inline Ckt_Obj_t *        GetFanout         (int i = 0) const               { return pCktFanouts[i]; }
    inline int                GetFanoutNum      (void) const                    { return static_cast <int> (pCktFanouts.size()); }
    inline void               InsertFanout      (int i, Ckt_Obj_t * pCktFanout) { pCktFanouts.insert(pCktFanouts.begin() + i, pCktFanout); }
    inline void               PopBackFanout     (void)                          { pCktFanouts.pop_back(); }
    inline std::string        GetName           (void) const                    { return std::string(Abc_ObjName(pAbcObj)); }
    inline float              GetArrivalTime    (void) const                    { return (static_cast<abc::Abc_Time_t *>(pAbcObj->pNtk->pManTime->vArrs->pArray[pAbcObj->Id]))->Rise; }
    inline bool               IsPI              (void) const                    { return type == Ckt_Obj_Type_t::PI; }
    inline bool               IsPO              (void) const                    { return type == Ckt_Obj_Type_t::PO; }
    inline bool               IsConst           (void) const                    { return type == Ckt_Obj_Type_t::CONST0 || type == Ckt_Obj_Type_t::CONST1; }
    inline bool               IsInv             (void) const                    { return type == Ckt_Obj_Type_t::INV; }
    inline bool               IsDanggling       (void) const                    { return pCktFanouts.empty() && type != Ckt_Obj_Type_t::PO; }
    inline bool               IsAddedInv        (void) const                    { return isNewInv; }
    inline bool               HasAddedInv       (void) const                    { return pCktInv != nullptr; }
    inline Ckt_Obj_t *        GetAddedInv       (void) const                    { return pCktInv; }
    inline void               SetAddedInv       (Ckt_Obj_t & cktObjInv)         { pCktInv = &cktObjInv; cktObjInv.isNewInv = true; }
};


class Ckt_Rpl_Info_t
{
public:
    Ckt_Obj_t *               pCktObjFrom;
    int                       iCktFanin;
    Ckt_Obj_t *               pCktObjTo;
    int                       iCktFanout;

    explicit                  Ckt_Rpl_Info_t    (Ckt_Obj_t * pObj1, int iFanin, Ckt_Obj_t * pObj2, int iFanout);
                              ~Ckt_Rpl_Info_t   (void);
};


std::ostream &                operator <<       (std::ostream & os, const Ckt_Obj_Type_t & type);
std::ostream &                operator <<       (std::ostream & os, const Ckt_Rpl_Info_t & info);
Ckt_Obj_Type_t                Ckt_GetObjType    (abc::Abc_Obj_t * pAbcObj);
bool                          Ckt_SopIsConst0   (char * pSop);
bool                          Ckt_SopIsConst1   (char * pSop);
bool                          Ckt_SopIsBuf      (char * pSop);
bool                          Ckt_SopIsInvGate  (char * pSop);
bool                          Ckt_SopIsAndGate  (char * pSop);
bool                          Ckt_SopIsOrGate   (char * pSop);
bool                          Ckt_SopIsNandGate (char * pSop);
bool                          Ckt_SopIsNorGate  (char * pSop);
bool                          Ckt_SopIsXorGate  (char * pSop);
bool                          Ckt_SopIsXnorGate (char * pSop);
bool                          Ckt_SopIsAOI21Gate(char * pSop);
bool                          Ckt_SopIsAOI22Gate(char * pSop);
bool                          Ckt_SopIsOAI21Gate(char * pSop);
bool                          Ckt_SopIsOAI22Gate(char * pSop);


#endif
