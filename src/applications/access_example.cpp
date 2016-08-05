//
// Created by kuscum on 8/4/16.
//

#include <iostream>
#include "access_example.h"
#include "include/StencilForTxHPC/TxHPC.h"

double kernel_TxHPC_access(int x, int y, Grid<double> grid){
    double newValue =  grid.cell(x-1, y); //Take the cell above

    std::cout << "VALUE IS " << newValue << std::endl;

    return (newValue + 1);
}

double init_TxHPC_access(){
    return 1;
}