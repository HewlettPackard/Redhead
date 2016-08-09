//
// Created by kuscum on 8/4/16.
//

#ifndef FASD_ACCESS_EXAMPLE_H
#define FASD_ACCESS_EXAMPLE_H

#include <include/StencilForTxHPC/TxHPC.h>

using namespace StencilForTxHPC;
double kernel_TxHPC_access(int x, int y, Grid<double> grid);
double init_TxHPC_access();

double access_TxHPC(char** memoryPointer, int amountOfShelfs, int prefetch_size, int x_axes, int y_axes, int iterations);
double access_WithOutTxHPC(char** memoryPointer, int x_axes, int y_axes, int iterations);
void kernel_access(int x, int y, GridPointer gP, Grid<double> grid);

#endif //FASD_ACCESS_EXAMPLE_H
