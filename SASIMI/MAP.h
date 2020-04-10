//
// Created by elab on 18-4-17.
//

#ifndef MAP_MAP_H
#define MAP_MAP_H

#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <malloc.h>
#include <time.h>
#include <vector>
#include <queue>
#include <string>
#include <list>
#include <algorithm>
#include <map>
#include <cassert>
#include <sstream>
#include <random> // Chang modify, use random device

using namespace std;
#define ERROR 0.05
// #define AEMRate 0.0019584
// #define AEMRate 0.0009792
// #define AEMRate 0.0002448
// #define AEMRate 0.0001224
// #define AEMRate 0.0000612
// #define AEMRate 0.0000306
// #define AEMRate 0.0000153
#define AEMFlag 0
#define SimValue 100000
#define TYPE vector<node>
#define ITER vector<vector<node>::iterator>
#define random(x) (rand()%x)
#define Pointer vector<node>::iterator
#define inlong 5
#define lambda 1
#define FLOOR 1
#define HIGESTORDER 65 //higest order that can be changed, if AT changes output more than it, this AT will be forbidden, just for AEM.
#define Binomal_95 1.96 // 95% confidence for binomal section

enum NODEGATE {gno,ginv,gnand,gnor,\
gand,gor,gxor,gxnor,gaoi21,gaoi22,goai21,goai22,gbuf,gzero,gone};
enum NODETYPE {in,out,intern};




typedef struct Node{
    string var;
    NODETYPE type;
    string gatename;
    NODEGATE g;
    double area;
    vector<int> out;
    vector<int> in;
    vector<int> mffc;
    unsigned long *V1; //CPM
    unsigned long *V2;
    unsigned long *V3;
    double error;
    double reducearea;
    double P; //P=reducerea/error
    bool I[SimValue];
    int sub;
    bool *W;
    unsigned long inloc[inlong];  //locate the node's input node
    double delay;
    bool inv;
    double gatedelay;
    double inverterror;
    bool *temp;
    unsigned long QNL;  // location in the QN
    //double delayslack;
}node;

typedef struct LOCATION{
    int loc;
    double P;
    double reducearea;
}LOC;

static bool SortTN(const LOC &v1, const LOC &v2);

typedef struct OutSort{
    int x;
    unsigned long loc;
}OUTSORT;

static bool SortOut(const OUTSORT &v1, const OUTSORT &v2);

typedef struct Path{
    int n;  //number of this node
    int h;  //height of this node
}PATH;


static inline void ComputeBooleanDifference(bool *BD, const int x, const TYPE& N);

class MAP{
    string name;
    int del,sub;  //choose the delete node ande substitute node
    bool Omax;
    bool O2max;
public:
    int I,O,A;
    long AEMThreshold;
    double LastAEM;
    TYPE N;  //the N[0] is zero,N[1] is one
    vector<int> QN;
    double MaxDelay;
    double MaxArea;
    int GateNum;
    vector<LOC> TN;
    vector< vector<bool> > TI; //True value of output
    vector<OUTSORT> SN;
    bool ErrFlag[SimValue];
    clock_t matrixTime;
    clock_t errorTime;
    clock_t areaTime;
    clock_t findTime;
    int matrixnode;
    int errornode;
    unsigned long *TrueValue;
    unsigned long *AEMTempValue;
    MAP();
    ~MAP();
    void InputFile(const char*  f);
    void InputNode(string s,NODETYPE d);
    void GateNode(string s);
    int FindNode(const string var);
    void OutputFile(const char*  f="/a.aag");
    void RandomInput(unsigned seed); // Chang modify, add seed
    void RandomInputBoost(unsigned seed);
    void PrintInputPattern(const char *f);
    void PrintOutputPattern(void);
    void ReadInputPattern(const char *f);
    void SimulateNode(void);
    double ReduceArea(const int l);
    double MeasureArea(const int x,const int sub);
    void MeasureTwoVote(const int x);
    double Measure(int x,double nowerror);
    double MeasureWithAEM(int x, double nowerror);
    int FindDeleteNode(double nowerror);
    void EvaluateTI(void);  //push the accurate value of output node into TI
    void CollectTI(MAP & map0); // Chang modify, collect TI from map0
    //void SimulateApproNode(const int x);
    void DeleteNode(const int x);
    double AddError(const int x,const int sub);
    double AEMAddError (const int x, const int sub);
    double AddErrorWithSASIMI(const int x, const int sub);
    double MeasureWithSASIMI(int x);
    double SimError(const int x);
    double AccurateError();
    void ComputeMFFC(void);
    void IterationMFFC(int src, int x, int * outDegree, bool choice);
    void ComputeDelayAndArea();
    void TopologicalSorting(void);
    void GatebufNode(string s);
    bool IterateArea(int sub,int x);
    void ReconvergeTwoPath(int x,map<int,int> &Hmap);
    void ComputeTempNodeValue(int x);
    void MeasureAllVote(const int x);
    void RecursiveFindOutput(const int x);
    void MeasureOneVote(const int x);
    void MeasureAdjustVote(const int x,const int up);
    void FindPathNode(const int x, const int up,int cur,map<int,PATH> &Hmap,bool F);  //Find the nodes in the up-floor of path of node x
    double AccurateMeasureError(const int x,const int sub);
    void OutputToMATLAB(const char*  f="/a.aag", const char * functionname = "f");  //Just for f and g module of polar circuit
    void ChangeNodeName(void);
    void EvaluateTrueValue(void);
    void CollectTrueValue(MAP & map0); // Chang modify, collect TrueValue from map0
    double AEMSimError(const int x);
    void SingleInputPattern(int val, int loc);
    void SetZeroOne(void);
    unsigned long ERComparedWithAccurate(MAP &map, const int num);  //Compute ER with original circuit, only useful to output smaller than 64
    unsigned long AEMComparedWithAccurate(MAP &map, const int num);  //Compute AEM with original circuit, only useful to output smaller than 64
    void MeasureAllVote_Meng(); //Compute CPM accurately by Meng Chang's method, compute all nodes's CPM at one time
    bool CheckJoint(vector< vector<unsigned long> >& isInFanoutCone, map<unsigned long, int>& disjointSet, unsigned long& jNode1, unsigned long& jNode2);
    void ResetInLoc(); //Reset the input variable set of each node for delete some nodes.
    void MeasueAdjustVote_Meng(const int up);
    bool CheckJointAdjust(vector< vector<unsigned long> >& isInFanoutCone, map<unsigned long, int>& disjointSet, unsigned long& jNode1, unsigned long& jNode2, const int up);
    double NewSimError(const int x, double pasterror);
};


#endif //MAP_MAP_H
