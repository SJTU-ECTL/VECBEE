#ifndef CKT_NTK_H
#define CKT_NTK_H


#include <bits/stdc++.h>
#include <boost/random.hpp>
#include "cktObj.h"


class Ckt_Ntk_t
{
private:
    abc::Abc_Ntk_t *          pAbcNtk;        // the corresponding ABC network
    int                       nValueClusters; // nValueClusters = # frame / 64, discard remaining frames
    std::list <Ckt_Obj_t>     cktObjs;        // circuit objects
    std::vector <Ckt_Obj_t *> pCktPis;        // primary inputs pointers
    std::vector <Ckt_Obj_t *> pCktPos;        // primary outputs pointers
    Ckt_Obj_t *               pCktConst0;     // const 0 pointer
    Ckt_Obj_t *               pCktConst1;     // const 1 pointer

    Ckt_Ntk_t(const Ckt_Ntk_t & other);
    Ckt_Ntk_t & operator = (const Ckt_Ntk_t & other);

public:
    explicit Ckt_Ntk_t(abc::Abc_Ntk_t * p_abc_ntk, int nFrames = 1024);
    ~Ckt_Ntk_t(void);
    void PrintInfo(void) const;
    void PrintTopologicalOrder(void);
    void GenerateInputDistribution(unsigned seed = 314);
    void PrintInputDistribution(void) const;
    void DFS(Ckt_Obj_t * pCktObj, std::vector <Ckt_Obj_t *> & pOrderedObjs);
    void SortObjects(std::vector <Ckt_Obj_t *> & pOrderedObjs);
    void FeedForward(void);
    void SimulatorChecker(void);

    inline int GetObjNum(void) const { return cktObjs.size(); }
};


#endif
