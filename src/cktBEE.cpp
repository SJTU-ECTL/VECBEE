#include "cktBEE.h"


using namespace std;


void BatchErrorEstimation(Ckt_Ntk_t & ckt, Ckt_Ntk_t * cktRef)
{
    // get topological sequence
    vector <Ckt_Obj_t *> pOrderedObjs;
    ckt.SortObjects(pOrderedObjs);
}
