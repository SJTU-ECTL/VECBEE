#ifndef CKT_BEE_H
#define CKT_BEE_H


#include "cktNtk.h"


class Ckt_Rpl_Pair_t
{
public:
    Ckt_Obj_t * pTS;
    Ckt_Obj_t * pSS;
    int addedER;

    Ckt_Rpl_Pair_t(Ckt_Obj_t * p_ts, Ckt_Obj_t * p_ss);
    Ckt_Rpl_Pair_t(const Ckt_Rpl_Pair_t & other);
    ~Ckt_Rpl_Pair_t(void);
};


std::ostream &  operator <<         (std::ostream & os, const Ckt_Rpl_Pair_t & pr);
void            BatchErrorEstimation(Ckt_Ntk_t & ckt, Ckt_Ntk_t & cktRef);
Ckt_Obj_t *     CheckExpansion      (std::list <Ckt_Obj_t *> & cut);
void            Expand              (Ckt_Obj_t & cktObj, std::list <Ckt_Obj_t *> & cut);
void            FindCut             (Ckt_Ntk_t & ckt, std::list <Ckt_Obj_t *> & cut, Ckt_Obj_t & cktSrcObj);
void            BuildSubNtk         (std::vector <Ckt_Obj_t *> & pOrdObjs, std::list <Ckt_Obj_t *> & subNtk);
bool            HasSamePo           (Ckt_Ntk_t & ckt1, Ckt_Ntk_t & ckt2);
void            GetBooleanDifference(Ckt_Ntk_t & ckt, std::vector <Ckt_Obj_t *> & pOrdObjs, std::vector <uint64_t> & isPoICorrect, int fb);
void            GetAddedErrorRate   (Ckt_Ntk_t & ckt, std::vector <Ckt_Rpl_Pair_t> & pairs, int fb, uint64_t isCorrect);
void            GetValidPair        (Ckt_Ntk_t & ckt, std::vector <Ckt_Obj_t *> & pOrdObjs, std::vector < Ckt_Rpl_Pair_t > & pairs);


#endif
