#ifndef SASIMI_H
#define SASIMI_H


#include "simulator.h"


class SASIMI_Manager_t{
private:
    int nFrame;
    Metric_t metricType;
    double errorBound;

    SASIMI_Manager_t & operator = (const SASIMI_Manager_t &);
    SASIMI_Manager_t(const SASIMI_Manager_t &);

public:
    explicit SASIMI_Manager_t(Abc_Ntk_t * pNtk, int nFrame, Metric_t metricType, double errorBound);
    ~SASIMI_Manager_t();
    void GreedySelection(Abc_Ntk_t * pOriNtk);
    void PatchConst(Abc_Ntk_t * pNtk);
    void CollectMFFC(IN Abc_Ntk_t * pAppNtk, OUT std::vector <Vec_Ptr_t *> & vMffcs);
    void FreeMFFC(std::vector <Vec_Ptr_t *> & vMffcs);
    void GetCPM(IN Simulator_t & oriSmlt, IN Simulator_t & appSmlt, OUT std::vector < std::vector <tVec> > & bds);
    void CollectAllLACs(IN Simulator_t & oriSmlt, IN Simulator_t & appSmlt, IN std::vector < std::vector <tVec> > & bds, IN std::vector <Vec_Ptr_t * > & vMffcs);
    void CollectNodeLAC(IN Abc_Obj_t * pObj, IN Simulator_t & appSmlt, IN std::vector <tVec> & isERInc, IN std::vector <tVec> & isERDec, IN std::vector <tVec> & sources, IN std::vector <Vec_Ptr_t * > & vMffcs, IN int baseER);
    void GetDError(IN Simulator_t & appSmlt, IN Abc_Obj_t * pTS, IN Abc_Obj_t * pSS, IN std::vector <tVec> & isERInc, IN std::vector <tVec> & isERDec, OUT std::pair<int, int> & errors);
    double GetDArea(Abc_Obj_t * pTS, Abc_Obj_t * pSS, std::vector <Vec_Ptr_t *> & vMffcs);
};


class LAC_t {
private:
    Abc_Obj_t * pObj;
    double FOM;
    double dArea;

    LAC_t & operator = (const LAC_t &);
    LAC_t(const LAC_t &);

public:
    explicit LAC_t();
    ~LAC_t();
};


extern "C" {
    void Abc_NodeMffcConeSuppPrint(Abc_Obj_t * pNode);
}


#endif
