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

    int** LFSPtr = cat<int>::createShelvesAndMMAP(amountOfShelves, SHELVESIZE, MMAP_DIR_LFS);
    cat<int>::checkMemory(LFSPtr, amountOfShelves);

#pragma mark CALL APPLICATIONS WITHOUT TxHPC
    int iterations = 1000;
    //Input parameters of following applications:
    //Memory pointer, Length of X-Axes, Length of Y-Axes, Print result, Number of iterations

    //Each cell in Jacobian is a double, ergo 8 byte.
    double jacobian2 = jacobian2d(devshmPtr[0], 2500, 2500, false, iterations); //500 MB
    std::cout << "Jacobian2D: " << jacobian2 << std::endl;

    double jacobian3 = jacobian3d(devshmPtr[0], 400, 400, 400, false, iterations); //+500 MB
    std::cout << "Jacobian3D: " << jacobian3 << std::endl;

    //Each cell in GameOfLife is a int, ergo 4 byte.
    double gameOfLife1 = gameOfLife(devshmPtr[0], 11200, 11200, false, iterations); // +500 MB
    std::cout << "GameOfLife: " << gameOfLife1 << std::endl;


#pragma mark CALLING APPLICATIONS WITH TxHPC


/*
    double spM1 =  sparseMatrixMultiplication(devshmPtr[0], 2500, 2500, false, iterations); // 500MB
    std::cout << "SparseMatrixMultiplication in StencilForTxHPC style: " << spM1 << std::endl;
    //Retrieve Backup
    //The function sparseMatrixMultiplication(...) changes the devshmPtr[i]. No Idea why! - So retrieving backup
    for (int i = 0; i < amountOfShelves; ++i) {
        devshmPtr[i] = backupPtr[i];
    }
*/
    //Version from the internet //1000, 2500, 100
    double SpMv2 = SpMV(devshmPtr, 100, 2500, 100, iterations); //microseconds
    std::cout << "SparseMatrixMultiplication internet style: " << SpMv2 <<  " microseconds" << std::endl;

    int *tempTest = devshmPtr[0];

    gnuplot_compareApplications("Jacobian2D", "Jacobian2D on /Dev/SHM - Size: 500MB - Iterations: 100", "App_without_TxHPC", "App_with_TxHPC", jacobian2, 0, "milliseconds");
    gnuplot_compareApplications("Jacobian3D", "Jacobian3D on /Dev/SHM - Size: ~500MB - Iterations: 100", "App_without_TxHPC", "App_with_TxHPC", jacobian3, 0, "milliseconds");
    gnuplot_compareApplications("GameOfLife", "GameOfLife on /Dev/SHM - Size: ~500MB - Iterations: 100", "App_without_TxHPC", "App_with_TxHPC", gameOfLife1, 0, "milliseconds");
    gnuplot_compareApplications("SparseMatrixMultiplication", "SparseMatrixMultiplicatoin V2 on /Dev/SHM - Size: ~500MB - Iterations: 100", "App_without_TxHPC", "App_with_TxHPC", SpMv2, 0, "microseconds");

    return 0;
};
