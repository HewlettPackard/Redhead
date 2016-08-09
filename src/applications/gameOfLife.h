//
// Created by kuscum on 7/22/16.
//

#ifndef FASD_GAMEOFLIFE_H
#define FASD_GAMEOFLIFE_H

#include "include/StencilForTxHPC/TxHPC.h"

using namespace StencilForTxHPC;



int kernel_gameOfLife_TxHPC(int x, int y, Grid<int> grid);
int kernel_gameOfLife_TxHPC_more_computation(int x, int y, Grid<int> grid);

int init_gameOfLife();

void kernel_gameOfLife(int x, int y, GridPointer gP, Grid<int> grid);
double gameOfLife(char** memoryPointer, int x_axes, int y_axes, bool printGrid, int iterations);
double gameOfLife_more_computation(void* memoryPointer, int x_axes, int y_axes, bool printGrid, int iterations);
double gameOfLife_TxHPC(char** memoryPointer, int amountOfShelfs, int prefetch_size, int x_axes, int y_axes, int iterations);
double gameOfLife_TxHPC_more_computation(char** memoryPointer, int amountOfShelfs, int prefetch_size, int x_axes, int y_axes,
                                         int iterations);

#endif //FASD_GAMEOFLIFE_H
