//
// Created by kuscum on 7/18/16.
//

#include <chrono>
#include "jacobian.h"
//#include "include/StencilForTxHPC/TxHPC.h"

double kernel_TxHPC_jacobian2d(int x, int y, Grid<double> grid){
    double newValue =  0.25 * (grid.cell(x+1, y) + grid.cell(x-1, y) + grid.cell(x, y+1) + grid.cell(x, y-1));

    std::cout << "VALUE IS " << newValue << std::endl;

    return newValue;
}

//This is called on cells which are not in the defined domain so they are init. with this value
//Init every cell which is not in the domain with the value 1
double init_TxHPC_jacobian2d(){
     return 1;
}

void kernel_jacobian2d(int x, int y, GridPointer gP, Grid<double> grid){
    double newValue = 0 * (grid.getCell(x, y)) + 0.25 * (
          grid.getCell(x+1, y) + grid.getCell(x-1, y)
        + grid.getCell(x, y+1) + grid.getCell(x, y-1));

    grid.Emit(newValue, gP);
}

void kernel_jacobian3d(int x, int y, int z, GridPointer gP, Grid<double> grid){
    double newValue =
    0.167 * (
          grid.getCell(x+1, y, z) + grid.getCell(x-1, y, z)
        + grid.getCell(x, y+1, z) + grid.getCell(x, y-1, z)
        + grid.getCell(x, y, z+1) + grid.getCell(x, y, z-1));

    grid.Emit(newValue, gP);
}

double jacobian2d(void* memoryPointer, int x_axes, int y_axes, bool printGrid, int iterations){
    Grid<double> grid(x_axes, y_axes, (double*)memoryPointer); //Creates grid in memory

    //Init Grid.
    double** array;
    array = new double* [x_axes];
    for (int k = 0; k < x_axes; ++k) {
        array[k] = new double[y_axes];
    }
    //Set the boundaries to such a value
    for (int i = 0; i < x_axes; ++i) {
        for (int j = 0; j < y_axes; ++j) {
            if(j == 0){
                //first row
                array[i][j] = 1;
            }
            else if (j == (y_axes-1)){
                //last row
                array[i][j] = 1;
            }
            else{
                array[i][j] = 0;
            }
        }
    }

    grid.initGrid(array, 0, x_axes, 0, y_axes);
    //Set the dimension properly, so the run function does not go out of the array boundary
    Domain d(1, x_axes-1, 1, y_axes-1);
    Stencil<double> stencil(kernel_jacobian2d, &grid, &d);
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

double jacobian3d(void* memoryPointer, int x_axes, int y_axes, int z_axes, bool printGrid, int iterations){
    Grid<double> grid(x_axes, y_axes, z_axes, (double*)memoryPointer); //Creates grid in memory

    //Init Grid.
    double*** array = new double** [x_axes];
    //Set sample z array
    double z_array[z_axes];
    for (int l = 0; l < z_axes; ++l) {
        z_array[l] = l;
    }

    for (int i = 0; i < x_axes; ++i) {
        array[i] = new double* [y_axes];
        for (int j = 0; j < y_axes; ++j) {
            //Alloc memory
            array[i][j] = new double[z_axes];

            //Init sample values
            for (int k = 0; k < z_axes; ++k) {
                //Make left and right wall 1, every other value should be 0
                //So we can simulate heat diffusion
                if((i == 0) ||(i == x_axes-1)){
                    array[i][j][k] = 1;
                }else{
                    array[i][j][k] = 0;
                }
            }
        }
    }

    grid.initGrid(array, 0, x_axes, 0, y_axes, 0, z_axes);
    //Set the dimension properly, so the run function does not go out of the array boundary
    Domain dim3d(1, x_axes-1, 1, y_axes-1, 1, z_axes-1);
    Stencil<double> stencil(kernel_jacobian3d, &grid, &dim3d);
    //stencil.printGrid(); For Debug

    //Measured time
    std::chrono::steady_clock::time_point begin_run = std::chrono::steady_clock::now();
    stencil.iterate(iterations);
    std::chrono::steady_clock::time_point end_run = std::chrono::steady_clock::now();
    //Set to microseconds
    double measuredtime =  static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds> (end_run - begin_run).count());

    if(printGrid){
        stencil.printDomain(&dim3d);
    }
    return measuredtime;
};

