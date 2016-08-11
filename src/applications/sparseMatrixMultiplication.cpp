//
// Created by kuscum on 7/24/16.
//

#include <chrono>
#include "sparseMatrixMultiplication.h"
//#include "include/StencilForTxHPC/TxHPC.h"

#define arraySize 2500
#define maxRandomValue 1000

int additionalArray[arraySize][arraySize]; //Dense array
int sparseMatrix[arraySize][arraySize];

//With TxHPC
int kernel_SpM_TxHPC(int x, int y, Grid<int > grid){
    int value = grid.cell(x, y) + sparseMatrix[x][y] * additionalArray[x][y];
    return value;
}

int init_spM(){
    return 0;
}

//Without TxHPC
void kernel_SpM(int x, int y, GridPointer gP, Grid<int> grid){
    int value = grid.getCell(x, y) + sparseMatrix[x][y] * additionalArray[x][y];
    grid.Emit(value, gP);
}

//Init add. array for calculation
void initAdditionalArrays(){
    for (int i = 0; i < arraySize; ++i) {
        for (int j = 0; j < arraySize; ++j) {
            additionalArray[i][j] = rand() % maxRandomValue;
            sparseMatrix[i][j] = 0;
        }
    }
}

double sparseMatrixMultiplication(void* memoryPointer, int x_axes, int y_axes, bool printGrid, int iterations){
    //Allocating Grid in Memory
    int *ptr1 = reinterpret_cast<int*>(memoryPointer);
    Grid<int> grid(x_axes, y_axes, ptr1); //Creates grid in memory

    //Init Grid
    int** array;
    array = new int* [x_axes];
    for (int k = 0; k < x_axes; ++k) {
        array[k] = new int[y_axes];
    }
    srand (time(NULL));

    for (int i = 0; i < x_axes; ++i) {
        for (int j = 0; j < y_axes; ++j) {
            array[i][j] = rand() % maxRandomValue; //random values between 999 - 0
            additionalArray[i][j] = rand() % maxRandomValue;
            sparseMatrix[i][j] = 0;
        }
    }
    grid.initGrid(array, 0, x_axes, 0, y_axes);

    //Init other arrays
    //Sparse matrix
    int notZeroEntries = rand() % 11 + 1; //1 - 10 Entries which are not zero
    int x = 0, y = 0;
    for (int l = 0; l < notZeroEntries; ++l) {
        x = rand() % arraySize;
        y = rand() % arraySize;

        sparseMatrix[x][y] = rand() % maxRandomValue;
    }

    //Set the Domain
    Domain d(0, x_axes, 0, y_axes);
    Stencil<int> stencil(kernel_SpM, &grid, &d);

    //stencil.printGrid();
    //stencil.run(); same as iterate(1)
    //Measured time
    std::chrono::steady_clock::time_point begin_run = std::chrono::steady_clock::now();
    stencil.iterate(iterations);
    std::chrono::steady_clock::time_point end_run = std::chrono::steady_clock::now();
    //Set to microseconds
    double measuredtime =  static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds> (end_run - begin_run).count());

    if(printGrid){
        stencil.printDomain(&d);
    }

    return measuredtime;
};

double sparseMatrix_TxHPC(char** memoryPointer, int amountOfShelfs, int prefetch_size, int x_axes, int y_axes, int iterations){
    Grid<int> spM(x_axes, y_axes); //Creates grid in memory
    Domain spM_d(1, 1, x_axes-2, y_axes-2); //Left and right bound
    initAdditionalArrays();
    Stencil<int> stencil_spM(kernel_SpM_TxHPC, &spM, &spM_d);
    stencil_spM.setInitFunction(init_spM);

    stencil_spM.init(memoryPointer, amountOfShelfs, SHELVESIZE, 1, 1024);

    std::chrono::steady_clock::time_point begin_run = std::chrono::steady_clock::now();
    stencil_spM.run_withTxHPC(iterations); //Number of iterations
    std::chrono::steady_clock::time_point end_run = std::chrono::steady_clock::now();
    double measuredtime =  static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds> (end_run - begin_run).count());
    return measuredtime;
};