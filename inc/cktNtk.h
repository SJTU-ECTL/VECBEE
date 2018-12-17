#ifndef CKT_NTK_H
#define CKT_NTK_H


#include <bits/stdc++.h>
#include <boost/random.hpp>
#include "cktObj.h"


class Ckt_Ntk_t
{
private:
    abc::Abc_Ntk_t *        pAbcNtk;        // the corresponding ABC network
    int                     nValueClusters; // nValueClusters = # frame / 64, discard remaining frames
    std::list <Ckt_Obj_t>   cktObjs;        // circuit objects
    std::list <Ckt_Obj_t *> pCktPis;         // primary inputs pointers
    std::list <Ckt_Obj_t *> pCktPos;         // primary outputs pointers

    Ckt_Ntk_t(const Ckt_Ntk_t & other);
    Ckt_Ntk_t & operator = (const Ckt_Ntk_t & other);

public:
    explicit Ckt_Ntk_t(abc::Abc_Ntk_t * p_abc_ntk, int nFrames = 1024);
    ~Ckt_Ntk_t(void);
    void PrintInfo(void) const;
    void GenerateInputDistribution(unsigned seed = 314);
};


#endif
