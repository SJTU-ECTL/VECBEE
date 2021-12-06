#include "headers.h"
#include "cmdline.h"
#include "sasimi.h"


using namespace std;
using namespace std::filesystem;
using namespace cmdline;


parser Cmdline_Parser(int argc, char * argv[])
{
    parser option;
    option.add <string> ("input", 'i', "original circuit file", false, "data/su/c880.blif");
    option.add <string> ("output", 'o', "path to output circuit files", false, "appNtk/");
    option.add <string> ("library", 'l', "standard cell library", false, "data/library/mcnc.genlib");
    option.add <string> ("metricType", '\0', "error metric type, er, nmed", false, "er");
    option.add <float>  ("errorBound", 'e', "error metric upper bound", false, 0.05, range(0.0, 1.0));
    option.add <int>    ("frameNumber", 'f', "simulation round", false, 100000, range(1, INT_MAX));
    option.add <int>    ("maxLevel", '\0', "max TFO cut level", false, INT_MAX, range(1, INT_MAX));
    option.parse_check(argc, argv);
    return option;
}


int main(int argc, char * argv[])
{
    // command line parser
    parser option = Cmdline_Parser(argc, argv);
    string input = option.get <string> ("input");
    string output = option.get <string> ("output");
    string library = option.get <string> ("library");
    Metric_t metricType = (option.get <string> ("metricType") == "er")? Metric_t::ER: Metric_t::NMED;
    float errorBound = option.get <float> ("errorBound");
    int frameNumber = option.get <int> ("frameNumber");
    int maxLevel = option.get <int> ("maxLevel");

    // deal with IO
    path inputPath(input);
    DASSERT(exists(inputPath));
    path outputPath(output);
    create_directory(outputPath);
    path outPrefix(outputPath);
    outPrefix += inputPath.stem().string();

    // start abc
    Abc_Start();

    // read library and circuit
    Abc_Frame_t * pAbc = Abc_FrameGetGlobalFrame();
    ostringstream command("");
    command << "read " << library;
    cout << "abc command " << command.str() << endl;
    DASSERT(!Cmd_CommandExecute(pAbc, command.str().c_str()));
    command.str("");
    command << "read_blif " << inputPath.string();
    cout << "abc command " << command.str() << endl;
    DASSERT(!Cmd_CommandExecute(pAbc, command.str().c_str()));
    command.str("");
    command << "print_stats";
    cout << "abc command " << command.str() << endl;
    DASSERT(!Cmd_CommandExecute(pAbc, command.str().c_str()));

    // sasimi + vecbee
    Abc_Ntk_t * pNtk = Abc_FrameReadNtk(pAbc);
    SASIMI_Manager_t sasimiMng(pNtk, frameNumber, maxLevel, metricType, errorBound);
    sasimiMng.GreedySelection(pNtk, outPrefix.string());

    // stop abc
    Abc_Stop();

    return 0;
}
