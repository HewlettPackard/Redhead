//
// Created by kuscum on 7/24/16.
//

#ifndef FASD_SPARSEMATRIXMULTIPLICATION_H
#define FASD_SPARSEMATRIXMULTIPLICATION_H

#include "include/StencilForTxHPC/TxHPC.h"

using namespace StencilForTxHPC;

int kernel_SpM_TxHPC(int x, int y, Grid<int > grid);
void initAdditionalArrays();
double sparseMatrix_TxHPC(char** memoryPointer, int amountOfShelfs, int prefetch_size, int x_axes, int y_axes, int iterations);

int init_spM();

void kernel_SpM(int x, int y, GridPointer gP, Grid<int> grid);
double sparseMatrixMultiplication(void* memoryPointer, int x_axes, int y_axes, bool printGrid, int iterations);

#endif //FASD_SPARSEMATRIXMULTIPLICATION_H
