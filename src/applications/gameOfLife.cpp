//
// Created by kuscum on 7/22/16.
//

#include <chrono>
#include "gameOfLife.h"
//#include "include/StencilForTxHPC/TxHPC.h"


#define ITERATIONS_GML 10
long _debug_TxHPC = 0;
long _debug2 = 0;
int kernel_gameOfLife_TxHPC_more_computation(int x, int y, Grid<int> grid){
    for (int i = 1; i < ITERATIONS_GML; ++i) {
        kernel_gameOfLife_TxHPC(x, y, grid);
    }
    return kernel_gameOfLife_TxHPC(x, y, grid);
}

//With TxHPC
int kernel_gameOfLife_TxHPC(int x, int y, Grid<int> grid){
//    _debug_TxHPC++;
    int sum =
            //Horizontal Neighbours
            grid.cell(x+1, y) + grid.cell(x-1, y)
            + grid.cell(x, y+1) + grid.cell(x, y-1)
            //Diagonal Neighbours
            + grid.cell(x+1, y+1) + grid.cell(x+1, y-1)
            + grid.cell(x-1, y-1) + grid.cell(x-1, y+1);

    if(grid.cell(x, y) == 1){
        //Cell alive - does it survive?
        if((1 < sum) && (sum < 4)){
            //if 2 or 3 neighbors, the cell survives
           return 1;
        }
        else{
            //cell dies! Cell is either overpopulated or isolated
           return 0;
        }
    }
    else{
        //Cell is dead - will it be born?
        if(sum == 3){
            //Cell lives
            return 1;
        }
        else{
            //Cell stays dead
            return 0;
        }
    }
}

//This is called on cells which are not in the defined domain so they are init. with this value
//Init every cell which is not in the domain with the value 1
int init_gameOfLife() {
    return 1;
}

//Without TxHPC
void kernel_gameOfLife(int x, int y, GridPointer gP, Grid<int> grid){
//    _debug2++;
    int sum =
            //Horizontal Neighbours
              grid.getCell(x+1, y) + grid.getCell(x-1, y)
            + grid.getCell(x, y+1) + grid.getCell(x, y-1)
            //Diagonal Neighbours
            + grid.getCell(x+1, y+1) + grid.getCell(x+1, y-1)
            + grid.getCell(x-1, y-1) + grid.getCell(x-1, y+1);

    if(grid.getCell(x, y) == 1){
        //Cell alive - does it survive?
        if((1 < sum) && (sum < 4)){
            //if 2 or 3 neighbors, the cell survives
            grid.Emit(1, gP);
            return;
        }
        else{
            //cell dies! Cell is either overpopulated or isolated
            grid.Emit(0, gP);
            return;
        }
    }
    else{
        //Cell is dead - will it be born?
        if(sum == 3){
            //Cell lives
            grid.Emit(1, gP);
            return;
        }
        else{
            //Cell stays dead
            grid.Emit(0, gP);
            return;
        }
    }
}


double gameOfLife_more_computation(void* memoryPointer, int x_axes, int y_axes, bool printGrid, int iterations){
    /*  const int x_axes = x_length;
      const int y_axes = y_length;*/
    Grid<int> grid(x_axes, y_axes, (int*)memoryPointer); //Creates grid in memory
    //Init Grid.
    int** array;
    array = new int* [x_axes];
    for (int k = 0; k < x_axes; ++k) {
        array[k] = new int[y_axes];
    }

    //Init Grid with random live and dead cells
    srand (time(NULL));
    for (int i = 0; i < x_axes; ++i) {
        for (int j = 0; j < y_axes; ++j) {
            if((j == 0) || (i == 0) || (i == x_axes-1) || (j == y_axes-1)){
                //Init with 0, these are the boundaries
                array[i][j] = 0;
            }
            else{
                array[i][j] = rand() % 2;
            }
        }
    }
    grid.initGrid(array, 0, x_axes, 0, y_axes);
    //Set the dimension properly, so the run function does not go out of the array boundary
    Domain d(1, 1, x_axes-1, y_axes-1);
    Stencil<int> stencil(kernel_gameOfLife, &grid, &d);

    //Measured time
    std::chrono::steady_clock::time_point begin_run = std::chrono::steady_clock::now();
    for (int l = 0; l < ITERATIONS_GML; ++l) {
        stencil.iterate(iterations);
    }
    std::chrono::steady_clock::time_point end_run = std::chrono::steady_clock::now();
    //Set to microseconds
    double measuredtime =  static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds> (end_run - begin_run).count());
    std::cout << "kernel was called " << _debug2 << std::endl;

    if(printGrid){
        stencil.printDomain(&d);
    }

    /*//Further Functions to use
    stencil.iterate(10); //iterates through run
    stencil.print(); //prints the solution
    stencil.VisIt(); //Visualizes the data if its 3D with VisIt
    stencil.MPI(); //Runs simulation on multiple processors*/

    return measuredtime;
};

double gameOfLife(char** memoryPointer, int x_axes, int y_axes, bool printGrid, int iterations){
    /*  const int x_axes = x_length;
      const int y_axes = y_length;*/
    Grid<int> grid(x_axes, y_axes, (int*)memoryPointer[0]); //Creates grid in memory
    //Init Grid.
    int** array;
    array = new int* [x_axes];

    for (int k = 0; k < x_axes; ++k) {
        array[k] = new int[y_axes];
    }

    //Init Grid with random live and dead cells
    srand (time(NULL));
    for (int i = 0; i < x_axes; ++i) {
        for (int j = 0; j < y_axes; ++j) {
            if((j == 0) || (i == 0) || (i == x_axes-1) || (j == y_axes-1)){
                //Init with 0, these are the boundaries
                array[i][j] = 0;
            }
            else{
                array[i][j] = rand() % 2;
            }
        }
    }
    grid.initGrid(array, 0, x_axes, 0, y_axes);
    //Set the dimension properly, so the run function does not go out of the array boundary
    Domain d(1, 1, x_axes-1, y_axes-1);
    Stencil<int> stencil(kernel_gameOfLife, &grid, &d);

    //Measured time
    std::chrono::steady_clock::time_point begin_run = std::chrono::steady_clock::now();
    stencil.iterate(iterations);
    std::chrono::steady_clock::time_point end_run = std::chrono::steady_clock::now();
    //Set to microseconds
    double measuredtime =  static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds> (end_run - begin_run).count());

    std::cout << "kernel was called " << _debug2 << std::endl;

    if(printGrid){
        stencil.printDomain(&d);
    }

    /*//Further Functions to use
    stencil.iterate(10); //iterates through run
    stencil.print(); //prints the solution
    stencil.VisIt(); //Visualizes the data if its 3D with VisIt
    stencil.MPI(); //Runs simulation on multiple processors*/

    return measuredtime;
};

double gameOfLife_TxHPC(char** memoryPointer, int amountOfShelfs, int prefetch_size, int x_axes, int y_axes, int iterations){
    Grid<int> game_of_life(x_axes, y_axes); //Creates grid in memory
    Domain d_gOfLife(1, 1, (x_axes-2), (y_axes-2)); //Left and right bound
    Stencil<int> stencil_gOfLife(kernel_gameOfLife_TxHPC, &game_of_life, &d_gOfLife);
    stencil_gOfLife.setInitFunction(init_gameOfLife);
    stencil_gOfLife.init(memoryPointer, amountOfShelfs, SHELVESIZE, prefetch_size, 1024);

    std::chrono::steady_clock::time_point begin_run = std::chrono::steady_clock::now();
    stencil_gOfLife.run_withTxHPC(iterations); //Number of iterations
    std::chrono::steady_clock::time_point end_run = std::chrono::steady_clock::now();

    double measuredtime =  static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds> (end_run - begin_run).count());
    std::cout << "kernel was called " << _debug_TxHPC << std::endl;
    return measuredtime;
};

double gameOfLife_TxHPC_more_computation(char** memoryPointer, int amountOfShelfs, int prefetch_size, int x_axes, int y_axes, int iterations){
    Grid<int> game_of_life(x_axes, y_axes); //Creates grid in memory
    Domain d_gOfLife(1, 1, (x_axes-2), (y_axes-2)); //Left and right bound
    Stencil<int> stencil_gOfLife(kernel_gameOfLife_TxHPC_more_computation, &game_of_life, &d_gOfLife);
    stencil_gOfLife.setInitFunction(init_gameOfLife);
    stencil_gOfLife.init(memoryPointer, amountOfShelfs, SHELVESIZE, prefetch_size, 1024);

    std::chrono::steady_clock::time_point begin_run = std::chrono::steady_clock::now();
    stencil_gOfLife.run_withTxHPC(iterations); //Number of iterations
    std::chrono::steady_clock::time_point end_run = std::chrono::steady_clock::now();
    double measuredtime =  static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds> (end_run - begin_run).count());
    std::cout << "kernel was called " << _debug_TxHPC << std::endl;
    return measuredtime;
};