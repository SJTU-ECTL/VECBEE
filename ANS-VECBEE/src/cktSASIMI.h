#ifndef CKT_SASIMI_H
#define CKT_SASIMI_H


#include "cktGateNet.h"
#include "cktTiming.h"


class Ckt_SASIMI_Candi_t
{
public:
    Ckt_Gate_t * pTS;
    Ckt_Gate_t * pSS;
    int addedER;

    explicit Ckt_SASIMI_Candi_t(Ckt_Gate_t * p_ts, Ckt_Gate_t * p_ss);
    Ckt_SASIMI_Candi_t(const Ckt_SASIMI_Candi_t & other);
    ~Ckt_SASIMI_Candi_t(void);
};


std::ostream &  operator <<             (std::ostream & os, const Ckt_SASIMI_Candi_t & pr);
void            Ckt_BatchErrorEstimation(Ckt_Gate_Net_t & ckt, Ckt_Gate_Net_t & cktRef);
Ckt_Gate_t *    Ckt_CheckExpansion      (std::list <Ckt_Gate_t *> & cut);
void            Ckt_Expand              (Ckt_Gate_t & cktObj, std::list <Ckt_Gate_t *> & cut);
void            Ckt_FindCut             (Ckt_Gate_Net_t & ckt, std::list <Ckt_Gate_t *> & cut, Ckt_Gate_t & cktSrcObj);
void            Ckt_BuildSubNtk         (std::vector <Ckt_Gate_t *> & pOrdObjs, std::list <Ckt_Gate_t *> & subNtk);
bool            Ckt_HasSamePo           (Ckt_Gate_Net_t & ckt1, Ckt_Gate_Net_t & ckt2);
void            Ckt_GetBooleanDifference(Ckt_Gate_Net_t & ckt, std::vector <Ckt_Gate_t *> & pOrdObjs, std::vector <uint64_t> & isPoICorrect, int fb);
void            Ckt_GetAddedErrorRate   (std::vector <Ckt_SASIMI_Candi_t> & pairs, int fb, uint64_t isCorrect);
void            Ckt_GetALCs             (Ckt_Gate_Net_t & ckt, std::vector <Ckt_Gate_t *> & pOrdObjs, std::vector < Ckt_SASIMI_Candi_t > & pairs);
void            Ckt_EnumerateTest       (Ckt_Gate_Net_t & ckt);


#endif
