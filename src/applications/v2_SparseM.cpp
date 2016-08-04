//
// Created by kuscum on 6/30/16.
//

/*
SpMV - http://crd-legacy.lbl.gov/~oliker/papers/SC07_SPMV.pdf
*/

#include "v2_SparseM.h"
#include <tuple>
#include <chrono>
#include "include/gnuplot/gnuplot-iostream.h"

double SpMV(int** memoryPointer, int maxRandomValueInMatrix, int lengthOfAxes, int maxValuesInSparse, int iterations){
    double measureTimeStamps = 0;
    /* initialize random seed: */
    srand (time(NULL));
    int x = lengthOfAxes;
    int y = lengthOfAxes;

    //Every Matrix is in one shelve
    int *ptr1 = memoryPointer[0];
    int *ptr2 = memoryPointer[1];
    int *ptr3 = memoryPointer[2];

    int **Y_mmaped =  new int*[x];
    int **VAL_mmaped = new int*[x];
    int **X_mmaped = new int*[x];

    for (int l = 0; l < x; ++l) {
        Y_mmaped[l] = &ptr1[y * l];
        VAL_mmaped[l] = &ptr2[y * l];
        X_mmaped[l] = &ptr3[y * l];
    }

        //Init Matrices, Randomize X and Y
        for (int i = 0; i < x; ++i) {
            for (int j = 0; j < y; ++j) {
                VAL_mmaped[i][j] = 0;
                X_mmaped[i][j] = rand() % (maxRandomValueInMatrix + 1);
                Y_mmaped[i][j] = rand() % (maxRandomValueInMatrix + 1);
            }
        }

        //Randomize VAL 2D Array - should be a spare array
        int amountEntriesInA = rand() % maxValuesInSparse + 1; //Entries which are not 0 - between 1 and 4
        int ind[amountEntriesInA]; //column indices of each element
        int ptr[amountEntriesInA]; //keep track where each row starts
        for (int a = 0; a < amountEntriesInA; ++a) {
            ind[a] = rand() % y;
            ptr[a] = rand() % x;
            //Place it randomly in VAL
            VAL_mmaped[ptr[a]][ind[a]] = rand() % (maxRandomValueInMatrix + 1) + 1; //Random number
        }
/*
        //Debug Print solution
        for (int m = 0; m < lengthOfAxes; ++m) {
            for (int i = 0; i < lengthOfAxes; ++i) {
                std::cout << Y_mmaped[m][i] << " ";
            }
            std::cout << std::endl;
        }*/

        std::chrono::steady_clock::time_point begin_run = std::chrono::steady_clock::now();
        for (int l = 0; l < iterations; ++l) {
            // y <- y + A * x
            for (int i = 0; i < amountEntriesInA; ++i) {
                Y_mmaped[ptr[i]][ind[i]] = Y_mmaped[ptr[i]][ind[i]] + VAL_mmaped[ptr[i]][ind[i]] * X_mmaped[ptr[i]][ind[i]];
            }
        }
        std::chrono::steady_clock::time_point end_run = std::chrono::steady_clock::now();

/*
        std::cout << std::endl;
        //Debug Print solution
        for (int m = 0; m < lengthOfAxes; ++m) {
            for (int i = 0; i < lengthOfAxes; ++i) {
                std::cout << Y_mmaped[m][i] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << "VAL: " << std::endl;
        //Debug Print solution
        for (int m = 0; m < lengthOfAxes; ++m) {
            for (int i = 0; i < lengthOfAxes; ++i) {
                std::cout << VAL_mmaped[m][i] << " ";
            }
            std::cout << std::endl;
        }
*/

    double measuredTime = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(
                end_run - begin_run).count());

    return measuredTime;
};