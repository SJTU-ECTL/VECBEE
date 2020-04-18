#include "MAP.h"

// procedures to start and stop the ABC framework
// (should be called before and after the ABC procedures are called)

#define TOPNUM 10


bool excelsystem(const char *cmdstring) {
    int status;
    if(NULL == cmdstring) //濡傛灉cmdstring涓虹┖瓒佹棭闂€€鍚э紝灏界system()鍑芥暟涔熻兘澶勭悊绌烘寚閽?
    {
        return false;
    }
    status = system(cmdstring);
    if(status < 0)
    {
        printf("cmd: %s\t ", cmdstring); // 杩欓噷鍔″繀瑕佹妸errno淇℃伅杈撳嚭鎴栬鍏og
        return false;
    }

    if(WIFEXITED(status))
    {
        //printf("normal termination, exit status = %d\n", WEXITSTATUS	(status)); //鍙栧緱cmdstring鎵ц缁撴灉
    }
    else if(WIFSIGNALED(status))
    {
        printf("abnormal termination,signal number =%d\n", WTERMSIG(status)); //濡傛灉cmdstring琚俊鍙蜂腑鏂紝鍙栧緱淇″彿鍊?
    }
    else if(WIFSTOPPED(status))
    {
        printf("process stopped, signal number =%d\n", WSTOPSIG(status)); //濡傛灉cmdstring琚俊鍙锋殏鍋滄墽琛岋紝鍙栧緱淇″彿鍊?
    }
    return true;
}

float AEMRate;
int cntRound = 0;

int main(int argc,char **argv) {
    MAP map0; // Chang modify, original circuit
    MAP map;
    char *target=argv[1];
    AEMRate=stof(argv[2]);
    //char target[50];
    //strcat(target,target2);
    //strcat(target,"_rem");
    char inputfile[50]="./blif_used/";
    strcat(inputfile,target);
    strcat(inputfile,".blif");
    map0.InputFile(inputfile); // Chang modify, initialize original circuit
    map.InputFile(inputfile);

    // map.RandomInput();
    // char printpattern[50]=("./in/");
    // strcat(printpattern,target);
    // strcat(printpattern,".in");
    // map.PrintInputPattern(printpattern);
    // return 0;

    //map.RandomInput();
    char printpattern[50]=("./in/");
    strcat(printpattern,target);
    strcat(printpattern,".in");
    map0.ReadInputPattern(printpattern); // Chang modify, initialize original circuit
    map.ReadInputPattern(printpattern);
    //map.PrintInputPattern(printpattern);

    if(!AEMFlag) {
        // map.SimulateNode();  //For error rate
        // map.EvaluateTI(); // Chang modify, get correct output at runtime
        double error = 0, preerror = 0;
        int del = -1;
        int i = 0, j = 0, circle = 0;
        double sumerror = 0;
        double SimulateArea = map.MaxArea;
        char outputfile[50] = "./record/test/";
        strcat(outputfile, target);
        strcat(outputfile, "/");
        // printf("Original Area:%lf, Original Delay:%lf, Orignial Gate :%d\n", map.MaxArea, map.MaxDelay,
        //        (int) map.QN.size());
        while (error <= ERROR) {
            cout << cntRound++ << endl;
            // random_device rd;
            // unsigned seed = static_cast <unsigned> (rd());
            // map0.RandomInput(seed);
            map0.SimulateNode();
            map0.EvaluateTI();
            // map.RandomInput(seed);
            map.SimulateNode();
            map.CollectTI(map0); // Chang modify, change input pattern and get correct output at runtime

            // if (cntRound == 6) {
            //     for(int i=map.I;i<map.I+map.O;i++){
            //         int j = 87267;
            //         cout << "Warning: " << map.N[i].var << "," << map0.N[i].I[j] << "," << map.N[i].I[j] << endl;
            //     }
            // }

            clock_t tempTime = clock();
            map.FindDeleteNode(error);
            map.findTime += (clock() - tempTime);
            //sumerror=error;
            if (map.TN.empty())
                break;

            // for (i = 0; i < map.TN.size(); i++) {
            //     auto & x = map.TN[i];
            //     auto & N = map.N;
            //     cout << "(" << N[x.loc].var << "," << N[N[x.loc].sub].var << "," << N[x.loc].inv << "," << N[x.loc].error << "," << x.reducearea << "," << x.P << ")" << endl;
            // }

            del = -1;
            for (i = 0; i < TOPNUM && i < map.TN.size(); i++) {
                del = map.TN[i].loc;
                if (del == -1 || map.N[del].sub == -1)
                    break;
                error = map.SimError(del);
                // auto & x = map.TN[i];
                // auto & N = map.N;
                // cout << "(" << N[x.loc].var << "," << N[N[x.loc].sub].var << "," << N[x.loc].inv << "," << N[x.loc].error << "," << x.reducearea << "," << x.P << ")" << endl;
                if (error <= ERROR)
                    break;
                else {
                    // printf("The %d choose is not good\n", i);
                    map.SimulateNode();
                }
            }
            if (del == -1 || map.N[del].sub == -1)
                break;
            sumerror += map.N[del].error;
            cout << "Change gate " << map.N[del].var;
            cout << " with gate" << map.N[map.N[del].sub].var;
            cout << "  , changed gate loc: " << del;
            cout << ",  sub gate loc: " << map.N[del].sub;
            cout << "Inverter: " << map.N[del].inv << endl;
            printf("Simulate error is %lf\n", sumerror);
            printf("Now Error is %lf\n", error);
            printf("Simulate added error is %lf, Now added error is %lf\n", map.N[del].error, error - preerror);
            if (error > ERROR)
                break;
            SimulateArea -= map.N[del].reducearea;
            map.DeleteNode(del);
            printf("Approximate gate:%d, Approximate area:%lf\n", map.GateNum, SimulateArea);
            map.TopologicalSorting();
            map.ComputeDelayAndArea();
            char out[100], outnum[40];
            circle++;
            sprintf(outnum, "%s_%d_%lf.blif", target, circle, error);
            strcpy(out, outputfile);
            strcat(out, outnum);
            map.OutputFile(out);

            //map.OutputFile(outputfile);
            //error=map.AccurateError();
            printf("Accurate error is %lf, Area :%lf, Delay: %lf, Gate: %d\n", error, map.MaxArea, map.MaxDelay,
                   (int) map.QN.size());
            printf("Now running time: %ld\n", clock() / CLOCKS_PER_SEC);
            preerror = error;
        }
        strcat(outputfile,target);
        strcat(outputfile,".blif");
        map.OutputFile(outputfile);
    }
    else {
        // map.SimulateNode();  //For average error magnitude
        // map.EvaluateTI();
        // map.EvaluateTrueValue(); // Chang modify, update TI and TrueValue at runtime
        double error = 0, preerror = 0;
        int del = -1;
        int i = 0, j = 0, circle = 0;
        double sumerror = 0;
        double SimulateArea = map.MaxArea;
        char outputfile[50] = "./record/Confi_AEM/";
        strcat(outputfile, target);
        strcat(outputfile, "/");
        printf("Original Area:%lf, Original Delay:%lf, Orignial Gate :%d\n", map.MaxArea, map.MaxDelay,
               (int) map.QN.size());
        while (error <= map.AEMThreshold) {
            random_device rd;
            unsigned seed = static_cast <unsigned> (rd());
            map0.RandomInput(seed);
            map0.SimulateNode();
            map0.EvaluateTI();
            map0.EvaluateTrueValue();
            map.RandomInput(seed);
            map.SimulateNode();
            map.CollectTI(map0);
            map.CollectTrueValue(map0); // Chang modify, change input pattern and get correct output at runtime
            clock_t tempTime = clock();
            map.FindDeleteNode(error);
            map.findTime += (clock() - tempTime);
            //sumerror=error;
            if (map.TN.empty())
                break;
            del = -1;
            for (i = 0; i < TOPNUM && i < map.TN.size(); i++) {
                del = map.TN[i].loc;
                if (del == -1 || map.N[del].sub == -1)
                    break;
                error = map.AEMSimError(del);
                if (error <= map.AEMThreshold)
                    break;
                else {
                    printf("The %d choose is not good\n", i);
                    map.SimulateNode();
                }
            }
            if (del == -1 || map.N[del].sub == -1)
                break;
            sumerror += map.N[del].error;
            cout << "Change gate " << map.N[del].var;
            cout << " with gate" << map.N[map.N[del].sub].var;
            cout << "  , changed gate loc: " << del;
            cout << ",  sub gate loc: " << map.N[del].sub;
            cout << "Inverter: " << map.N[del].inv << endl;
            printf("Simulate AEM is %lf\n", sumerror);
            printf("Now AEM is %lf\n", error);
            printf("Simulate added AEM is %lf, Now added AEM is %lf\n", map.N[del].error, error - preerror);
            if (error > map.AEMThreshold)
                break;
            SimulateArea -= map.N[del].reducearea;
            map.DeleteNode(del);
            printf("Approximate gate:%d, Approximate area:%lf\n", map.GateNum, SimulateArea);
            map.TopologicalSorting();
            map.ComputeDelayAndArea();
            char out[100], outnum[40];
            circle++;
            sprintf(outnum, "%s_%d_%lf.blif", target, circle, error);
            strcpy(out, outputfile);
            strcat(out, outnum);
            map.OutputFile(out);

            //map.OutputFile(outputfile);
            //error=map.AccurateError();
            printf("Accurate error is %lf, Area :%lf, Delay: %lf, Gate: %d\n", error, map.MaxArea, map.MaxDelay,
                   (int) map.QN.size());
            printf("Now running time: %ld\n", clock() / CLOCKS_PER_SEC);
            preerror = error;
        }
        cout << "AEM Threshold: " << map.AEMThreshold << " Now AEM: " << error << endl;
        strcat(outputfile,target);
        strcat(outputfile,".blif");
        map.OutputFile(outputfile);
    }



    printf("ALL running time: %ld\n",clock()/CLOCKS_PER_SEC);
    printf("Error propagation matrix time: %ld with %d node\n",map.matrixTime/CLOCKS_PER_SEC,map.matrixnode);
    printf("Error simulation time: %ld with %d node\n",map.errorTime/CLOCKS_PER_SEC,map.errornode);
    printf("Area time: %ld\n",map.areaTime/CLOCKS_PER_SEC);
    printf("Whole find time: %ld\n",map.findTime/CLOCKS_PER_SEC);


    return 0;
}
