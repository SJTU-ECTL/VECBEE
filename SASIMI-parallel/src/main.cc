#include "headers.h"
#include "cmdline.h"
#include "sasimi.h"


using namespace std;
using namespace cmdline;


parser Cmdline_Parser(int argc, char * argv[])
{
    parser option;
    option.add <string> ("input", 'i', "original circuit file", false, "data/su/c880.blif");
    option.add <string> ("library", 'l', "standard cell library", false, "data/genlib/mcnc.genlib");
    option.add <string> ("metricType", '\0', "error metric type, er, aem", false, "er");
    option.add <float>  ("errorBound", 'e', "error metric upper bound", false, 0.05, range(0.0, 1.0));
    option.add <int>    ("frameNumber", 'f', "simulation round", false, 100000, range(1, INT_MAX));
    option.parse_check(argc, argv);
    return option;
}


void RenameNtk(Abc_Ntk_t * pNtk, string & fileName)
{
    uint32_t pos0 = fileName.find(".blif");
    DASSERT(pos0 != fileName.npos);
    uint32_t pos1 = fileName.rfind("/");
    if (pos1 == fileName.npos)
        pos1 = -1;
    Ckt_NtkRename(pNtk, fileName.substr(pos1 + 1, pos0 - pos1 - 1).c_str());
}


int main(int argc, char * argv[])
{
    // command line parser
    parser option = Cmdline_Parser(argc, argv);
    string input = option.get <string> ("input");
    string library = option.get <string> ("library");
    Metric_t metricType = (option.get <string> ("metricType") == "er")? Metric_t::ER: Metric_t::NMED;
    float errorBound = option.get <float> ("errorBound");
    int frameNumber = option.get <int> ("frameNumber");

    // start abc
    Abc_Start();

    // read library and circuit
    Abc_Frame_t * pAbc = Abc_FrameGetGlobalFrame();
    ostringstream command("");
    command << "read data/library/mcnc.genlib";
    DASSERT(!Cmd_CommandExecute(pAbc, command.str().c_str()));
    command.str("");
    command << "read_blif " << input;
    DASSERT(!Cmd_CommandExecute(pAbc, command.str().c_str()));
    command.str("");
    command << "print_stats";
    DASSERT(!Cmd_CommandExecute(pAbc, command.str().c_str()));

    // sasimi + vecbee
    Abc_Ntk_t * pNtk = Abc_FrameReadNtk(pAbc);
    SASIMI_Manager_t sasimiMng(pNtk, frameNumber, metricType, errorBound);
    sasimiMng.GreedySelection(pNtk);

    // stop abc
    Abc_Stop();

    return 0;
}
