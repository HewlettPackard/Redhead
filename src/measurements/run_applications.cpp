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

void gnuplot_compareApplications(std::string filePath, std::string titleOfChart, std::string titleOfApplication1, std::string titleOfApplication2, double timestampApplication1, double timestampApplication2, std::string typeOfSeconds, int iterations){
    std::string scriptFilePath = ">Script_" + filePath + "_" + to_string(iterations) + ".gp";
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
    int iterations = 1;
    long sizeOfDimension=0;
    //Input parameters of following applications:
    //Memory pointer, Length of X-Axes, Length of Y-Axes, Print result, Number of iterations

    std::string application;
    //jf = Jacobi Iteration
    //gf = GameOfLife
    //gf_computation = GameOfLife With More Computation
    //sm = SparseMatrix
    //ac = AccessTimes
    //jf_scale = Jacobi Iteration Scaling Up to 1TB
    std::string memoryType;

    if(argc > 2){
        application = argv[1];
        istringstream ss(argv[2]);
        if(!(ss >> iterations)){
            cerr << "Second argument needs to be a number. Invalid number: " << argv[2] << '\n';
        }
        //OPTIONAL PARAMETER
        //USED IF jf_scale is called
        if(argc > 3){
            istringstream ss2(argv[3]);
            if(!(ss >> sizeOfDimension)){
                cerr << "Second argument needs to be a number. Invalid number: " << argv[3] << '\n';
            }
        }

    }
    else{
        handle_error("You need to have at least 3 arguments.");
    }


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

//// Decomment this to use LFS
//    int** LFSPtr = cat<int>::createShelvesAndMMAP(amountOfShelves, SHELVESIZE, MMAP_DIR_LFS);
//    cat<int>::checkMemory(LFSPtr, amountOfShelves);


#pragma mark CALL APPLICATIONS

    if(application == "jf"){
        std::cout<<"JF STARTED." << std::endl;
        //Each cell in Jacobian is a double, ergo 8 byte.
        double jacobian2 = jacobian2d(devshmPtr[0], 2500, 2500, false, iterations); //500 MB
        std::cout << "Jacobian2D: " << jacobian2 << " milliseconds" << std::endl;

//        double jacobian3 = jacobian3d(devshmPtr[0], 400, 400, 400, false, iterations); //+500 MB
//        std::cout << "Jacobian3D: " << jacobian3 << " milliseconds" << std::endl;

        double jacTxHPC_measuredTime = jacobian2d_TxHPC((char**)devshmPtr, amountOfShelves, 4, 2500, 2500, iterations++);
        std::cout << "Jacobi 2D TxHPC: " << jacTxHPC_measuredTime <<  " milliseconds" << std::endl;

        std::string j2D("Jacobian2D on NVM - Size: 500MB - Iterations: ");
        j2D += to_string(iterations);

//        std::string j3D("Jacobian3D on /Dev/SHM - Size: ~500MB - Iterations:");
//        j3D += to_string(iterations);

        gnuplot_compareApplications("Jacobian2D", j2D.c_str(), "App_without_TxHPC", "App_with_TxHPC", jacobian2, jacTxHPC_measuredTime, "milliseconds", iterations);
        //gnuplot_compareApplications("Jacobian3D", j3D.c_str(), "App_without_TxHPC", "App_with_TxHPC", jacobian3, 0, "milliseconds", iterations);
    }
    else if(application == "jf_scale"){
        std::cout<<"JF_scale STARTED." << std::endl;

        //1GB = 11 180
        //10GB = 35356
        //100GB = 111 804
        //250GB = 176776
        //500GB = 250 000
        //1000TB = 353 553

        std::string j2D("Jacobian2D on NVM");
        j2D += " - Size: " + to_string(sizeOfDimension*sizeOfDimension* sizeof(double)) + ", ";
        j2D += "Iterations: " + to_string(iterations);

        double jacTxHPC_measuredTime = jacobian2d_TxHPC((char**)devshmPtr, amountOfShelves, 20, sizeOfDimension, sizeOfDimension, iterations);
        std::cout << "Jacobi 2D TxHPC: " << jacTxHPC_measuredTime <<  " milliseconds" << std::endl;

        double _jacobian2d= jacobian2d(devshmPtr[0], 2500, 2500, false, iterations); //500 MB
        std::cout << "Jacobian2D: " << _jacobian2d << " milliseconds" << std::endl;

        gnuplot_compareApplications("Jacobi2D_Iteration_ScalingUP", j2D.c_str(), "App_without_TxHPC", "App_with_TxHPC", _jacobian2d, jacTxHPC_measuredTime, "milliseconds", iterations);
    }

    else if(application == "gf"){
        std::cout << "gf started with " << iterations << " Iterations." << std::endl;


        std::string gf("GameOfLife on NVM - Size: 500MB - Iterations: ");
        gf += to_string(iterations);

        //Each cell in GameOfLife is a int, ergo 4 byte.
        //11200
        double gameOfLife1 = gameOfLife((char**)devshmPtr, 11200, 11200, false, iterations); // +500 MB
        std::cout << "GameOfLife: " << gameOfLife1 << " milliseconds " << std::endl;

        //11200
        double gameOfLife_TxHPC_measured = gameOfLife_TxHPC((char**) devshmPtr, amountOfShelves, 20, 11200, 11200, iterations+2);
        std::cout << "GameOfLife 2D TxHPC: " << gameOfLife_TxHPC_measured <<  " milliseconds" << std::endl;

        gnuplot_compareApplications("GameOfLife", gf, "App_without_TxHPC", "App_with_TxHPC", gameOfLife1, gameOfLife_TxHPC_measured, "milliseconds", iterations);
    }
    else if(application == "gf_computation"){
        std::cout << "gf with more computation started with " << iterations << " Iterations." << std::endl;

        std::string gf("GameOfLife with 100 times more computation on NVM - Size: ~500MB - Iterations: ");
        gf += to_string(iterations);

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

        gnuplot_compareApplications("GameOfLife_MoreComputation", gf, "App_without_TxHPC", "App_with_TxHPC", gameOfLife1, gameOfLife_TxHPC_measured, "milliseconds", iterations);
    }
    else if(application == "sm"){
        std::cout << "SpM started" << std::endl;
        std::string sm("SparseMatrixMultiplicatoin V2 on NVM - Size: ~500MB - Iterations:");
        sm += to_string(iterations);


        //Version from the internet //1000, 2500, 100
        //Compare this with the version in file 'sparseMatrixMultiplication'
        double SpMv2 = SpMV(devshmPtr, 100, 2500, 100, iterations); //microseconds
        std::cout << "SparseMatrixMultiplication internet style: " << SpMv2 <<  " microseconds" << std::endl;
        //The function sparseMatrixMultiplication(...) changes the devshmPtr[i]. No Idea why! - So retrieving backup
        for (int i = 0; i < amountOfShelves; ++i) {
            devshmPtr[i] = backupPtr[i];
        }

        double spM_TxHPC = sparseMatrix_TxHPC((char**) devshmPtr, amountOfShelves, 4, 2500, 2500, iterations+2);
        std::cout << "SparseMatrix TxHPC: " << spM_TxHPC <<  " microseconds" << std::endl;

        gnuplot_compareApplications("SparseMatrixMultiplication", sm, "App_without_TxHPC", "App_with_TxHPC", SpMv2, spM_TxHPC, "microseconds", iterations);
    }
    else if(application == "ac"){
        std::cout << "AC started" << std::endl;

        std::string ac("Memory Bandwidth on NVM - Size: ~500MB - Iterations: ");
        ac += to_string(iterations);

        double accessTimes_TxHPC = access_TxHPC((char**) devshmPtr, amountOfShelves, 40, 11180, 11180, iterations+2);
        std::cout << "AccessTime: " << accessTimes_TxHPC <<  " milliseconds" << std::endl;

        double accessTime_withoutTxHPC = access_WithOutTxHPC ((char**) devshmPtr, 11180, 11180, iterations);
        std::cout << "AccessTime: " << accessTime_withoutTxHPC <<  " milliseconds" << std::endl;
        gnuplot_compareApplications("MemoryBandwidth", ac, "App_without_TxHPC", "App_with_TxHPC", accessTime_withoutTxHPC, accessTimes_TxHPC, "microseconds", iterations);
    }

/*
    double spM1 =  sparseMatrixMultiplication(devshmPtr[0], 2500, 2500, false, iterations); // 500MB
    std::cout << "SparseMatrixMultiplication in StencilForTxHPC style: " << spM1 << std::endl;
    //Retrieve Backup

*/
    return 0;
};
