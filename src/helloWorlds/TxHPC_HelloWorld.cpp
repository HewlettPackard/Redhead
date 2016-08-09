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

    int iterations = 1;
    //Creates 11 shelves and mmaps it to this process
    int amountOfShelves = 11;
    int* backupPtr[amountOfShelves];
    char** devshmPtr = cat<char>::createShelvesAndMMAP(amountOfShelves, SHELVESIZE, MMAP_DIR_DEVSHM);
    cat<char>::checkMemory(devshmPtr, amountOfShelves);

#pragma mark TxHPC
//    //// Jacobian 2D
    Grid<double> grid(2500, 2500); //Creates grid in memory
    Domain d(1, 0, 2498, 2499); //Left and right bound
    Stencil<double> stencil(kernel_TxHPC_jacobian2d, &grid, &d);
    stencil.setInitFunction(init_TxHPC_jacobian2d);
    stencil.init(devshmPtr, amountOfShelves, SHELVESIZE, 4, 1024);
    stencil.run_withTxHPC(iterations); //Number of iterations

    //// Game Of Life
//    Grid<int> game_of_life(11200, 11200); //Creates grid in memory
//    Domain d_gOfLife(1, 1, 2498, 2498); //Left and right bound
//    Stencil<int> stencil_gOfLife(kernel_gameOfLife_TxHPC, &game_of_life, &d_gOfLife);
//    stencil_gOfLife.setInitFunction(init_gameOfLife);
//    stencil_gOfLife.init(devshmPtr, amountOfShelves, SHELVESIZE, 4, 1024);
//    stencil_gOfLife.run_withTxHPC(iterations); //Number of iterations

    //// SparseM
//    Grid<int> spM(2500, 2500); //Creates grid in memory
//    Domain spM_d(1, 1, 2498, 2498); //Left and right bound
//    initAdditionalArrays();
//    Stencil<int> stencil_spM(kernel_SpM_TxHPC, &spM, &spM_d);
//    stencil_spM.setInitFunction(init_spM);
//    stencil_spM.init(devshmPtr, amountOfShelves, SHELVESIZE, 1, 1024);
//    stencil_spM.run_withTxHPC(iterations); //Number of iterations

//    //// Acess times
//    Grid<double> gridForAcess(100, 100);
//    Domain domainPoints(1,0,98,99);
//    Stencil<double> accessTimes(kernel_TxHPC_access, &gridForAcess, &domainPoints);
//    accessTimes.setInitFunction(init_TxHPC_access);
//    accessTimes.init(devshmPtr, amountOfShelves, SHELVESIZE, 1, 1024);
//    accessTimes.run_withTxHPC(iterations);

    return 0;
}