#ifndef CKT_SOP_H
#define CKT_SOP_H


#include <bits/stdc++.h>
#include "abcApi.h"
#include "cktBit.h"
#include "cktSopNet.h"


class Ckt_Sing_Sel_Info_t;
class Ckt_Sop_Net_t;


enum class Ckt_Sop_Cat_t
{
    PI, PO, CONST0, CONST1, INTER
};


class Ckt_Sop_t
{
private:
    //
    // common properties
    //
    abc::Abc_Obj_t *                        pAbcObj;            // the corresponding ABC object
    Ckt_Sop_Net_t *                         pCktNtk;            // the corresponding CKT network
    Ckt_Sop_Cat_t                           type;               // object type
    bool                                    isVisited;          // whether the object is visited
    int                                     topoId;             // mark the index in the topological sequence
    int                                     nLiterals;          // # literals
    std::vector <std::string>               SOP;                // positional cube notation
    std::vector <uint64_t>                  valueClusters;      // simluation value clusters
    std::vector <uint64_t>                  foConeInfo;         // mark whether POs are in the objects' fanout cone, each bit corresponds a PO
    std::vector <Ckt_Sop_t *>               pCktFanins;         // fanin pointers
    std::vector <Ckt_Sop_t *>               pCktFanouts;        // fanout pointers

    //
    // only used for cut network
    //
    Ckt_Sop_Net_t *                         pCktCutNtk;         // cut network, containing a copy of objects between itself and its cut
    Ckt_Sop_t *                             pCktObjOri;         // the original object in the parent network
    Ckt_Sop_t *                             pCktObjCopy;        // temporarily record the copied object
    std::vector <uint64_t>                  isDiff;             // valueClusters ^ pCktObjOri->valueClusters

    //
    // only used for batch error estimation
    //
    std::vector <uint64_t>                  BD;                 // partial boolean difference
    std::vector <uint64_t>                  BDInc;              // influence on all POs which will increase the error rate
    std::vector <uint64_t>                  BDDec;              // influence on all POs which will decrease the error rate

    //
    // forbid automatically generate
    //
    Ckt_Sop_t &                             operator =          (const Ckt_Sop_t &);


public:
    explicit                                Ckt_Sop_t           (abc::Abc_Obj_t * p_abc_obj, Ckt_Sop_Net_t * p_ckt_ntk);
                                            Ckt_Sop_t           (const Ckt_Sop_t & other);
                                            ~Ckt_Sop_t          (void);
    void                                    PrintFanios         (void) const;
    void                                    CollectSOP          (void);
    int                                     GetLiteralsNum      (void);
    void                                    PrintClusters       (void) const;
    void                                    UpdateClusters      (void);
    void                                    UpdateCluster       (int i);
    void                                    FlipClustersFrom    (Ckt_Sop_t * pCktObj);
    uint64_t                                GetClusterValue     (std::vector <std::string> & newSOP, Ckt_Sop_Cat_t type, int i);
    void                                    GetClustersValue    (std::vector <std::string> & newSOP, Ckt_Sop_Cat_t type, std::vector <uint64_t> & values);
    void                                    XorClustersValue    (std::vector <uint64_t> & values);
    void                                    ReplaceBy           (std::vector <std::string> & newSOP, Ckt_Sop_Cat_t _type, Ckt_Sing_Sel_Info_t & info);
    void                                    CheckFanio          (void) const;
    void                                    ClearCutNtk         (void);
    void                                    ClearMem            (void);

    inline abc::Abc_Obj_t *                 GetAbcObj           (void) const                                                        { return pAbcObj; }
    inline Ckt_Sop_Net_t *                  GetCktNtk           (void) const                                                        { return pCktNtk; }
    inline Ckt_Sop_Cat_t                    GetType             (void) const                                                        { return type; }
    inline void                             SetType             (Ckt_Sop_Cat_t _type)                                               { type = _type; }
    inline bool                             GetVisited          (void) const                                                        { return isVisited; }
    inline void                             SetVisited          (void)                                                              { isVisited = true; }
    inline void                             ResetVisited        (void)                                                              { isVisited = false; topoId = 0; }
    inline void                             ResetVisited2       (void)                                                              { isVisited = false; }
    inline void                             SetTopoId           (int i)                                                             { topoId = i; }
    inline int                              GetTopoId           (void) const                                                        { return topoId; }
    inline int                              GetNLiterals        (void) const                                                        { return nLiterals; }
    inline int                              GetClustersSize     (void) const                                                        { return static_cast <int> (valueClusters.size()); }
    inline int                              GetSimNum           (void) const                                                        { return static_cast <int> (valueClusters.size()); }
    inline void                             ResizeClusters      (int len)                                                           { valueClusters.resize(len); }
    inline void                             SetCluster          (int i, uint64_t value)                                             { valueClusters[i] = value; }
    inline void                             FlipCluster         (int i)                                                             { valueClusters[i] = ~valueClusters[i]; }
    inline uint64_t                         GetCluster          (int i) const                                                       { return valueClusters[i]; }
    inline int                              GetSOPSize          (void) const                                                        { return static_cast <int> (SOP.size()); }
    inline int                              GetSOPISize         (int i) const                                                       { return static_cast <int> (SOP[i].size()); }
    inline void                             SetSOP              (const std::vector <std::string> & s)                               { SOP.assign(s.begin(), s.end()); }
    inline void                             SetSOPIJ            (int i, int j, char val)                                            { SOP[i][j] = val; }
    inline char                             GetSOPIJ            (int i, int j) const                                                { return SOP[i][j]; }
    inline void                             CopySOP             (std::vector <std::string> & s) const                               { s.assign(SOP.begin(), SOP.end()); }
    inline std::vector <std::string>        GetSOP              (void) const                                                        { return SOP; }
    inline void                             InitFoCone          (int f)                                                             { foConeInfo.resize((f >> 6) + 1, 0); }
    inline int                              GetFoConeSize       (void) const                                                        { return static_cast <int> (foConeInfo.size()); }
    inline uint64_t                         GetFoCone           (int i) const                                                       { return foConeInfo[i]; }
    inline void                             SetFoCone           (int f)                                                             { Ckt_SetBit(foConeInfo[f >> 6], f); }
    inline void                             SelfOrFoCone        (Ckt_Sop_t * pCktObj)                                               { for (int i = 0; i < GetFoConeSize(); ++i) foConeInfo[i] |= pCktObj->foConeInfo[i]; }
    inline void                             AddFanio            (Ckt_Sop_t * pCktFanin)                                             { pCktFanins.emplace_back(pCktFanin); pCktFanin->pCktFanouts.emplace_back(this); }
    inline void                             AddFanin            (Ckt_Sop_t * pCktFanin)                                             { pCktFanins.emplace_back(pCktFanin); }
    inline Ckt_Sop_t *                      GetFanin            (int i = 0) const                                                   { return pCktFanins[i]; }
    inline int                              GetFaninNum         (void) const                                                        { return static_cast <int> (pCktFanins.size()); }
    inline void                             WriteFanin          (int i, Ckt_Sop_t * pCktFanin)                                      { pCktFanins[i] = pCktFanin; }
    inline Ckt_Sop_t *                      GetFanout           (int i = 0) const                                                   { return pCktFanouts[i]; }
    inline int                              GetFanoutNum        (void) const                                                        { return static_cast <int> (pCktFanouts.size()); }
    inline void                             InsertFanout        (int i, Ckt_Sop_t * pCktFanout)                                     { pCktFanouts.insert(pCktFanouts.begin() + i, pCktFanout); }
    inline void                             PopBackFanout       (void)                                                              { pCktFanouts.pop_back(); }
    inline std::string                      GetName             (void) const                                                        { return std::string(Abc_ObjName(pAbcObj)); }
    inline bool                             IsPI                (void) const                                                        { return type == Ckt_Sop_Cat_t::PI; }
    inline bool                             IsPO                (void) const                                                        { return type == Ckt_Sop_Cat_t::PO; }
    inline bool                             IsConst             (void) const                                                        { return type == Ckt_Sop_Cat_t::CONST0 || type == Ckt_Sop_Cat_t::CONST1; }
    inline bool                             IsConst0            (void) const                                                        { return type == Ckt_Sop_Cat_t::CONST0; }
    inline bool                             IsConst1            (void) const                                                        { return type == Ckt_Sop_Cat_t::CONST1; }
    inline bool                             IsDanggling         (void) const                                                        { return pCktFanouts.empty() && !IsPO(); }
    inline void                             SetCutNtk           (Ckt_Sop_Net_t * p_ckt_cut_ntk)                                     { pCktCutNtk = p_ckt_cut_ntk; }
    inline Ckt_Sop_Net_t *                  GetCutNtk           (void) const                                                        { return pCktCutNtk; }
    inline void                             SetOriObj           (Ckt_Sop_t * pCktObj)                                               { pCktObjOri = pCktObj; }
    inline Ckt_Sop_t *                      GetOriObj           (void) const                                                        { return pCktObjOri; }
    inline void                             SetCopiedObj        (Ckt_Sop_t * pCktObj)                                               { pCktObjCopy = pCktObj; }
    inline void                             ClearCopiedObj      (void)                                                              { pCktObjCopy = nullptr; }
    inline Ckt_Sop_t *                      GetCopiedObj        (void) const                                                        { return pCktObjCopy; }
    inline bool                             IsInBaseNtk         (void) const                                                        { return (GetOriObj() == nullptr); }
    inline bool                             IsInCutNtk          (void) const                                                        { return (GetOriObj() != nullptr); }
    inline void                             ResizeIsDiff        (void)                                                              { isDiff.resize(GetSimNum()); }
    inline void                             UpdateIsDiff        (void)                                                              { for (int i = 0; i < GetSimNum(); ++i) isDiff[i] = valueClusters[i] ^ pCktObjOri->valueClusters[i]; }
    inline void                             ResizeBD            (void)                                                              { BD.resize(GetSimNum()); }
    inline void                             SetBD               (void)                                                              { std::fill(BD.begin(), BD.end(), static_cast <uint64_t> (ULLONG_MAX)); }
    inline void                             ResetBD             (void)                                                              { std::fill(BD.begin(), BD.end(), 0); }
    inline void                             UpdateBD            (Ckt_Sop_t * pCut)                                                  { Ckt_Sop_t * pOriCut = pCut->GetOriObj(); for (int i = 0; i < GetSimNum(); ++i) BD[i] |= (pCut->isDiff[i] & pOriCut->BD[i]); }
    inline int                              GetBDSize           (void) const                                                        { return static_cast <int> (BD.size()); }
    inline uint64_t                         GetBD               (int i) const                                                       { return BD[i]; }
    inline void                             ResizeBDInc         (void)                                                              { BDInc.resize(GetSimNum()); }
    inline void                             ResetBDInc          (void)                                                              { std::fill(BDInc.begin(), BDInc.end(), 0); }
    inline void                             UpdateBDInc         (void)                                                              { for (int i = 0; i < GetSimNum(); ++i) BDInc[i] |= BD[i]; }
    inline uint64_t                         GetBDInc            (int i) const                                                       { return BDInc[i]; }
    inline int                              GetIncER            (std::vector <uint64_t> & isCor, std::vector <uint64_t> & vals )    { int ret = 0; for (int i = 0; i < GetSimNum(); ++i) ret += Ckt_CountOneNum(isCor[i] & BDInc[i] & vals[i]); return ret; }
    inline void                             ResizeBDDec         (void)                                                              { BDDec.resize(GetSimNum()); }
    inline void                             ResetBDDec          (void)                                                              { std::fill(BDDec.begin(), BDDec.end(), static_cast <uint64_t> (ULLONG_MAX)); }
    inline void                             UpdateBDDec         (std::vector <uint64_t> & isPoCorrect)                              { for (int i = 0; i < GetSimNum(); ++i) BDDec[i] &= (isPoCorrect[i] ^ BD[i]); }
    inline uint64_t                         GetBDDec            (int i) const                                                       { return BDDec[i]; }
    inline int                              GetDecER            (std::vector <uint64_t> & isCor, std::vector <uint64_t> & vals )    { int ret = 0; for (int i = 0; i < GetSimNum(); ++i) ret += Ckt_CountOneNum(~isCor[i] & BDDec[i] & vals[i]); return ret; }
};


class Ckt_Sing_Sel_Info_t
{
public:
    Ckt_Sop_t *                             pCktObj;
    Ckt_Sop_Cat_t                           type;
    std::vector <std::string>               SOP;

    explicit                                Ckt_Sing_Sel_Info_t (void);
    explicit                                Ckt_Sing_Sel_Info_t (Ckt_Sop_t * p_ckt_obj, Ckt_Sop_Cat_t _type, const std::vector <std::string> & _sop);
                                            ~Ckt_Sing_Sel_Info_t(void);
};


std::ostream &                              operator <<         (std::ostream & os, const std::vector <std::string> & SOP);
std::ostream &                              operator <<         (std::ostream & os, const Ckt_Sop_Cat_t & type);
Ckt_Sop_Cat_t                               Abc_GetSopType      (abc::Abc_Obj_t * pObj);
int                                         GetLiteralsNum      (const std::vector <std::string> & SOP);


#endif
