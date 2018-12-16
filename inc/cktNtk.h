#ifndef CKT_NTK_H
#define CKT_NTK_H


#include <bits/stdc++.h>
#include <boost/random.hpp>
#include "cktObj.h"


class Ckt_Ntk_t
{
private:
    abc::Abc_Ntk_t *        pAbcNtk;    // the corresponding ABC network
    std::list <Ckt_Obj_t>   cktObjs;    // a list of circuit objects
    std::list <Ckt_Obj_t>   cktPis;     // a list of primary inputs
    std::list <Ckt_Obj_t>   cktPos;     // a list of primary outputs

    Ckt_Ntk_t(const Ckt_Ntk_t & other);
    Ckt_Ntk_t & operator = (const Ckt_Ntk_t & other);

public:
    explicit Ckt_Ntk_t(abc::Abc_Ntk_t * p_abc_ntk);
    ~Ckt_Ntk_t(void);
    void PrintInfo(void) const;
    void GenerateInputDistribution(void);
};


#endif
