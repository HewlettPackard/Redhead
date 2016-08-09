//
// Created by kuscum on 8/4/16.
//

#include <iostream>
#include <chrono>
#include "access_example.h"
#include "include/StencilForTxHPC/TxHPC.h"

double kernel_TxHPC_access(int x, int y, Grid<double> grid){
    double newValue =  grid.cell(x-1, y); //Take the cell above
//    std::cout << "VALUE IS " << newValue << std::endl;
    return (newValue + 1);
}

void kernel_access(int x, int y, GridPointer gP, Grid<double> grid){
    double newValue = (grid.getCell(x-1, y) +1);

    grid.Emit(newValue, gP);
}

double init_TxHPC_access(){
    return 1;
}

double access_TxHPC(char** memoryPointer, int amountOfShelfs, int prefetch_size, int x_axes, int y_axes, int iterations){
    Grid<double> gridForAcess(x_axes, y_axes);
    Domain domainPoints(1,0,x_axes-2,y_axes-1);
    Stencil<double> accessTimes(kernel_TxHPC_access, &gridForAcess, &domainPoints);
    accessTimes.setInitFunction(init_TxHPC_access);
    accessTimes.init(memoryPointer, amountOfShelfs, SHELVESIZE, prefetch_size, 1024);

    std::chrono::steady_clock::time_point begin_run = std::chrono::steady_clock::now();
    accessTimes.run_withTxHPC(iterations); //Number of iterations
    std::chrono::steady_clock::time_point end_run = std::chrono::steady_clock::now();
    double measuredtime =  static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds> (end_run - begin_run).count());
    return measuredtime;
};


double access_WithOutTxHPC(char** memoryPointer, int x_axes, int y_axes, int iterations){
    Grid<double> grid(x_axes, y_axes, (double*)memoryPointer[0]); //Creates grid in memory

    Domain dim3d(1, 1, x_axes-2, y_axes-2);
    Stencil<double> stencil(kernel_access, &grid, &dim3d);
    //stencil.printGrid(); For Debug

    //Measured time
    std::chrono::steady_clock::time_point begin_run = std::chrono::steady_clock::now();
    stencil.iterate(iterations);
    std::chrono::steady_clock::time_point end_run = std::chrono::steady_clock::now();
    //Set to microseconds
    double measuredtime =  static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds> (end_run - begin_run).count());

    return measuredtime;
};