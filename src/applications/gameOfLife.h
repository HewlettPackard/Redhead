//
// Created by kuscum on 7/22/16.
//

#ifndef FASD_GAMEOFLIFE_H
#define FASD_GAMEOFLIFE_H

#include "include/StencilForTxHPC/TxHPC.h"

using namespace StencilForTxHPC;



double kernel_gameOfLife_TxHPC(int x, int y, Grid<int> grid);

void kernel_gameOfLife(int x, int y, GridPointer gP, Grid<int> grid);
double gameOfLife(void* memoryPointer, int x_axes, int y_axes, bool printGrid, int iterations);

#endif //FASD_GAMEOFLIFE_H
