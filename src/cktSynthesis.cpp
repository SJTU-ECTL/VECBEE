#include "cktSynthesis.h"


using namespace std;
using namespace abc;


float Ckt_Synthesis(Ckt_Sop_Net_t & ckt)
{
    Abc_Frame_t * pAbc = Abc_FrameGetGlobalFrame();
    Abc_Ntk_t * pNtkNew = Abc_NtkDup(ckt.GetAbcNtk());

    Abc_FrameSetCurrentNetwork(pAbc, pNtkNew);

    float delay, area, areaOld, delayOld;
    char Command[1000];

    sprintf( Command, "strash; map;" );
    assert( !Cmd_CommandExecute(pAbc, Command) );
    areaOld = GetArea(pNtkNew);
    delayOld = Abc_GetArrivalTime(pNtkNew);
    cout << "area = "<< areaOld << " delay = " << delayOld << endl;
    while (1) {
        sprintf( Command, "strash; balance; rewrite -l; refactor -l; balance; map" );
        assert( !Cmd_CommandExecute(pAbc, Command) );
        delay = Abc_GetArrivalTime( pNtkNew);
        area = GetArea( pNtkNew );
        if (area >= areaOld)
            break;
        areaOld = area;
        delayOld = delay;
        cout << "rewrite -l, refactor -l : " << "area = " << areaOld << " delay = " << delayOld << endl;

        sprintf( Command, "strash; balance; rewrite -l; rewrite -lz; balance; map" );
        assert( !Cmd_CommandExecute(pAbc, Command) );
        delay = Abc_GetArrivalTime( pNtkNew);
        area = GetArea( pNtkNew );
        if (area >= areaOld)
            break;
        areaOld = area;
        delayOld = delay;
        cout << "rewrite -l, rewrite -lz : " << "area = " << areaOld << " delay = " << delayOld << endl;

        sprintf( Command, "strash; balance; refactor -lz; rewrite -lz; balance; map" );
        assert( !Cmd_CommandExecute(pAbc, Command) );
        delay = Abc_GetArrivalTime( pNtkNew);
        area = GetArea( pNtkNew );
        if (area >= areaOld)
            break;
        areaOld = area;
        delayOld = delay;
        cout << "refactor -lz, rewrite -lz : " << "area = " << areaOld << " delay = " << delayOld << endl;
    }

    cout << "area = "<< areaOld << " delay = " << delayOld << endl;
    // Abc_NtkDelete(pNtkNew);
    return areaOld;
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
