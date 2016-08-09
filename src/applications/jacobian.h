//
// Created by kuscum on 7/18/16.
//

#ifndef FASD_JACOBIAN2D_H
#define FASD_JACOBIAN2D_H

#include <include/StencilForTxHPC/TxHPC.h>

using namespace StencilForTxHPC;

void kernel_jacobian2d(int x, int y, GridPointer gP, Grid<double> grid);
double jacobian2d(void* memoryPointer, int x_axes, int y_axes, bool printGrid, int iterations);

void kernel_jacobian3d(int x, int y, int z, GridPointer gP, Grid<double> grid);
double jacobian3d(void* memoryPointer, int x_axes, int y_axes, int z_axes, bool printGrid, int iterations);

//TxHPC
double kernel_TxHPC_jacobian2d(int x, int y, Grid<double> grid);
double init_TxHPC_jacobian2d();
double jacobian2d_TxHPC(char** memoryPointer, int amountOfShelfs, int prefetch_size, int x_axes, int y_axes, int iterations);


#endif //FASD_JACOBIAN2D_H