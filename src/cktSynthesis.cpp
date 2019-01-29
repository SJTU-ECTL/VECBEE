#include "cktSynthesis.h"


using namespace std;
using namespace abc;


float Ckt_Synthesis(Ckt_Sop_Net_t & ckt)
{
    string Command;

    Abc_Frame_t * pAbc = Abc_FrameGetGlobalFrame();
    Abc_FrameReplaceCurrentNetwork(pAbc, Abc_NtkDup(ckt.GetAbcNtk()));

    Command = string("strash; balance; rewrite -l; refactor -l; balance; rewrite -l; rewrite -lz; balance; refactor -lz; rewrite -lz; balance;");
    for (int i = 0; i < 10; ++i)
        assert( !Cmd_CommandExecute(pAbc, Command.c_str()) );
    Command = string("map -a;");
    assert( !Cmd_CommandExecute(pAbc, Command.c_str()) );
    float area = GetArea(Abc_FrameReadNtk(pAbc));
    float delay = Abc_GetArrivalTime(Abc_FrameReadNtk(pAbc));
    cout << "area = " << area << " delay = " << delay << endl;
    return area;
}


float GetArea(Abc_Ntk_t * pNtk)
{
    assert(Abc_NtkHasMapping(pNtk));
    Vec_Ptr_t * vNodes = Abc_NtkDfs(pNtk, 0);
    float area = 0.0;
    Abc_Obj_t * pObj;
    int i;
    Vec_PtrForEachEntry(Abc_Obj_t *, vNodes, pObj, i)
        area += Mio_GateReadArea( (Mio_Gate_t *)pObj->pData );
    Vec_PtrFree(vNodes);
    return area;
}
