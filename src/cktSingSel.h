#ifndef CKT_SING_SEL_H
#define CKT_SING_SEL_H


#include "cktSopNet.h"


class Ckt_Sing_Sel_ALC_t
{
public:
    Ckt_Sop_t * pCktObj;
    std::vector <std::string> SOP;
    Ckt_Sop_Cat_t type;
    int addedER;

    Ckt_Sing_Sel_ALC_t(Ckt_Sop_t * p_ckt_obj, std::vector <std::string> _sop, Ckt_Sop_Cat_t _type);
    Ckt_Sing_Sel_ALC_t(const Ckt_Sing_Sel_ALC_t & other);
    ~Ckt_Sing_Sel_ALC_t(void);
};


std::ostream &  operator <<             (std::ostream & os, const std::vector <std::string> & SOP);
std::ostream &  operator <<             (std::ostream & os, const Ckt_Sing_Sel_ALC_t & ALC);
void            Ckt_BatchErrorEstimation(Ckt_Sop_Net_t & ckt, Ckt_Sop_Net_t & cktRef);
Ckt_Sop_t *     Ckt_CheckExpansion      (std::list <Ckt_Sop_t *> & cut);
void            Ckt_Expand              (Ckt_Sop_t & cktObj, std::list <Ckt_Sop_t *> & cut);
void            Ckt_FindCut             (Ckt_Sop_Net_t & ckt, std::list <Ckt_Sop_t *> & cut, Ckt_Sop_t & cktSrcObj);
void            Ckt_BuildSubNtk         (std::vector <Ckt_Sop_t *> & pOrdObjs, std::list <Ckt_Sop_t *> & subNtk);
bool            Ckt_HasSamePo           (Ckt_Sop_Net_t & ckt1, Ckt_Sop_Net_t & ckt2);
void            Ckt_GetBooleanDifference(Ckt_Sop_Net_t & ckt, std::vector <Ckt_Sop_t *> & pOrdObjs, std::vector <uint64_t> & isPoICorrect, int fb);
void            Ckt_GetAddedErrorRate   (std::vector <Ckt_Sing_Sel_ALC_t> & ALCs, int fb, uint64_t isCorrect);
void            Ckt_GetALCs             (Ckt_Sop_Net_t & ckt, std::vector <Ckt_Sop_t *> & pOrdObjs, std::vector < Ckt_Sing_Sel_ALC_t > & ALCs, int maxLiteralNum = 5);
void            Ckt_GetALCsRecur        (Ckt_Sop_t * pCktObj, std::vector < Ckt_Sing_Sel_ALC_t > & ALCs, int i, int j, int n, int maxLiteralNum = 5);
void            Ckt_EnumerateTest       (Ckt_Sop_Net_t & ckt);


#endif
