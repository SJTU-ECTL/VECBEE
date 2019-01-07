#ifndef CKT_BEE_H
#define CKT_BEE_H


#include "cktGateNet.h"


class Ckt_Rpl_Pair_t
{
public:
    Ckt_Gate_t * pTS;
    Ckt_Gate_t * pSS;
    int addedER;

    Ckt_Rpl_Pair_t(Ckt_Gate_t * p_ts, Ckt_Gate_t * p_ss);
    Ckt_Rpl_Pair_t(const Ckt_Rpl_Pair_t & other);
    ~Ckt_Rpl_Pair_t(void);
};


std::ostream &  operator <<         (std::ostream & os, const Ckt_Rpl_Pair_t & pr);
void            BatchErrorEstimation(Ckt_Gate_Net_t & ckt, Ckt_Gate_Net_t & cktRef);
Ckt_Gate_t *     CheckExpansion     (std::list <Ckt_Gate_t *> & cut);
void            Expand              (Ckt_Gate_t & cktObj, std::list <Ckt_Gate_t *> & cut);
void            FindCut             (Ckt_Gate_Net_t & ckt, std::list <Ckt_Gate_t *> & cut, Ckt_Gate_t & cktSrcObj);
void            BuildSubNtk         (std::vector <Ckt_Gate_t *> & pOrdObjs, std::list <Ckt_Gate_t *> & subNtk);
bool            HasSamePo           (Ckt_Gate_Net_t & ckt1, Ckt_Gate_Net_t & ckt2);
void            GetBooleanDifference(Ckt_Gate_Net_t & ckt, std::vector <Ckt_Gate_t *> & pOrdObjs, std::vector <uint64_t> & isPoICorrect, int fb);
void            GetAddedErrorRate   (Ckt_Gate_Net_t & ckt, std::vector <Ckt_Rpl_Pair_t> & pairs, int fb, uint64_t isCorrect);
void            GetValidPair        (Ckt_Gate_Net_t & ckt, std::vector <Ckt_Gate_t *> & pOrdObjs, std::vector < Ckt_Rpl_Pair_t > & pairs);
void            ReplaceTest         (Ckt_Gate_Net_t & ckt);



#endif
