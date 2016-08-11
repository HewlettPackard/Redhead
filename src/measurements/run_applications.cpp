//
// Created by kuscum on 6/23/16.
//

#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>

//Applications

#include <src/applications/v2_SparseM.h>
#include <src/applications/jacobian.h>
#include <src/applications/gameOfLife.h>
#include <src/applications/sparseMatrixMultiplication.h>

//Includes
#include <include/portable_pointer/portable_pointer.h>
#include <include/portable_pointer/catalogue.h>
#include <include/gnuplot/gnuplot-iostream.h>

//Contains all directives
#include <config/settings.h>
#include <src/applications/access_example.h>

using namespace std;

void gnuplot_compareApplications(std::string filePath, std::string titleOfChart, std::string titleOfApplication1, std::string titleOfApplication2, double timestampApplication1, double timestampApplication2, std::string typeOfSeconds){
    std::string scriptFilePath = ">Script_" + filePath + ".gp";
    Gnuplot gp(scriptFilePath);
    filePath += ".data";
    // Data will be sent via a temporary file.  These are erased when you call
    // gp.clearTmpfiles() or when gp goes out of scope.  If you pass a filename
    // (e.g. "gp.file1d(pts, 'mydata.dat')"), then the named file will be created
    // and won't be deleted (this is useful when creating a script).

    double y_axes = 0;

    //Adjust the Y-Axes to the value of the highest bar
    if(timestampApplication2 <= timestampApplication1){
        y_axes = timestampApplication1 * 1.1;
    }else{
        y_axes = timestampApplication2 * 1.1;
    }

    std::vector<std::tuple<int, std::string, double> > bars;
    bars.push_back(std::make_tuple(0, titleOfApplication1, timestampApplication1));
    bars.push_back(std::make_tuple(1, titleOfApplication2, timestampApplication2));
    //bars.push_back(std::make_tuple(2, titleOfApplication3, timestampApplication3));

    //Labels
    gp << "set xlabel 'Applications'\n";
    gp << "set ylabel 'Time in "<< typeOfSeconds <<"'\n";
    gp << "set title '" << titleOfChart <<"'\n";

    gp << "set yrange [0:" << y_axes <<"]\n";
    gp << "set style line 1 lc rgb 'red' \n";
    gp << "set style line 2 lc rgb '#406090' \n";
    //gp << "set style line 3 lc rgb 'blue' \n";
    gp << "set boxwidth 0.5 \n";
    gp << "set style fill solid 1.0 border -1 \n";
    gp << "plot" << gp.file1d(bars, filePath) << "every ::0::0 using 1:3:xtic(2) title '~"<<  static_cast<int>(timestampApplication1) << " " << typeOfSeconds <<"' with boxes ls 1,"
                 << gp.file1d(bars, filePath) << "every ::1::1 using 1:3:xtic(2) title '~"<< static_cast<int>(timestampApplication2) << " " << typeOfSeconds << "' with boxes ls 2, \n";
                 //<< gp.file1d(bars) << "every ::2::2 using 1:3:xtic(2) title '"<< static_cast<int>(timestampApplication3) << " " << typeOfSeconds << "' with boxes ls 3;
};

//, std::vector<std::pair<double, double>> xy_pts,
void gnuplot_generateScaleUpGraph(std::string filePath, std::string titleOfChart, std::string titleOfApplication1, std::string typeOfSeconds){
    std::string scriptFilePath = ">Script_" + filePath + ".gp";
    Gnuplot gp(scriptFilePath);
    filePath += ".data";

    std::vector<std::pair<double, double>> xy_pts;
    for(double x=-2; x<2; x+=0.01) {
        double y = x*x*x;
        xy_pts.push_back(std::make_pair(x, y));
    }

    //Labels
    gp << "set xlabel 'Size of Data in GB'\n";
    gp << "set ylabel 'Time in "<< typeOfSeconds <<"'\n";
    gp << "set title '" << titleOfChart <<"'\n";

    gp << "set xrange [0:1000]\nset yrange [1:1000]\n";
    gp << "plot" << gp.file1d(xy_pts, filePath) << "with points title '"<<titleOfApplication1<<"'\n" << std::endl;
}

int main(int argc, char *argv[])
{
#pragma mark GET MEMORY
    //Creates 11 shelves and mmaps it to this process
    int amountOfShelves = 11;
    int* backupPtr[amountOfShelves];
    int** devshmPtr = cat<int>::createShelvesAndMMAP(amountOfShelves, SHELVESIZE, MMAP_DIR_DEVSHM);
    cat<int>::checkMemory(devshmPtr, amountOfShelves);

    //Backup memory pointer, so it can be retrieved once the memory changed
    for (int i = 0; i < amountOfShelves; ++i) {
        backupPtr[i] = devshmPtr[i];
    }
//    int** LFSPtr = cat<int>::createShelvesAndMMAP(amountOfShelves, SHELVESIZE, MMAP_DIR_LFS);
//    cat<int>::checkMemory(LFSPtr, amountOfShelves);



#pragma mark CALL APPLICATIONS
    int iterations = 100;
    //Input parameters of following applications:
    //Memory pointer, Length of X-Axes, Length of Y-Axes, Print result, Number of iterations

    std::string application;
    //jf = Jacobi Iteration
    //gf = GameOfLife
    //sm = SparseMatrix
    //ac = AccessTimes
    //jf_scale = Jacobi Iteration Scaling Up to 1TB

    if(argc > 2){
        application = argv[1];
        iterations = strtol(argv[2], NULL, 10);
    }


    if(application == "jf"){
        //Each cell in Jacobian is a double, ergo 8 byte.
        double jacobian2 = jacobian2d(devshmPtr[0], 2500, 2500, false, iterations); //500 MB
        std::cout << "Jacobian2D: " << jacobian2 << " milliseconds" << std::endl;

        double jacobian3 = jacobian3d(devshmPtr[0], 400, 400, 400, false, iterations); //+500 MB
        std::cout << "Jacobian3D: " << jacobian3 << " milliseconds" << std::endl;

        double jacTxHPC_measuredTime = jacobian2d_TxHPC((char**)devshmPtr, amountOfShelves, 4, 2500, 2500, iterations++);
        std::cout << "Jacobi 2D TxHPC: " << jacTxHPC_measuredTime <<  " milliseconds" << std::endl;

        gnuplot_compareApplications("Jacobian2D", "Jacobian2D on /Dev/SHM - Size: 500MB - Iterations: 100", "App_without_TxHPC", "App_with_TxHPC", jacobian2, jacTxHPC_measuredTime, "milliseconds");
        gnuplot_compareApplications("Jacobian3D", "Jacobian3D on /Dev/SHM - Size: ~500MB - Iterations: 100", "App_without_TxHPC", "App_with_TxHPC", jacobian3, 0, "milliseconds");
    }
    else if(application == "jf_IT"){
        std::cout<<"JF_SCALE STARTED." << std::endl;

        iterations = 100;
        //1GB = 11 180
        //10GB = 35356
        //100GB = 111 804
        //250GB = 176776
        //500GB = 250 000
        //1000TB = 353 553

        double jacTxHPC_measuredTime = jacobian2d_TxHPC((char**)devshmPtr, amountOfShelves, 200, 100, 100, iterations);
        std::cout << "Jacobi 2D TxHPC: " << jacTxHPC_measuredTime <<  " milliseconds" << std::endl;


         jacTxHPC_measuredTime = jacobian2d_TxHPC((char**)devshmPtr, amountOfShelves, 200, 100, 100, iterations);
        std::cout << "Jacobi 2D TxHPC: " << jacTxHPC_measuredTime <<  " milliseconds" << std::endl;

        gnuplot_compareApplications("Jacobi2D_Iteration_ScalingUP", "Jacobian2D on /Dev/SHM - Size: 500MB - Iterations: 100", "_", "App_with_TxHPC", 0, jacTxHPC_measuredTime, "milliseconds");
    }

    else if(application == "gf"){
        std::cout << "gf started with " << iterations << " Iterations." << std::endl;

        //Each cell in GameOfLife is a int, ergo 4 byte.
        //11200
       double gameOfLife1 = gameOfLife((char**)devshmPtr, 11200, 11200, false, iterations); // +500 MB
        std::cout << "GameOfLife: " << gameOfLife1 << " milliseconds " << std::endl;

//        for (int i = 0; i < amountOfShelves; ++i) {
//            devshmPtr[i] = backupPtr[i];
//        }

        //11200
        double gameOfLife_TxHPC_measured = gameOfLife_TxHPC((char**) devshmPtr, amountOfShelves, 20, 11200, 11200, iterations+2);
        std::cout << "GameOfLife 2D TxHPC: " << gameOfLife_TxHPC_measured <<  " milliseconds" << std::endl;

        gnuplot_compareApplications("GameOfLife", "GameOfLife on /Dev/SHM - Size: ~500MB - Iterations: 100", "App_without_TxHPC", "App_with_TxHPC", gameOfLife1, gameOfLife_TxHPC_measured, "milliseconds");
    }
    else if(application == "gf_IT"){
        std::cout << "gf with more computation started with " << iterations << " Iterations." << std::endl;
        //Each cell in GameOfLife is a int, ergo 4 byte.
        //11200
        double gameOfLife1 = gameOfLife_more_computation(devshmPtr[0], 11200, 11200, false, iterations); // +500 MB
        std::cout << "GameOfLife: " << gameOfLife1 << " milliseconds " << std::endl;

        for (int i = 0; i < amountOfShelves; ++i) {
            devshmPtr[i] = backupPtr[i];
        }

        //11200
        double gameOfLife_TxHPC_measured = gameOfLife_TxHPC_more_computation((char**) devshmPtr, amountOfShelves, 20, 11200, 11200, iterations+2);
        std::cout << "GameOfLife 2D TxHPC: " << gameOfLife_TxHPC_measured <<  " milliseconds" << std::endl;

        gnuplot_compareApplications("GameOfLife_MoreComputation", "GameOfLife with 100 times more computation on /Dev/SHM - Size: ~500MB - Iterations: 100", "App_without_TxHPC", "App_with_TxHPC", gameOfLife1, gameOfLife_TxHPC_measured, "milliseconds");
    }
    else if(application == "sm"){
        //Version from the internet //1000, 2500, 100
        //Compare this with the version in file 'sparseMatrixMultiplication'
        double SpMv2 = SpMV(devshmPtr, 100, 2500, 100, iterations); //microseconds
        std::cout << "SparseMatrixMultiplication internet style: " << SpMv2 <<  " microseconds" << std::endl;
        //The function sparseMatrixMultiplication(...) changes the devshmPtr[i]. No Idea why! - So retrieving backup
        for (int i = 0; i < amountOfShelves; ++i) {
            devshmPtr[i] = backupPtr[i];
        }

        double spM_TxHPC = sparseMatrix_TxHPC((char**) devshmPtr, amountOfShelves, 4, 2500, 2500, iterations);
        std::cout << "SparseMatrix TxHPC: " << spM_TxHPC <<  " microseconds" << std::endl;

        gnuplot_compareApplications("SparseMatrixMultiplication", "SparseMatrixMultiplicatoin V2 on /Dev/SHM - Size: ~500MB - Iterations: 100", "App_without_TxHPC", "App_with_TxHPC", SpMv2, spM_TxHPC, "microseconds");
    }
    else if(application == "ac"){
        std::cout << "AC started" << std::endl;

        double accessTimes_TxHPC = access_TxHPC((char**) devshmPtr, amountOfShelves, 40, 11180, 11180, iterations+2);
        std::cout << "AccessTime: " << accessTimes_TxHPC <<  " milliseconds" << std::endl;

        double accessTime_withoutTxHPC = access_WithOutTxHPC ((char**) devshmPtr, 11180, 11180, iterations);
        std::cout << "AccessTime: " << accessTime_withoutTxHPC <<  " milliseconds" << std::endl;
        gnuplot_compareApplications("MemoryBandwidth", "Memory Bandwidth on /Dev/SHM - Size: ~500MB - Iterations: 1000", "App_without_TxHPC", "App_with_TxHPC", accessTime_withoutTxHPC, accessTimes_TxHPC, "microseconds");
    }

/*
    double spM1 =  sparseMatrixMultiplication(devshmPtr[0], 2500, 2500, false, iterations); // 500MB
    std::cout << "SparseMatrixMultiplication in StencilForTxHPC style: " << spM1 << std::endl;
    //Retrieve Backup

*/
    return 0;
};
