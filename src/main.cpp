#include <bits/stdc++.h>
#include "cktBEE.h"
#include "cmdline.h"
#include "cktSop.h"


using namespace std;
using namespace abc;
using namespace cmdline;


parser Cmdline_Parser(int argc, char * argv[])
{
    parser option;
    option.add <string> ("file",   'f', "Circuit file",       false, "data/gate/c432.blif");
    option.add <string> ("genlib", 'g', "Map libarary file",  false, "data/genlib/mcnc.genlib");
    option.add <float>  ("error",  'e', "Error rate",         false, 0.05, range(0, 1));
    option.add <int>    ("number", 'n', "Frame number",       false, 1024, range(1, INT_MAX));
    option.parse_check(argc, argv);
    return option;
}


void Execute_Gate_Net(parser & option)
{
    string file = option.get <string> ("file");
    string genlib = option.get <string> ("genlib");
    int number = option.get <int> ("number");

    Abc_Start();
    Abc_Frame_t * pAbc = Abc_FrameGetGlobalFrame();
    string command = "read_genlib -v " + genlib;
    assert( Cmd_CommandExecute(pAbc, command.c_str()) == 0 );
    command = "read " + file;
    assert( Cmd_CommandExecute(pAbc, command.c_str()) == 0 );

    Ckt_Gate_Net_t ckt(Abc_FrameReadNtk(pAbc), number);
    Ckt_Gate_Net_t cktRef(ckt);

    cout << "brute" << endl;
    ReplaceTest(ckt);
    // ckt.CheckSimulator();
    cout << "batch" << endl;
    BatchErrorEstimation(ckt, cktRef);

    Abc_Stop();
}


void Execute_Sop_Net(parser & option)
{
    string file = option.get <string> ("file");
    string genlib = option.get <string> ("genlib");
    int number = option.get <int> ("number");

    Abc_Start();
    Abc_Frame_t * pAbc = Abc_FrameGetGlobalFrame();
    string command = "read_genlib -v " + genlib;
    assert( Cmd_CommandExecute(pAbc, command.c_str()) == 0 );
    command = "read " + file;
    assert( Cmd_CommandExecute(pAbc, command.c_str()) == 0 );

    Ckt_Sop_Net_t ckt(Abc_FrameReadNtk(pAbc), number);
    Ckt_Sop_Net_t cktRef(ckt);

    ckt.PrintInfo();

    Abc_Stop();
}


int main(int argc, char * argv[])
{
    parser option = Cmdline_Parser(argc, argv);
    // Execute_Gate_Net(option);
    Execute_Sop_Net(option);

    return 0;
}
