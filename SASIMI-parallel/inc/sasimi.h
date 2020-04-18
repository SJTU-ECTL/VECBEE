#ifndef SASIMI_H
#define SASIMI_H


#include "simulator.h"


class LAC_t;


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
    void GreedySelection(Abc_Ntk_t * pOriNtk, std::string outPrefix);
    void PatchConst(Abc_Ntk_t * pNtk);
    void CollectMFFC(IN Simulator_t & appSmlt, OUT std::vector <Vec_Ptr_t *> & vMffcs);
    void FreeMFFC(std::vector <Vec_Ptr_t *> & vMffcs);
    void GetCPM(IN Simulator_t & oriSmlt, IN Simulator_t & appSmlt, OUT std::vector < std::vector <tVec> > & bds);
    void CollectAllLACs(IN Simulator_t & oriSmlt, IN Simulator_t & appSmlt, IN std::vector < std::vector <tVec> > & bds, IN std::vector <Vec_Ptr_t * > & vMffcs, OUT std::vector <LAC_t> & nodeLACs);
    void CollectNodeLAC(IN Abc_Obj_t * pObj, IN Simulator_t & appSmlt, IN std::vector <tVec> & isERInc, IN std::vector <tVec> & isERDec, IN std::vector <tVec> & sources, IN std::vector <Vec_Ptr_t * > & vMffcs, IN int baseER, OUT LAC_t & nodeLAC);
    void SortCandLACs(IN std::vector <LAC_t> & nodeLACs, IN int nFrame, OUT std::vector <LAC_t> & candLACs);
    int ApplyBestLAC(Simulator_t & oriSmlt, Simulator_t & appSmlt, std::vector <LAC_t> & candLACs, int topNum, int cntRound, std::string outPrefix);
    void GetDError(IN Simulator_t & appSmlt, IN Abc_Obj_t * pTS, IN Abc_Obj_t * pSS, IN std::vector <tVec> & isERInc, IN std::vector <tVec> & isERDec, OUT std::pair<int, int> & errors);
    double GetDArea(Abc_Obj_t * pTS, Abc_Obj_t * pSS, std::vector <Vec_Ptr_t *> & vMffcs);
};


class LAC_t {
private:
    Abc_Obj_t * pTS;
    Abc_Obj_t * pSS;
    bool isInv;
    double dError;
    double dArea;
    double FOM;

public:
    explicit LAC_t();
    ~LAC_t();
    void Init();
    void Update(Abc_Obj_t * pTS, Abc_Obj_t * pSS, bool isInv, double error, double dArea, double FOM);
    void Print(int nFrame) const;
    inline Abc_Obj_t * GetTS() const {return pTS;}
    inline Abc_Obj_t * GetSS() const {return pSS;}
    inline bool GetIsInv() const {return isInv;}
    inline double GetDError() const {return dError;}
    inline double GetDArea() const {return dArea;}
    inline void SetFOM(double FOM) {this->FOM = FOM;}
    inline double GetFOM() const {return FOM;}
    bool operator < (const LAC_t & other) const;
};


extern "C" {
    void Abc_NodeMffcConeSuppPrint(Abc_Obj_t * pNode);
}


#endif
