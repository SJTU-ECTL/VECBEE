#include <bits/stdc++.h>
#include "cmdline.h"
#include "cktSASIMI.h"
#include "cktSingSel.h"
#include "cktSynthesis.h"
#include "cktBlif.h"


using namespace std;
using namespace abc;
using namespace cmdline;


parser Cmdline_Parser(int argc, char * argv[])
{
    parser option;
    option.add <string> ("file",   'f', "Circuit file",       false, "data/sop/c432.blif");
    option.add <string> ("genlib", 'g', "Map libarary file",  false, "data/genlib/mcnc.genlib");
    option.add <float>  ("error",  'e', "Error rate",         false, 0.05f, range(0.0f, 1.0f));
    option.add <int>    ("number", 'n', "Frame number",       false, 1024, range(1, INT_MAX));
    option.parse_check(argc, argv);
    return option;
}


void Execute_Gate_Net(Abc_Ntk_t * pAbcNtk, int number)
{
    Ckt_Gate_Net_t ckt(pAbcNtk, number);
    Ckt_Gate_Net_t cktRef(ckt);

    // ckt.PrintInfo();
    cout << "brute" << endl;
    Ckt_EnumerateTest(ckt);
    cout << "batch" << endl;
    Ckt_BatchErrorEstimation(ckt, cktRef);
    // ckt.TestSimulatorSpeed();
}


void Execute_Sop_Net(Abc_Ntk_t * pAbcNtk, int number, float ERThres)
{
    Ckt_Sop_Net_t ckt(pAbcNtk, number);
    Ckt_Sop_Net_t cktRef(ckt);
    int EThres = static_cast <int> (ERThres * number);

    // cout << "#nodes = " << ckt.GetObjNum() - ckt.GetPiNum() - ckt.GetPoNum() << endl;
    // cout << "#PIs = " << ckt.GetPiNum() << endl;
    // cout << "#POs = " << ckt.GetPoNum() << endl;
    // float orgArea = Ckt_Synthesis2(ckt);
    // cout << "Original area = " << orgArea << endl;
    while (Ckt_SingleSelectionOnce(ckt, cktRef, EThres) <= EThres);

    assert(system("if [ ! -d approx ]; then mkdir approx; fi") != -1);
    string fileName("approx/");
    fileName += ckt.GetName();
    fileName += string("_");
    ckt.GenInputDist(314);
    ckt.FeedForward();
    cktRef.GenInputDist(314);
    cktRef.FeedForward();
    float er = ckt.GetErrorRate(cktRef) / static_cast <float> (number);
    stringstream str;
    str << er;
    fileName += str.str();
    fileName += string(".blif");
    cout << fileName << endl;
    Ckt_WriteBlif(ckt, fileName);

    float newArea = Ckt_Synthesis2(ckt, str.str());
    cout << "Final area = " << newArea << endl;
}


void Manual_Test(Abc_Ntk_t * pAbcNtk, int number)
{
    Ckt_Sop_Net_t ckt(pAbcNtk, number);
    Ckt_Sop_Net_t cktRef(ckt);
    ckt.GenInputDist(314);
    cktRef.GenInputDist(314);
    cktRef.FeedForward();
    vector <string> SOP;

    // cout << "Ref" << endl;
    // for (int i = 0; i < cktRef.GetPoNum(); ++i)
    //     cout << cktRef.GetPo(i)->GetName() << "\t" << Ckt_GetBit(cktRef.GetPo(i)->GetCluster(0), 63) << endl;

    SOP.clear();
    ckt.ReplaceByName(string("G284gat"), SOP, Ckt_Sop_Cat_t::CONST1);
    ckt.ReplaceByName(string("G348gat"), SOP, Ckt_Sop_Cat_t::CONST0);
    ckt.ReplaceByName(string("G400gat"), SOP, Ckt_Sop_Cat_t::CONST0);
    ckt.ReplaceByName(string("G529gat"), SOP, Ckt_Sop_Cat_t::CONST1);
    ckt.ReplaceByName(string("G526gat"), SOP, Ckt_Sop_Cat_t::CONST0);
    ckt.ReplaceByName(string("G527gat"), SOP, Ckt_Sop_Cat_t::CONST1);
    ckt.ReplaceByName(string("G528gat"), SOP, Ckt_Sop_Cat_t::CONST1);
    ckt.ReplaceByName(string("G432gat"), SOP, Ckt_Sop_Cat_t::CONST0);
    // Ckt_WriteBlif(ckt, "output.blif");

    // cout << "Bef" << endl;
    // ckt.FeedForward();
    // cout << ckt.GetErrorRate(cktRef) << endl;
    // for (int i = 0; i < ckt.GetPoNum(); ++i)
    //     cout << ckt.GetPo(i)->GetName() << "\t" << Ckt_GetBit(ckt.GetPo(i)->GetCluster(0), 63) << endl;

    cout << "brute" << endl;
    Ckt_EnumerateTest(ckt, cktRef);
    cout << "batch" << endl;
    Ckt_Sing_Sel_Candi_t bestASE;
    Ckt_BatchErrorEstimation(ckt, cktRef, bestASE);

    // ckt.ReplaceByName(string("G287gat"), SOP, Ckt_Sop_Cat_t::CONST0);
    // cout << "Aft" << endl;
    // ckt.FeedForward();
    // cout << ckt.GetErrorRate(cktRef) << endl;
    // for (int i = 0; i < ckt.GetPoNum(); ++i)
    //     cout << ckt.GetPo(i)->GetName() << "\t" << Ckt_GetBit(ckt.GetPo(i)->GetCluster(0), 63) << endl;
}


void Manual_Test2(Abc_Ntk_t * pAbcNtk, int number)
{
    // Ckt_Sop_Net_t ckt(pAbcNtk, number);
    // Ckt_Sop_Net_t cktRef(ckt);
    // Ckt_SingleSelectionOnce(ckt, cktRef);
    Ckt_Gate_Net_t ckt(pAbcNtk, number);
    ckt.TestSimulatorSpeed();
}


int main(int argc, char * argv[])
{
    parser option = Cmdline_Parser(argc, argv);
    string file = option.get <string> ("file");
    string genlib = option.get <string> ("genlib");
    int number = option.get <int> ("number");
    float ERThres = option.get <float> ("error");

    Abc_Start();
    Abc_Frame_t * pAbc = Abc_FrameGetGlobalFrame();
    string command = "read_genlib -v " + genlib;
    assert( Cmd_CommandExecute(pAbc, command.c_str()) == 0 );
    command = "read_blif " + file;
    assert( Cmd_CommandExecute(pAbc, command.c_str()) == 0 );

    Abc_Ntk_t * pAbcNtk = Abc_FrameReadNtk(pAbc);

    if (Abc_NtkIsSopLogic(pAbcNtk))
        Execute_Sop_Net(pAbcNtk, number, ERThres);
    else if (Abc_NtkIsMappedLogic(pAbcNtk))
        Execute_Gate_Net(pAbcNtk, number);
    // Manual_Test(pAbcNtk, number);
    // Manual_Test2(pAbcNtk, number);
    Abc_Stop();

    return 0;
}
