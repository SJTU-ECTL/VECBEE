#ifndef CKT_GATE_NET_H
#define CKT_GATE_NET_H


#include <boost/random.hpp>
#include "cktGate.h"


class Ckt_Gate_t;
class Ckt_SASIMI_Info_t;


class Ckt_Gate_Net_t
{
private:
    abc::Abc_Ntk_t *            pAbcNtk;            // the corresponding ABC network
    int                         nValueClusters;     // nValueClusters = # frame / 64, discard remaining frames
    std::list <Ckt_Gate_t>      cktObjs;            // circuit objects
    std::vector <Ckt_Gate_t *>  pCktPis;            // primary inputs pointers
    std::vector <Ckt_Gate_t *>  pCktPos;            // primary outputs pointers
    Ckt_Gate_t *                pCktConst0;         // const 0 pointer
    Ckt_Gate_t *                pCktConst1;         // const 1 pointer

    Ckt_Gate_Net_t &            operator =          (const Ckt_Gate_Net_t & other);

public:
    explicit                    Ckt_Gate_Net_t      (abc::Abc_Ntk_t * p_abc_ntk, int nFrames = 1024);
                                Ckt_Gate_Net_t      (const Ckt_Gate_Net_t & other);
                                ~Ckt_Gate_Net_t     (void);
    void                        PrintInfo           (void) const;
    void                        PrintTopoOrder      (void);
    void                        GenInputDist        (unsigned seed = 314);
    void                        PrintInput          (void) const;
    void                        PrintOutput         (void) const;
    void                        DFS                 (Ckt_Gate_t * pCktObj, std::vector <Ckt_Gate_t *> & pOrderedObjs);
    void                        SetAllUnvisited     (void);
    void                        SetAllUnvisited2    (void);
    void                        SortObjects         (std::vector <Ckt_Gate_t *> & pOrderedObjs);
    void                        FeedForward         (void);
    void                        FeedForward         (std::vector <Ckt_Gate_t *> & pOrderedObjs);
    void                        FeedForward         (std::list <Ckt_Gate_t *> & pOrderedObjs, int i);
    void                        BackupSimRes        (void);
    void                        CheckSimulator      (void);
    void                        TestSimulatorSpeed  (void);
    int                         GetErrorRate        (Ckt_Gate_Net_t & refNtk);
    Ckt_Gate_t *                AddInverter         (Ckt_Gate_t & cktObj);
    Ckt_Gate_t *                GetInverter         (Ckt_Gate_t & cktObj);
    Ckt_Gate_t *                GetInverter2        (Ckt_Gate_t & cktObj);
    void                        Replace             (Ckt_Gate_t & cktOldObj, Ckt_Gate_t & cktNewObj, std::vector <Ckt_SASIMI_Info_t> & info, bool isInv = false);
    void                        ReplaceByName       (std::string oldName, std::string newName, std::vector <Ckt_SASIMI_Info_t> & info);
    void                        RecoverFromRpl      (std::vector <Ckt_SASIMI_Info_t> & info);
    void                        CheckFanio          (void) const;
    void                        UpdateFoCone        (void);
    void                        PrintCut            (void) const;
    void                        PrintCutNtk         (void) const;
    void                        PrintSimRes         (void) const;
    void                        PrintBD             (void) const;

    inline int                  GetObjNum           (void) const        { return static_cast <int> (cktObjs.size()); }
    inline int                  GetPoNum            (void) const        { return static_cast <int> (pCktPos.size()); }
    inline Ckt_Gate_t *         GetPo               (int i = 0) const   { return pCktPos[i]; }
    inline Ckt_Gate_t *         GetConst0           (void) const        { return pCktConst0; }
    inline Ckt_Gate_t *         GetConst1           (void) const        { return pCktConst1; }
    inline int                  GetValClustersNum   (void) const        { return nValueClusters; }
    inline abc::Abc_Ntk_t *     GetAbcNtk           (void) const        { return pAbcNtk; }
    inline std::string          GetName             (void) const        { return std::string(pAbcNtk->pName); }
};


#endif
