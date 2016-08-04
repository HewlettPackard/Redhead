#include <stdio.h>
#include <cstring>

#include <include/StencilForTxHPC/TxHPC.h>
#include <config/settings.h>

#include <src/applications/jacobian.h>
#include <vector>

using namespace StencilForTxHPC;


int main(){
    Grid<double> grid(2500, 2500); //Creates grid in memory
    Domain d(1, 2500-1, 1, 2500-1);
    Stencil<double> stencil(kernel_jacobian2d, &grid, &d);

    long LogicalArray[10][10];
    long LinearArray[5][20]; //5 Stripes with a 20 cells

    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            LogicalArray[i][j] = j;
        }
    }
    //std::cout << "Linear Array" << std::endl;
    for (int k = 0; k < 5; ++k) {
        for (int i = 0; i < 20; ++i) {
            LinearArray[k][i] = 0; //INIT
        }
    }

#pragma Mark copy values into stripes of values
    int stripeID = 0;
    int no_cell = 0;
    for (int x = 0; x < 10; ++x) {
        for (int y = 0; y < 10; ++y) {
            int linearIndex = trans_2D_to_lin(x, y, 10);
            std::pair<int,int> stripeToCell = lin_to_offset_stripeID(linearIndex, 20); //First offset and then StripeID
            LinearArray[stripeToCell.second][stripeToCell.first] = LogicalArray[x][y];
            no_cell++;
            if(((no_cell % 20) == 0) && (no_cell != 0)){
                stripeID++;
            }
        }
    }
    if(!(no_cell == 100)){
        handle_error("not enough cells touched. Memory init went wrong.");
    }

#pragma mark read out the values in the linear array
    int counter = 0;
    for (int m = 0; m < 5; ++m) {
        for (int i = 0; i < 20; ++i) {
            if(!(counter == LinearArray[m][i])){
                handle_error("Address translating went wrong.");
            }
            counter++;
            if(counter == 10){
                counter = 0;
            }
        }
    }


    long array3D[5][5][5];
    long linearArray[5][25]; //5 Stripes with 25 Cells

    //INIT
    for (int l = 0; l < 5; ++l) {
        for (int i = 0; i < 5; ++i) {
            for (int j = 0; j < 5; ++j) {
                array3D[l][i][j] = j; //Count 1,2,3,4,5 in each row
            }
        }
    }
    for (int n = 0; n < 5; ++n) {
        for (int i = 0; i < 25; ++i) {
            linearArray[n][i] = 0;
        }
    }

    //Copy Values
    stripeID = 0;
    no_cell = 0;
    for (int i1 = 0; i1 < 5; ++i1) {
        for (int i = 0; i < 5; ++i) {
            for (int j = 0; j < 5; ++j) {
                int linearIndex = trans_3D_to_lin(j, i, i1, 5, 5);
                std::pair<int, int> stripeToCell = lin_to_offset_stripeID(linearIndex, 25); //First offset and then StripeID
                //std::cout << "The linear index is " << linearIndex << std::endl;
                linearArray[stripeToCell.second][stripeToCell.first] = array3D[i1][i][j]; //Do Copy
                no_cell++;
                if (((no_cell % 25) == 0) && (no_cell != 0)) {
                    stripeID++;
                }
            }
        }
    }
    if(!(no_cell == 125)){
        handle_error("Not enough cells touched. Something is wrong with the init.");
    }
    int counterFor3D = 0;
    for (int n = 0; n < 5; ++n) {
        for (int i = 0; i < 25; ++i) {
            if(!(counterFor3D == linearArray[n][i])){
                handle_error("Translation error in the 3D grid.");
            }
            counterFor3D++;
            if(counterFor3D == 5){
                counterFor3D = 0;
            }
        }
    }
    std::cout << "translation test successful."<< std::endl;
    return 0;
}