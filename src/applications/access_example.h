//
// Created by kuscum on 8/4/16.
//

#ifndef FASD_ACCESS_EXAMPLE_H
#define FASD_ACCESS_EXAMPLE_H

#include <include/StencilForTxHPC/TxHPC.h>

using namespace StencilForTxHPC;
double kernel_TxHPC_access(int x, int y, Grid<double> grid);
double init_TxHPC_access();


#endif //FASD_ACCESS_EXAMPLE_H
