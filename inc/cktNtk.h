#ifndef CKT_NTK_H
#define CKT_NTK_H


#include <boost/random.hpp>
#include "cktObj.h"
#include "cktCec.h"
#include "cktVisual.h"
#include "cktTiming.h"


class Ckt_Obj_t;
class Ckt_Rpl_Info_t;


class Ckt_Bit_Cnt_t
{
private:
    int table[65536];

public:
    explicit                  Ckt_Bit_Cnt_t    (void);
                              ~Ckt_Bit_Cnt_t   (void);

    inline int                GetOneNum        (uint64_t n) { return table[n >> 48] + table[(n >> 32) & 0xffff] + table[(n >> 16) & 0xffff] +table[n & 0xffff]; }
};


class Ckt_Ntk_t
{
private:
    abc::Abc_Ntk_t *          pAbcNtk;         // the corresponding ABC network
    int                       nValueClusters;  // nValueClusters = # frame / 64, discard remaining frames
    std::list <Ckt_Obj_t>     cktObjs;         // circuit objects
    std::vector <Ckt_Obj_t *> pCktPis;         // primary inputs pointers
    std::vector <Ckt_Obj_t *> pCktPos;         // primary outputs pointers
    Ckt_Obj_t *               pCktConst0;      // const 0 pointer
    Ckt_Obj_t *               pCktConst1;      // const 1 pointer

                              Ckt_Ntk_t        (const Ckt_Ntk_t & other);
    Ckt_Ntk_t &               operator =       (const Ckt_Ntk_t & other);

public:
    explicit                  Ckt_Ntk_t        (abc::Abc_Ntk_t * p_abc_ntk, int nFrames = 1024);
                              ~Ckt_Ntk_t       (void);
    void                      PrintInfo        (void) const;
    void                      PrintTopoOrder   (void);
    void                      GenInputDist     (unsigned seed = 314);
    void                      PrintInput       (void) const;
    void                      PrintOutput      (void) const;
    void                      DFS              (Ckt_Obj_t * pCktObj, std::vector <Ckt_Obj_t *> & pOrderedObjs);
    void                      SetAllUnvisited  (void);
    void                      SetAllUnvisited2 (void);
    void                      SortObjects      (std::vector <Ckt_Obj_t *> & pOrderedObjs);
    void                      FeedForward      (void);
    void                      FeedForward      (std::vector <Ckt_Obj_t *> & pOrderedObjs);
    void                      CheckSimulator   (void);
    float                     GetErrorRate     (Ckt_Ntk_t & refNtk, Ckt_Bit_Cnt_t * pTable = nullptr);
    Ckt_Obj_t *               AddInverter      (Ckt_Obj_t & cktObj);
    Ckt_Obj_t *               GetInverter      (Ckt_Obj_t & cktObj);
    void                      Replace          (Ckt_Obj_t & cktOldObj, Ckt_Obj_t & cktNewObj, std::vector <Ckt_Rpl_Info_t> & info, bool isInv = false);
    void                      ReplaceByName    (std::string oldName, std::string newName, std::vector <Ckt_Rpl_Info_t> & info);
    void                      RecoverFromRpl   (std::vector <Ckt_Rpl_Info_t> & info);
    void                      CheckFanio       (void) const;
    void                      ReplaceTest      (void);
    void                      UpdateFoCone     (void);

    inline int                GetObjNum        (void) const { return static_cast <int> (cktObjs.size()); }
    inline int                GetPoNum         (void) const { return static_cast <int> (pCktPos.size()); }
    inline int                GetValClustersNum(void) const { return nValueClusters; }
    inline abc::Abc_Ntk_t *   GetAbcNtk        (void) const { return pAbcNtk; }
    inline void               GetArrivalTime   (void) const { Abc_GetArrivalTime(GetAbcNtk()); }
};


#endif
