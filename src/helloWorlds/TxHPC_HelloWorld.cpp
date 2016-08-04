#include <stdio.h>
#include <cstring>

#include <include/StencilForTxHPC/TxHPC.h>
#include <config/settings.h>

#include <src/applications/jacobian.h>
#include <vector>

using namespace StencilForTxHPC;


int main(){
    //Creates 11 shelves and mmaps it to this process
    int amountOfShelves = 11;
    int* backupPtr[amountOfShelves];
    char** devshmPtr = cat<char>::createShelvesAndMMAP(amountOfShelves, SHELVESIZE, MMAP_DIR_DEVSHM);
    cat<char>::checkMemory(devshmPtr, amountOfShelves);

    Grid<double> grid(2500, 2500); //Creates grid in memory
    Domain d(1, 2498, 1, 2498);
    Stencil<double> stencil(kernel_TxHPC_jacobian2d, &grid, &d);
    stencil.setInitFunction(init_TxHPC_jacobian2d);

    stencil.init(devshmPtr, amountOfShelves, SHELVESIZE);
    stencil.run_withTxHPC();

    return 0;
}