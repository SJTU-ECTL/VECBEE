#ifndef CKT_SOP_H
#define CKT_SOP_H


#include <bits/stdc++.h>
#include "abcApi.h"
#include "cktBit.h"
#include "cktSopNet.h"


class Ckt_Sop_Net_t;


enum class Ckt_Sop_Cat_t
{
    PI, PO, CONST0, CONST1, INTER
};


class Ckt_Sop_t
{
private:
    abc::Abc_Obj_t *                        pAbcObj;            // the corresponding ABC object
    Ckt_Sop_Net_t *                         pCktNtk;            // the corresponding CKT network
    Ckt_Sop_Cat_t                           type;               // object type
    bool                                    isVisited;          // whether the object is visited
    int                                     topoId;             // mark the index in the topological sequence
    int                                     nLiterals;          // # literals
    std::vector <uint64_t>                  valueClusters;      // simluation value clusters
    std::vector <uint64_t>                  valueClustersBak;   // simluation value clusters backup
    std::vector <std::string>               SOP;                // positional cube notation
    std::vector < std::vector<std::string> >nonConstALCs;       // approximate local changes (exclude 0/1)
    std::vector <uint64_t>                  foConeInfo;         // mark whether POs are in the objects' fanout cone, each bit corresponds a PO
    std::vector <uint64_t>                  BD;                 // partial boolean difference
    std::vector <Ckt_Sop_t *>               pCktFanins;         // fanin pointers
    std::vector <Ckt_Sop_t *>               pCktFanouts;        // fanout pointers

    Ckt_Sop_t &                             operator =          (const Ckt_Sop_t &);

public:
    std::list <Ckt_Sop_t *>                 cut;                // the minimum cut in which objects' fanout cone are disjoint
    std::list <Ckt_Sop_t *>                 cutNtk;             // nodes among itself and its cut arranged in topological order
    uint64_t                                BDPlus;             // temporary
    uint64_t                                BDMinus;            // temporary

    explicit                                Ckt_Sop_t           (abc::Abc_Obj_t * p_abc_obj, Ckt_Sop_Net_t * p_ckt_ntk);
                                            Ckt_Sop_t           (const Ckt_Sop_t & other);
                                            ~Ckt_Sop_t          (void);
    void                                    PrintFanios         (void) const;
    void                                    PrintSOP            (void) const;
    void                                    PrintSOP            (std::vector <std::string> & strs) const;
    void                                    CollectSOP          (void);
    void                                    GenerateALCs        (int maxNLiterals = 5);
    void                                    GenerateALCsRecur   (int i, int j, int n, int maxNLiterals = 5);
    void                                    PrintALCs           (void) const;
    int                                     GetLiteralsNum      (void);
    void                                    PrintClusters       (void) const;
    void                                    UpdateClusters      (void);
    void                                    UpdateCluster       (int i);
    void                                    CheckFanio          (void) const;
    void                                    PrintBD             (void) const;

    inline abc::Abc_Obj_t *                 GetAbcObj           (void) const                    { return pAbcObj; }
    inline Ckt_Sop_Cat_t                    GetType             (void) const                    { return type; }
    inline bool                             GetVisited          (void) const                    { return isVisited; }
    inline void                             SetVisited          (void)                          { isVisited = true; }
    inline void                             ResetVisited        (void)                          { isVisited = false; topoId = 0; }
    inline void                             ResetVisited2       (void)                          { isVisited = false; }
    inline void                             SetTopoId           (int i)                         { topoId = i; }
    inline int                              GetTopoId           (void) const                    { return topoId; }
    inline int                              GetNLiterals        (void) const                    { return nLiterals; }
    inline int                              GetNonConstALCsSize (void) const                    { return static_cast <int> (nonConstALCs.size()); }
    inline int                              GetALCsNum          (void) const                    { return GetNonConstALCsSize() + 2; }
    inline int                              GetClustersSize     (void) const                    { return static_cast <int> (valueClusters.size()); }
    inline void                             ResizeClusters      (int len)                       { valueClusters.resize(len); }
    inline void                             BackupClusters      (void)                          { valueClustersBak.assign(valueClusters.begin(), valueClusters.end()); }
    inline void                             RecoverCluster      (int i)                         { valueClusters[i] = valueClustersBak[i]; }
    inline void                             SetCluster          (int i, uint64_t value)         { valueClusters[i] = value; }
    inline void                             FlipCluster         (int i)                         { valueClusters[i] = ~valueClusters[i]; }
    inline uint64_t                         GetCluster          (int i) const                   { return valueClusters[i]; }
    inline uint64_t                         XorClusterBak       (int i) const                   { return valueClusters[i] ^ valueClustersBak[i]; }
    inline void                             InitFoCone          (int f)                         { foConeInfo.resize((f >> 6) + 1, 0); }
    inline int                              GetFoConeSize       (void) const                    { return static_cast <int> (foConeInfo.size()); }
    inline uint64_t                         GetFoCone           (int i) const                   { return foConeInfo[i]; }
    inline void                             SetFoCone           (int f)                         { Ckt_SetBit(foConeInfo[f >> 6], f); }
    inline void                             SelfOrFoCone        (Ckt_Sop_t * pCktObj)           { for (int i = 0; i < GetFoConeSize(); ++i) foConeInfo[i] |= pCktObj->foConeInfo[i]; }
    inline void                             SetBD               (int i, uint64_t value)         { BD[i] = value; }
    inline uint64_t                         GetBD               (int i) const                   { return BD[i]; }
    inline void                             SelfOrBD            (int i, uint64_t value)         { BD[i] |= value; }
    inline void                             AddFanin            (Ckt_Sop_t * pCktFanin)         { pCktFanins.emplace_back(pCktFanin); pCktFanin->pCktFanouts.emplace_back(this); }
    inline Ckt_Sop_t *                      GetFanin            (int i = 0) const               { return pCktFanins[i]; }
    inline int                              GetFaninNum         (void) const                    { return static_cast <int> (pCktFanins.size()); }
    inline void                             WriteFanin          (int i, Ckt_Sop_t * pCktFanin)  { pCktFanins[i] = pCktFanin; }
    inline Ckt_Sop_t *                      GetFanout           (int i = 0) const               { return pCktFanouts[i]; }
    inline int                              GetFanoutNum        (void) const                    { return static_cast <int> (pCktFanouts.size()); }
    inline void                             InsertFanout        (int i, Ckt_Sop_t * pCktFanout) { pCktFanouts.insert(pCktFanouts.begin() + i, pCktFanout); }
    inline void                             PopBackFanout       (void)                          { pCktFanouts.pop_back(); }
    inline std::string                      GetName             (void) const                    { return std::string(Abc_ObjName(pAbcObj)); }
    inline bool                             IsPI                (void) const                    { return type == Ckt_Sop_Cat_t::PI; }
    inline bool                             IsPO                (void) const                    { return type == Ckt_Sop_Cat_t::PO; }
    inline bool                             IsConst             (void) const                    { return type == Ckt_Sop_Cat_t::CONST0 || type == Ckt_Sop_Cat_t::CONST1; }
    inline bool                             IsConst0            (void) const                    { return type == Ckt_Sop_Cat_t::CONST0; }
    inline bool                             IsConst1            (void) const                    { return type == Ckt_Sop_Cat_t::CONST1; }
    inline bool                             IsDanggling         (void) const                    { return pCktFanouts.empty() && !IsPO(); }
};


std::ostream &                              operator <<         (std::ostream & os, const Ckt_Sop_Cat_t & type);
Ckt_Sop_Cat_t                               Abc_GetSopType      (abc::Abc_Obj_t * pObj);


#endif
