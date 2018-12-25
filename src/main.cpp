#include <bits/stdc++.h>
#include "cktNtk.h"
#include "cktVisual.h"
#include "cmdline.h"


using namespace std;
using namespace abc;
using namespace cmdline;


parser Ckt_Cmdline_Parser(int argc, char * argv[])
{
    parser option;
    option.add <string> ("file",   'f', "Circuit file",       false, "data/blif/c432.blif");
    option.add <string> ("genlib", 'g', "Map libarary file",  false, "data/genlib/mcnc.genlib");
    // option.add <string> ("input",  'i', "Input pattern file", false, "");
    option.add <float>  ("error",  'e', "Error rate",         false, 0.05, range(0, 1));
    option.add <int>    ("number",  'n', "Frame number",      false, 1024, range(1, INT_MAX));
    option.parse_check(argc, argv);
    return option;
}


int main(int argc, char * argv[])
{
    parser option = Ckt_Cmdline_Parser(argc, argv);

    string file = option.get <string> ("file");
    string genlib = option.get <string> ("genlib");
    int number = option.get <int> ("number");
    Abc_Start();
    Abc_Frame_t * pAbc = Abc_FrameGetGlobalFrame();
    string command = "read " + genlib;
    assert( Cmd_CommandExecute(pAbc, command.c_str()) == 0 );
    command = "read " + file;
    assert( Cmd_CommandExecute(pAbc, command.c_str()) == 0 );

    Ckt_Bit_Cnt_t table;

    Ckt_Ntk_t cktRef(Abc_FrameReadNtk(pAbc), number);
    cktRef.GenInputDist(314);
    cktRef.FeedForward();
    Visualize(cktRef.GetAbcNtk(), "test.dot");

    // Ckt_Ntk_t cktApp(Abc_FrameReadNtk(pAbc), number);
    // cktApp.GenInputDist(314);
    // cktApp.FeedForward();
    // cout << cktApp.GetErrorRate(cktRef, table) << endl;
    // clock_t st = clock();
    // cout << clock() - st << endl;
    // cktRef.SimulatorChecker();

    Abc_Stop();
    return 0;
}
