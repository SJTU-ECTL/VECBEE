#ifndef SIMULATOR_H
#define SIMULATOR_H


#include "headers.h"
#include "abcApi.h"
#include "cktBit.h"
#include "cktUtil.h"
#include "debugAssert.h"


enum class Metric_t{ER, NMED, MRED};
enum class Map_t{SCL, LUT};


typedef std::vector <uint64_t> tVec;


class Simulator_t
{
private:
    Abc_Ntk_t * pNtk;
    int nFrame;
    int nBlock;
    int nLastBlock;
    int maxId;
    std::vector <int> topoIds;
    std::vector <int> level;
    tVec isInFoCone;
    std::vector <double> flow;
    std::vector <tVec> values;
    std::vector <tVec> tmpValues;
    std::vector <tVec> sinks;
    std::vector < std::list <Abc_Obj_t *> > djCuts;
    std::vector < std::vector <Abc_Obj_t *> > oneCuts;
    std::vector < std::vector <Abc_Obj_t *> > tfoCuts;
    std::vector < std::vector <Abc_Obj_t *> > cutNtks;
    std::vector < std::vector <tVec> > bdCuts;
    std::vector < std::vector <tVec> > bdOneCuts;

    Simulator_t & operator = (const Simulator_t &);
    Simulator_t(const Simulator_t &);

public:
    explicit Simulator_t(Abc_Ntk_t * pNtk, int nFrame = 64);
    ~Simulator_t();
    void Input(unsigned seed = 314);
    void Input(std::string fileName);
    void Simulate();
    void SimulateCutNtks();
    void SimulateOneCutNtks();
    void SimulateResub(Abc_Obj_t * pOldObj, void * pResubFunc, Vec_Ptr_t * vResubFanins);
    void SimulateSASIMI(Abc_Obj_t * pTS, Abc_Obj_t * pSS, bool isInv);
    void UpdateAigNode(Abc_Obj_t * pObj);
    void UpdateSopNode(Abc_Obj_t * pObj);
    void UpdateMapNode(Abc_Obj_t * pObj, bool isTmpValue = false);
    void UpdateAigObjCutNtk(Abc_Obj_t * pObj);
    void UpdateSopObjCutNtk(Abc_Obj_t * pObj);
    void UpdateMapObjCutNtk(Abc_Obj_t * pObj);
    void UpdateAigNodeResub(Abc_Obj_t * pObj, Hop_Obj_t * pResubFunc = nullptr, Vec_Ptr_t * vResubFanins = nullptr);
    void UpdateAigNodeResub(IN Abc_Obj_t * pObj, IN Hop_Obj_t * pResubFunc, IN Vec_Ptr_t * vResubFanins, OUT tVec & outValue);
    void UpdateSopNodeResub(Abc_Obj_t * pObj, char * pResubFunc = nullptr, Vec_Ptr_t * vResubFanins = nullptr);
    void UpdateSopNodeResub(Abc_Obj_t * pObj, char * pResubFunc, Vec_Ptr_t * vResubFanins, tVec & outValues);
    boost::multiprecision::int256_t GetInput(int lsb, int msb, int frameId = 0) const;
    boost::multiprecision::int256_t GetOutput(int lsb, int msb, int frameId = 0, bool isTmpValue = false) const;
    int64_t GetOutputFast(int blockId, int bitId) const;
    void PrintInputStream(int frameId = 0, bool isReverse = false) const;
    void PrintOutputStream(int frameId = 0, bool isReverse = false) const;
    void BuildCutNtks();
    void BuildAppCutNtks();
    void BuildOneCutNtks(int maxLevel);
    void UpdateFoConeAndLevel(Abc_Obj_t * pPivot);
    void FindOneCut(Abc_Obj_t * pPivot, int poId, std::set <Abc_Obj_t *> & cutNtkNodes, int maxLevel);
    void FindOneCut_rec(Abc_Obj_t * pNode, Abc_Obj_t * pPivot, int poId, std::set <Abc_Obj_t *> & cutNtkNodes, int maxLevel);
    void SortCutNtkNodes(Abc_Obj_t * pPivot, std::set <Abc_Obj_t *> & cutNtkNodes);
    void FindDisjointCut(Abc_Obj_t * pObj, std::list <Abc_Obj_t *> & djCut);
    void ExpandCut(Abc_Obj_t * pObj, std::list <Abc_Obj_t *> & djCut);
    Abc_Obj_t * ExpandWhich(std::list <Abc_Obj_t *> & djCut);
    void UpdateBoolDiff(IN Abc_Obj_t * pPo, IN Vec_Ptr_t * vNodes, INOUT std::vector <tVec> & bdPo);
    void UpdateBoolDiff(IN Vec_Ptr_t * vNodes, INOUT std::vector <tVec> & bds);
    void UpdateBoolDiffOneCut(IN int poId, IN Vec_Ptr_t * vNodes, INOUT std::vector <tVec> & bdPo);

    inline Abc_Ntk_t * GetNetwork() const {return pNtk;}
    inline int GetFrameNum() const {return nFrame;}
    inline int GetBlockNum() const {return nBlock;}
    inline int GetLastBlockLen() const {return nLastBlock;}
    inline int GetMaxId() const {return maxId;}
    inline int UpdateMaxId() {Abc_Obj_t * pObj; int i = 0; maxId = -1; Abc_NtkForEachObj(pNtk, pObj, i) maxId = std::max(maxId, pObj->Id); return maxId; }
    inline std::vector <tVec> * GetPValues() {return &values;}
    inline std::vector <tVec> * GetPTmpValues() {return &tmpValues;}
    inline void SetValues(Abc_Obj_t * pObj, int blockId, uint64_t value) {values[pObj->Id][blockId] = value;}
    inline void SetTmpValues(Abc_Obj_t * pObj, int blockId, uint64_t value) {tmpValues[pObj->Id][blockId] = value;}
    inline uint64_t GetValues(Abc_Obj_t * pObj, int blockId) const {return values[pObj->Id][blockId];}
    inline uint64_t GetTmpValues(Abc_Obj_t * pObj, int blockId) const {return tmpValues[pObj->Id][blockId];}
    inline bool GetValue(Abc_Obj_t * pObj, int blockId, int bitId) const {return Ckt_GetBit(values[pObj->Id][blockId], bitId);}
    inline bool GetTmpValue(Abc_Obj_t * pObj, int blockId, int bitId) const {return Ckt_GetBit(tmpValues[pObj->Id][blockId], bitId);}
    inline std::vector < std::list <Abc_Obj_t *> > * GetDjCuts() {return &djCuts;}
    inline uint64_t GetBdCut(Abc_Obj_t * pObj, int cutId, int blockId) const {return bdCuts[pObj->Id][cutId][blockId];}
};


double MeasureNMED(Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtk2, int nFrame, unsigned seed, bool isCheck = true);
double MeasureMRED(Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtk2, int nFrame, unsigned seed, bool isCheck = true);
double MeasureResubNMED(Simulator_t * pSmlt1, Simulator_t * pSmlt2, Abc_Obj_t * pOldObj, void * pResubFunc, Vec_Ptr_t * vResubFanins, bool isCheck = true);
double GetNMED(Simulator_t * pSmlt1, Simulator_t * pSmlt2, bool isCheck = true, bool isTmpValue = false);
int64_t GetNMEDFast(Simulator_t * pSmlt1, Simulator_t * pSmlt2, bool isCheck = true);
int64_t GetNMEDFast(std::vector <int64_t> & oriOutputs, std::vector <int64_t> & appOutputs);
void GetOffset(IN Simulator_t * pSmlt1, IN Simulator_t * pSmlt2, IN bool isCheck, INOUT std::vector < std::vector <int8_t> > & offsets);
double GetNMEDFromOffset(IN std::vector < std::vector <int8_t> > & offsets);
double MeasureER(Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtk2, int nFrame = 102400, unsigned seed = 314, bool isCheck = true);
double MeasureResubER(Simulator_t * pSmlt1, Simulator_t * pSmlt2, Abc_Obj_t * pOldObj, void * pResubFunc, Vec_Ptr_t * vResubFanins, bool isCheck = true);
double MeasureSASIMIER(Simulator_t * pSmlt1, Simulator_t * pSmlt2, Abc_Obj_t * pTS, Abc_Obj_t * pSS, bool isInv, bool isCheck = true);
double MeasureSASIMINMED(Simulator_t * pSmlt1, Simulator_t * pSmlt2, Abc_Obj_t * pTS, Abc_Obj_t * pSS, bool isInv, bool isCheck = true);
int GetER(Simulator_t * pSmlt1, Simulator_t * pSmlt2, bool isCheck = true, bool isTmpValue = false);
bool IOChecker(Abc_Ntk_t * pNtk1, Abc_Ntk_t * pNtk2);
bool SmltChecker(Simulator_t * pSmlt1, Simulator_t * pSmlt2);
Vec_Ptr_t * Ckt_NtkDfsResub(Abc_Ntk_t * pNtk, Abc_Obj_t * pObjOld, Vec_Ptr_t * vFanins);
void Ckt_NtkDfsResub_rec(Abc_Obj_t * pNode, Vec_Ptr_t * vNodes, Abc_Obj_t * pObjOld, Vec_Ptr_t * vFanins);

#endif
