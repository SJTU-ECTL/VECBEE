#ifndef CKT_SING_SEL_H
#define CKT_SING_SEL_H


#include "cktSopNet.h"
#include "cktVisual.h"
#include "cktCec.h"


class Ckt_Sing_Sel_Candi_t
{
public:
    Ckt_Sop_t * pCktObj;
    std::vector <std::string> SOP;
    Ckt_Sop_Cat_t type;
    int addedER;
    int newER;
    float score;

    explicit Ckt_Sing_Sel_Candi_t(void);
    explicit Ckt_Sing_Sel_Candi_t(Ckt_Sop_t * p_ckt_obj, std::vector <std::string> _sop, Ckt_Sop_Cat_t _type);
    Ckt_Sing_Sel_Candi_t(const Ckt_Sing_Sel_Candi_t & other);
    ~Ckt_Sing_Sel_Candi_t(void);
};


std::ostream &  operator <<             (std::ostream & os, const Ckt_Sing_Sel_Candi_t & candis);
void            Ckt_BatchErrorEstimation(Ckt_Sop_Net_t & ckt, Ckt_Sop_Net_t & cktRef, Ckt_Sing_Sel_Candi_t & res);
Ckt_Sop_t *     Ckt_CheckExpansion      (std::list <Ckt_Sop_t *> & cut);
void            Ckt_ObjExpand           (Ckt_Sop_t & cktObj, std::list <Ckt_Sop_t *> & cut);
void            Ckt_CollectVisited      (std::vector <Ckt_Sop_t *> & pOrdObjs, std::list <Ckt_Sop_t *> & subNtk);
void            Ckt_ObjFindCut          (Ckt_Sop_t & cktSrcObj, std::list <Ckt_Sop_t *> & cut);
void            Ckt_BuildCutNtks        (Ckt_Sop_Net_t & ckt, std::vector <Ckt_Sop_t *> & pOrdObjs);
void            Ckt_SimCutNtk           (Ckt_Sop_Net_t & cutNtk);
Ckt_Sop_Net_t * Ckt_CreateNtkFrom       (Ckt_Sop_t & cktSrcObj, std::list <Ckt_Sop_t *> & subNtk, std::list <Ckt_Sop_t *> & cut);
bool            Ckt_HasSamePo           (Ckt_Sop_Net_t & ckt1, Ckt_Sop_Net_t & ckt2);
void            Ckt_GetBoolDiff(Ckt_Sop_Net_t & ckt, std::vector <Ckt_Sop_t *> & pOrdObjs, std::vector <uint64_t> & isPoICorrect, int fb);
void            Ckt_GetAddedErrorRate   (std::vector <Ckt_Sing_Sel_Candi_t> & candis, int fb, uint64_t isCorrect);
void            Ckt_GetALCs             (Ckt_Sop_Net_t & ckt, std::vector <Ckt_Sop_t *> & pOrdObjs, std::vector < Ckt_Sing_Sel_Candi_t > & candis, int maxLiteralNum = 5);
void            Ckt_GetALCsRecur        (Ckt_Sop_t * pCktObj, std::vector < Ckt_Sing_Sel_Candi_t > & candis, int i, int j, int n, int maxLiteralNum = 5);
void            Ckt_EnumerateTest       (Ckt_Sop_Net_t & ckt, Ckt_Sop_Net_t & cktRef);
float           Ckt_SingleSelectionOnce (Ckt_Sop_Net_t & ckt, Ckt_Sop_Net_t & cktRef);


#endif
