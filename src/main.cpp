#include <bits/stdc++.h>
#include "cktSASIMI.h"
#include "cktSingSel.h"
#include "cmdline.h"


using namespace std;
using namespace abc;
using namespace cmdline;


parser Cmdline_Parser(int argc, char * argv[])
{
    parser option;
    option.add <string> ("file",   'f', "Circuit file",       false, "data/sop/c432.blif");
    option.add <string> ("genlib", 'g', "Map libarary file",  false, "data/genlib/mcnc.genlib");
    option.add <float>  ("error",  'e', "Error rate",         false, 0.05, range(0, 1));
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


void Execute_Sop_Net(Abc_Ntk_t * pAbcNtk, int number)
{
    Ckt_Sop_Net_t ckt(pAbcNtk, number);
    Ckt_Sop_Net_t cktRef(ckt);

    // cout << "brute" << endl;
    // Ckt_EnumerateTest(ckt);
    // cout << "batch" << endl;
    // Ckt_BatchErrorEstimation(ckt, cktRef);
    // ckt.CheckSimulator();
    ckt.TestSimulatorSpeed();
    Ckt_BuildCutNtks(ckt);
}


int main(int argc, char * argv[])
{
    parser option = Cmdline_Parser(argc, argv);
    string file = option.get <string> ("file");
    string genlib = option.get <string> ("genlib");
    int number = option.get <int> ("number");

    Abc_Start();
    Abc_Frame_t * pAbc = Abc_FrameGetGlobalFrame();
    string command = "read_genlib -v " + genlib;
    assert( Cmd_CommandExecute(pAbc, command.c_str()) == 0 );
    command = "read_blif " + file;
    assert( Cmd_CommandExecute(pAbc, command.c_str()) == 0 );

    Abc_Ntk_t * pAbcNtk = Abc_FrameReadNtk(pAbc);
    if (Abc_NtkIsSopLogic(pAbcNtk))
        Execute_Sop_Net(pAbcNtk, number);
    else if (Abc_NtkIsMappedLogic(pAbcNtk))
        Execute_Gate_Net(pAbcNtk, number);

    Abc_Stop();

    return 0;
}
