#include <stdio.h>
#include <cstring>

#include <include/StencilForTxHPC/TxHPC.h>
#include <config/settings.h>

#include <src/applications/jacobian.h>
#include <src/applications/access_example.h>
#include <src/applications/gameOfLife.h>
#include <src/applications/sparseMatrixMultiplication.h>
#include <src/applications/v2_SparseM.h>

#include <vector>

using namespace StencilForTxHPC;


int main(){

    int iterations = 1000;
    //Creates 11 shelves and mmaps it to this process
    int amountOfShelves = 11;
    int* backupPtr[amountOfShelves];
    char** devshmPtr = cat<char>::createShelvesAndMMAP(amountOfShelves, SHELVESIZE, MMAP_DIR_DEVSHM);
    cat<char>::checkMemory(devshmPtr, amountOfShelves);

#pragma mark TxHPC
//    //// Jacobian 2D
//    Grid<double> grid(2500, 2500); //Creates grid in memory
//    Domain d(1, 2499, 0, 2498); //Left and right bound
//    Stencil<double> stencil(kernel_TxHPC_jacobian2d, &grid, &d);
//    stencil.setInitFunction(init_TxHPC_jacobian2d);
//    stencil.init(devshmPtr, amountOfShelves, SHELVESIZE, 4, 1024);
//    //stencil.run_withTxHPC(iterations); //Number of iterations

    //// Acess times
    Grid<double> gridForAcess(100, 100);
//    Domain d1(0, 99, 1, 98);

    Domain domainPoints(1,0,98,99);

    Stencil<double> accessTimes(kernel_TxHPC_access, &gridForAcess, &domainPoints);
    accessTimes.setInitFunction(init_TxHPC_access);
    accessTimes.init(devshmPtr, amountOfShelves, SHELVESIZE, 1, 1024);

    accessTimes.run_withTxHPC(1000);


    //// Game Of Life

    //// SparseM

    //// Jacobian 3D




    return 0;
}