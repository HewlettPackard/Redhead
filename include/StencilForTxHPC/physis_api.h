//
// Created by kuscum on 7/11/16.
//

#ifndef FASD_PHYSIS_API_H
#define FASD_PHYSIS_API_H
#include <iostream>
#include <sstream>
#include "TxHPC.h"
#include <cstddef>


namespace StencilForTxHPC {

    static void iterate_compute(void* args, char * cur_stripe, Meta * meta_info, int stripe_id, char* load, char** before, char** after);
    static void control_function(void * args, meta * meta_info);

    static bool print_flag = true;
    static int shitcounter = 0; //Used to print the data in a correct layout

#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)
//    static void **memory;
//    static int amountOfPointer;

#pragma mark 2D
//Gives you the pointer to 1 single dimension
    static  int trans_2D_to_lin(int y, int x, int rowSize1){
        int lin = y * rowSize1 + x;
        return lin;
    };
    static int trans_3D_to_lin(int x, int y, int z, int rowSize, int colSize){
        int lin = z * rowSize * colSize + y * rowSize + x;
    };
//Gives you the pointer to the global view again
//Only works for 2D
    static  std::pair<int, int> lin_to_offset_stripeID(int lin, int rowSize2){
        int offset = lin % rowSize2;
        int stripeID = (lin/rowSize2); //Automatic Floor of value (rounding up)
        return std::make_pair(offset, stripeID);
    };
    static  std::pair<int, int> trans_lin_to_2D(int lin, int rowSize){
        int y = lin % rowSize;
        int x = (lin/rowSize);
        return std::make_pair(x, y);
    };
#pragma mark NDIM
//int ndim_to_linear(vector<long> coard, vector<long> dim){
//    int lin = 0;
//    int term = 0;
//    for (int i = 0; i <dim.size(); ++i) {
//        term = coard[i];
//        for (int j = 0; j < i; ++j) {
//            term *= dim[j];
//        }
//        lin += term;
//    }
//    return term;
//};
//int lin_to_ndim(lin, recordn){
//    vectorcoard, prev_div = 1;
//    for (int i = 0; i < 0; ++i) {
//        div = 1;
//        for (int j = 0; j < i; ++j) {
//            div = dim[j];
//            shit = 0;
//            for (i + 1 = 0; i < i; ++i) {
//
//
//            }
//        }
//    }
//}

//Initalizes the API and sets the memory pointers globally
//so the user can create grid objects
    class GridPointer {
    private:
        //Cannot be changed once they are initialed
        int x = -1, y = -1, z = -1;
    public:
        GridPointer(int set_x, int set_y, int set_z) {
            x = set_x;
            y = set_y;
            z = set_z;
        }

        GridPointer(int set_x, int set_y) {
            x = set_x;
            y = set_y;
        }

        int get_x() {
            return x;
        }

        int get_y() {
            return y;
        }

        int get_z() {
            return z;
        }
    };

//Represents a multidimensional grid
    template<class GridType>
    class Grid {
    private:
        //Without TxHPC
        GridType **dataPointer2D;
        GridType ***dataPointer3D;
        int dimensions = 0;
        int x = 0;
        int y = 0;
        int z = 0;
        //TxHPC
        GridType *currentStripe;
        GridType **prefetch_previousStripe;
        GridType **prefetch_successorStripe;
        int no_cells_in_stripe= 0;
        int stripeID = 0;

    public:
        bool initalized = false;
        Grid(int setx, int sety) {
            //Use this constructor if you use StencilForTxHPC
            dimensions = 2;
            x = setx;
            y = sety;
        }
        Grid(int setx, int sety, GridType *memoryPointer) {
            //Use this constructor if you dont use StencilForTxHPC
            dimensions = 2;
            x = setx;
            y = sety;

            //Convert pointer to 2d pointer
            dataPointer2D = new GridType*[setx];
            for (int k = 0; k < setx; ++k) {
                dataPointer2D[k]  = &memoryPointer[sety * k];
            }

            //testing the environment:
            //TODO Make a unit test out of this.
            for (int x = 0; x < setx; ++x) {
                for (int y = 0; y < sety; ++y) {
                    dataPointer2D[x][y] = y;
                }
            }
            for (int x = 0; x < setx; ++x) {
                for (int y = 0; y < sety; ++y) {
                    if(dataPointer2D[x][y] == y){
                        dataPointer2D[x][y] = 0; //Recover the init state
                    }
                    else{
                        std::cout << "Something is wrong with the memory that you provided." << std::endl;
                    }
                }
            }

        }

        Grid(int setx, int sety, int setz, GridType *memoryPointer) {
            //Init values
            dimensions = 3;
            x = setx;
            y = sety;
            z = setz;

            //Convert memoryPointer to 3D pointer
            dataPointer3D = new GridType** [setx];
            for (int i = 0; i < setx; ++i) {
                dataPointer3D[i] = new GridType* [sety];
                for (int j = 0; j < sety; ++j) {
                    dataPointer3D[i][j] = &memoryPointer[sety * setz * i + setz * j];
                }
            }
        }

        void config_GridForTxHPC(int set_stripeID, GridType *set_currentStripe, GridType **set_prefetch_previousStripe, GridType **set_prefetch_successorStripe, int set_no_cells_in_stripe){

//            std::cout << set_prefetch_successorStripe[0][0] << std::endl;
//            std::cout << "previous_stripe" << std::endl;
//            for (int i = 0; i < 704; ++i) {
//                std::cout << set_prefetch_previousStripe[0][i];
//            }
//            std::cout << std::endl;

            stripeID = set_stripeID;
            currentStripe = set_currentStripe;
            prefetch_previousStripe = set_prefetch_previousStripe;
            prefetch_successorStripe = set_prefetch_successorStripe;
            no_cells_in_stripe= set_no_cells_in_stripe;
        }

        void initGrid(GridType** (array), int minx, int maxx, int miny, int maxy){
            for (int i = minx; i < maxx; ++i) {
                for (int j = miny; j < maxy; ++j) {
                    double temp = array[i][j];
                    this->dataPointer2D[i][j] = temp;
                }
            }
        }

        void initGrid(GridType*** (array), int minx, int maxx, int miny, int maxy, int minz, int maxz){
            for (int i = minx; i < maxx; ++i) {
                for (int j = miny; j < maxy; ++j) {
                    for (int k = minz; k < maxz; ++k) {
                        double temp = array[i][j][k];
                        this->dataPointer3D[i][j][k] = temp;
                    }
                }
            }
        }

        void Emit(GridType value, GridPointer g) {
            if (dimensions == 2) {
                this->dataPointer2D[g.get_x()][g.get_y()] = value;
            }
            else if (dimensions == 3) {
                this->dataPointer3D[g.get_x()][g.get_y()][g.get_z()] = value;
            }
        }

        GridType getCell(int x, int y, int z) {
            GridType temp = dataPointer3D[x][y][z];
            return temp;
        }

        GridType getCell(int x, int y) {
            GridType temp = dataPointer2D[x][y];
            return temp;
        }

        GridType cell(int x, int y){
            //Need to translate this to the lin values and get the pointer

            int lin = trans_2D_to_lin(x, y, this->gety());

            //Getting the stripeID of the points
            std::pair<int, int> cellInStripe = lin_to_offset_stripeID(lin, no_cells_in_stripe);

            //Check if cell is in current stripe
            if(stripeID == cellInStripe.second){
                std::cout << "Cell is in current stripe" << std::endl;


                return currentStripe[cellInStripe.first];
            }else if((stripeID) < cellInStripe.second){
                std::cout << "I am in " << stripeID << ", Cell can be found in successor stripe" << std::endl;
                int findStripeID = (cellInStripe.second - stripeID) - 1;
                if((findStripeID > 0) || (findStripeID < 0)){
                    handle_error("ACCESSING MORE THAN 1 PREFETCH STRIPE.");
                }

                return prefetch_successorStripe[findStripeID][cellInStripe.first];
            }else if((stripeID) > cellInStripe.second){
                std::cout << "Cell can be found in previous stripe" << std::endl;
                int findStripeID = (stripeID - cellInStripe.second) - 1;
                if((findStripeID > 0) || (findStripeID < 0)){
                    handle_error("ACCESSING MORE THAN 1 PREFETCH STRIPE.");
                }

                return prefetch_previousStripe[findStripeID][cellInStripe.first];
            }else{
                //(current) stripeID - searchingForCellInStripe
                int searchingForStripe = stripeID - cellInStripe.second;
                std::cout << "Need access to stripe: " << searchingForStripe << std::endl;
                handle_error("Error, cell is out of bound: Cell is not away 1 stripe.");
            }
            return 0;
        }


        int getx(){
            return x;
        }
        int gety(){
            return y;
        }
        int getz(){
            return z;
        }
        int getdimensions(){
            return dimensions;
        }


    };

//Represents a dimension
    class Domain {
    private:
        int min_x = 0, min_y = 0, min_z = 0;
        int max_x = 0, max_y = 0, max_z = 0;
        int dimensions = 0;
    public:
        Domain(int setmin_x, int setmax_x, int setmin_y, int setmax_y, int setmin_z, int setmax_z) {
            min_x = setmin_x;
            min_y = setmin_y;
            min_z = setmin_z;
            max_x = setmax_x;
            max_y = setmax_y;
            max_z = setmax_z;
            dimensions = 3;
        }

        Domain(int P1_x, int P1_y, int P2_x, int P2_y) {

            min_x = P1_x;
            min_y = P1_y;

            max_x = P2_x;
            max_y = P2_y;
            dimensions = 2;
        }

        Domain(int setmin_x, int setmax_x){
            min_x = setmin_x;
            max_x = setmax_x;
            dimensions = 1;
        }
        Domain(){
            dimensions = 0; // NOT INIT.
        }

        int minx() {
            return min_x;
        }

        int miny() {
            return min_y;
        }

        int minz() {
            return min_z;
        }

        int maxx() {
            return max_x;
        }

        int maxy() {
            return max_y;
        }

        int maxz() {
            return max_z;
        }

        int getNumberDim() {
            return dimensions;
        }

        bool boundaryCheck(int checkX, int checkY, int checkZ) {
            if ((min_x > checkX) || (max_x < checkX)) {
                return false;
            }
            else if ((min_y > checkY) || (max_y < checkY)) {
                return false;
            }
            else if ((min_z > checkZ) || (max_z < checkZ)) {
                return false;
            }
            //is in boundary
            return true;
        }

        bool boundaryCheck(GridPointer p) {
            int checkX = p.get_x(), checkY = p.get_y(), checkZ = p.get_z();
            if ((min_x > checkX) || (max_x < checkX)) {
                return false;
            }
            else if ((min_y > checkY) || (max_y < checkY)) {
                return false;
            }
            else if ((min_z > checkZ) || (max_z < checkZ)) {
                return false;
            }
            //is in boundary
            return true;
        }
    };

    template<class TypeOfGrid>
    class Stencil {
    private:
        Domain *d;
        Grid<TypeOfGrid> *g;
        int iterationsOfStencil = 0;
//        void (*iterate_compute)(void * args, char * cur_stripe, Meta * meta_info, int stripe_id, char* load, char** before, char** after);
//        void (*control_function)(void * args, meta * meta_info);
    public:
        //Without TxHPC
        void (*kernel2d)(int, int, GridPointer, Grid<TypeOfGrid>);
        void (*kernel3d)(int, int, int, GridPointer, Grid<TypeOfGrid>);

        //With TxHPC
        double (*kernel2d_TxHPC)(int, int, Grid<TypeOfGrid>);
        double (*getinitFunction)();

        //Init Stencil
        Stencil(void (*set_kernel2D)(int, int, GridPointer, Grid<TypeOfGrid>), Grid<TypeOfGrid> *gridToWorkOn,
                Domain *domainToWorkIn) {
            this->g = gridToWorkOn;
            this->d = domainToWorkIn;
            kernel2d = set_kernel2D;
        }
        //Init Stencil
        Stencil(void (*set_kernel3D)(int, int, int, GridPointer, Grid<TypeOfGrid>), Grid<TypeOfGrid> *gridToWorkOn,
                Domain *domainToWorkIn) {
            g = gridToWorkOn;
            d = domainToWorkIn;
            kernel3d = set_kernel3D;
        }

        //Init Stencil
        Stencil(double (*set_kernel2D_TxHPC)(int, int, Grid<TypeOfGrid>), Grid<TypeOfGrid> *gridToWorkOn,
                Domain *domainToWorkIn) {
            this->g = gridToWorkOn;
            this->d = domainToWorkIn;
            kernel2d_TxHPC = set_kernel2D_TxHPC;
        }

        Domain* getDomain(){
            return d;
        }
        Grid<TypeOfGrid>* getGrid(){
            return g;
        }

        void setIteations(int i){
            this->iterationsOfStencil = i;
        }

        int getIterations(){
            return iterationsOfStencil;
        }

        void setInitFunction(double (*initfunction)(void)){
            getinitFunction  = initfunction;
        }

        //Runs the Stencil
        void run_withoutTxHPC() {

            //Run Stencil for each point, defined in the domain
            //Checks if domain is 2d, or 3d and then runs the kernel
            //kernel must be set, if not, an error will be thrown
            switch (d->getNumberDim()) {
                case 2:
                    for (int x = d->minx(); x < d->maxx(); ++x) {
                        for (int y = d->miny(); y < d->maxy(); ++y) {
                            GridPointer gP(x, y);
                            //TODO Check if kernel is defined
                            (*kernel2d)(x, y, gP, *g);
                        }
                    }
                    break;
                case 3:
                    for (int x = d->minx(); x < d->maxx(); ++x) {
                        for (int y = d->miny(); y < d->maxy(); ++y) {
                            for (int z = d->minz(); z < d->maxz(); ++z) {
                                GridPointer gP(x, y, z);
                                //TODO Check if kernel is defined
                                (*kernel3d)(x, y, z, gP, *g);
                            }
                        }
                    }
                    break;
                default:
                    break;
            }
        }


        void iterate(int i){
            for (int j = 0; j < i; ++j) {
                this->run_withoutTxHPC();
            }
        }



        void printGrid(){
            switch (this->g[0].getdimensions()){
                case 2:
                    std::cout << "Printing the whole Grid." << std::endl;
                    for (int a = 0; a < this->g->getx(); ++a) {
                        std::cout << "Row " << a << ":";
                        for (int b = 0; b < this->g->gety(); ++b) {
                            std::cout << "[" << this->g->getCell(a, b) << "]";
                        }
                        std::cout << std::endl;
                    }
                    std::cout << std::endl;
                    break;
                case 3:
                    std::cout << "Print 3D is not supported yet." << std::endl;
                    std::cout << "Therefore, I am printing just the walls starting from the first to the last" << std::endl;

                    for (int i = 0; i < this->g->getx(); ++i) {
                        for (int j = 0; j < this->g->gety(); ++j) {
                            for (int k = 0; k < this->g->getz(); ++k) {
                                std::cout << this->g->getCell(i, j, k) << " ";
                            }
                            std::cout << std::endl;
                        }
                        std::cout << std::endl;
                    }
                    break;
                default:
                    std::cout << "Dimension " << this->g->getdimensions() << " not supported." << std::endl;
                    break;
            }
        }

        void printDomain(Domain* d){
            switch (this->g[0].getdimensions()){
                case 2:
                    std::cout << "Printing Dimension in 2D." << std::endl;
                    for (int a = this->d->minx(); a < this->d->maxx(); ++a) {
                        std::cout << "Row " << a << ":";
                        for (int b = this->d->miny(); b < this->d->maxy(); ++b) {
                            std::cout << "[" << this->g->getCell(a, b) << "]";
                        }
                        std::cout << std::endl;
                    }
                    std::cout << std::endl;
                    break;
                case 3:
                    std::cout << "Print 3D is not supported yet." << std::endl;
                    std::cout << "Therefore, I am printing just [0][0][x]" << std::endl;

                    for (int i = d->minx(); i < d->maxx(); ++i) {
                        for (int j = d->miny(); j < d->maxy(); ++j) {
                            for (int k = d->minz(); k < d->maxz(); ++k) {
                                std::cout << this->g->getCell(i, j, k) << " ";
                            }
                            std::cout << std::endl;
                        }
                        std::cout << std::endl;
                    }
                    break;
                default:
                    std::cout << "Dimension " << this->g->getdimensions() << " not supported." << std::endl;
                    break;
            }
        }

        void VisIt(){

        }
        void MPI(){

        }

#pragma mark StencilForTxHPC Integration


        void iterate_WithTxHPC(int i){
            for (int j = 0; j < i; ++j) {
                this->run_withTxHPC();
            }
        }

        static void raid6_subgridAPI(int amountOfStripes, size_t stripe_size, int* stripe){
            //Runs the kernel once. Use iterate if you want to run it multiple times
            //TODO pass the kernel function! and call it with each point in the domain

        }



        //Runs the Stencil with StencilForTxHPC
        void run_withTxHPC(int iterations) {
            this->setIteations(iterations);
            char* stencilObj = static_cast<char*>(static_cast<void*>(this));
//            long sizeOfGrid = sizeof(this);
//            dumpptr[sizeOfGrid] = static_cast<char*>(static_cast<void*>(this->g));


            run(&(iterate_compute), stencilObj, &(StencilForTxHPC::control_function), stencilObj);
            //run_test(&Stencil<double>::*(kamesh));
        }

        void init(char** memoryPointer, int amountOfPointer, long sizeOfEachShelf, int prefetchSize, long sizeOfScratchPad){
            //In StencilForTxHPC the two dimensional grid is split into blocks and cells
            //a block contains multiple cell -> array[block][cell]

#pragma mark GATHER INPUT PARAMETERS

#ifdef JERASURE
            std::cout << "JERASURE FLAG WORKS. " << std::endl;
            //Prime number which is bigger than 8 shelfs
            int primeNumber = 11;
            int wordSize = 11;
            long no_of_words_in_a_block = 8;

            int blocksize = no_of_words_in_a_block * wordSize * sizeof(TypeOfGrid);
#else
            //non-JERASURE REED-Solman
            int wordSize = 32;
            int no_of_words_in_a_block = 32;
            int blocksize = wordSize * no_of_words_in_a_block;

#endif
            //Get number of devices
            double amountOfDevices = amountOfPointer-1; // 11 shelfs in total , 10 without metadata shelf
            double total_devices_WITHOUT_METASHELF = amountOfDevices;
            double no_of_data_devices = amountOfDevices-2; //2 parities
            double no_of_coding_devices = amountOfDevices-no_of_data_devices; //Remaining 2 Shelfs are for parity

            double gridSize = ceil(((g->getx() *  g->gety() * sizeof(TypeOfGrid)) * 1.048576));  //50MB

            double size_of_each_device = ceil((gridSize/no_of_data_devices));
            double no_of_stripes = ceil(size_of_each_device/blocksize);

            int prefetch_after = prefetchSize;
            int prefetch_before = prefetchSize;

            int memory_all_a = (gridSize * (amountOfDevices/no_of_data_devices))+1;
            double memory_all_b =  (no_of_stripes + prefetch_after + prefetch_before + 1);
            int memory_allocate = ceil((memory_all_a * memory_all_b)/no_of_stripes);

            memory_allocate += 3000;

            //long memory_allocate = ((((gridSize * ((amountOfDevices)/no_of_data_devices))+1) * (no_of_stripes + prefetch_after + prefetch_before + 1))/ no_of_stripes);



            long new_size_of_each_device = memory_allocate/total_devices_WITHOUT_METASHELF;

            int scratchPad = sizeOfScratchPad; //Minimum size of metadata running without scratchpad
            int crash_recovery = 0; //no crash occurred -> 0
/*
            //(I) Find right number of block size
//             int blocks = g->getx();
//             int cells =  g->gety();
//             int numberOfBlocksInAStripe = (blocks / amountOfDataDevices)+1;
//             blocks = (numberOfBlocksInAStripe * amountOfDataDevices);




             //(II) Find total size
             long totalMemory_MinusMetaInfoAndScratchPad = (blocks * cells) * sizeof(TypeOfGrid);

             //(III) Size of one block
             //long sizeOfOneBlock = wordSize * sizeof(TypeOfGrid);

             //(IV)
             int wordsInABlock = ((cells * amountOfDataDevices) / wordSize)+1;

             //(V) Get the total amount of memory needed
             long totalMemoryNeeded = wordsInABlock * blocks * wordSize;

             //(VI) Add Parity
             totalMemoryNeeded = totalMemoryNeeded * 1.25;

             int prefetchBefore = 1;
             int prefetchAfter = 1;

             //(VII) Add space for prefetch and prefetchAfter
             totalMemoryNeeded = totalMemoryNeeded + (prefetchAfter + prefetchBefore + 1) * amountOfDataDevices * (wordsInABlock*wordSize);


             if((sizeOfEachShelf*amountOfPointer) < totalMemoryNeeded){
                 handle_error("Provided memory is not big enough.");
             }





            */
            /* //Get number of devices
            int amountOfDevices = amountOfPointer-1; // 11 shelfs in total
            int total_devices_WITHOUT_METASHELF = amountOfDevices;
            int amountOfDataDevices = amountOfDevices-2; //2 parities
            int no_of_coding_devices = amountOfDevices-amountOfDataDevices; //Remaining 2 Shelfs are for parity

            //Prime number which is bigger than 8 shelfs
            int primeNumber = 11;
            if(!(primeNumber > amountOfDataDevices)){
                handle_error("error, prime number is not bigger than data devices.");
            }
            int wordSize = 11 * sizeof(TypeOfGrid);

            //(I) Find right number of block size
            int blocks = g->getx();
            int cells =  g->gety();
            int numberOfBlocksInAStripe = (blocks / amountOfDataDevices)+1;
            blocks = (numberOfBlocksInAStripe * amountOfDataDevices);

            //(II) Find total size
            long totalMemory_MinusMetaInfoAndScratchPad = (blocks * cells) * sizeof(TypeOfGrid);

            //(III) Size of one block
            //long sizeOfOneBlock = wordSize * sizeof(TypeOfGrid);

            //(IV)
            int wordsInABlock = ((cells * amountOfDataDevices) / wordSize)+1;

            //(V) Get the total amount of memory needed
            long totalMemoryNeeded = wordsInABlock * blocks * wordSize;

            //(VI) Add Parity
            totalMemoryNeeded = totalMemoryNeeded * 1.25;

            int prefetchBefore = 1;
            int prefetchAfter = 1;

            //(VII) Add space for prefetch and prefetchAfter
            totalMemoryNeeded = totalMemoryNeeded + (prefetchAfter + prefetchBefore + 1) * amountOfDataDevices * (wordsInABlock*wordSize);


            if((sizeOfEachShelf*amountOfPointer) < totalMemoryNeeded){
                handle_error("Provided memory is not big enough.");
            }


            int scratchPad = 1024; //Minimum size of metadata running without scratchpad
            int crash_recovery = 0; //no crash occurred -> 0*/

#pragma mark Translate int/longs to char*
            std::string totalMemory = std::to_string(memory_allocate);
            std::string no_of_data_devices_string = std::to_string((int)no_of_data_devices); //8 Shelfs are for coding
            std::string wordSize_string = std::to_string(primeNumber);
            std::string no_of_coding_devices_string = std::to_string((int)no_of_coding_devices);
            std::string no_of_words_in_a_block_string = std::to_string(no_of_words_in_a_block);
            std::string prefetchBefore_string = std::to_string(prefetch_before);
            std::string prefetchAfter_string = std::to_string(prefetch_after);
            std::string scratchPad_string = std::to_string(scratchPad);
            std::string crash_recovery_string = std::to_string(crash_recovery);

            //Translation into the API
            int inputParametersCount = 10;
            char* inputParameters[inputParametersCount];
            inputParameters[1] = (char*)(totalMemory.c_str());
            inputParameters[2] = (char*) no_of_data_devices_string.c_str();
            inputParameters[3] = (char*)wordSize_string.c_str();
            inputParameters[4] = (char*)no_of_coding_devices_string.c_str();
            inputParameters[5] = (char*)no_of_words_in_a_block_string.c_str();
            inputParameters[6] = (char*)prefetchBefore_string.c_str();
            inputParameters[7] = (char*)prefetchAfter_string.c_str();
            inputParameters[8] = (char*)scratchPad_string.c_str();
            inputParameters[9] = (char*)crash_recovery_string.c_str();

            initialize_raid6_parameters(inputParametersCount, inputParameters, memoryPointer, amountOfPointer);
        };
    };


#pragma mark FUNCTIONS FOR ITERARTIONS
    template <class TypeOfGrid>
    static Domain translate_Domain_To_Linear_Cell(Domain* domain_to_translate, Grid<TypeOfGrid>* g, int stripeID, int cellsInStripe){
        switch(domain_to_translate->getNumberDim())
        {
            case 2: {
                //2D Grid
                //Converting the points in the dimension
                int minP1 = trans_2D_to_lin(domain_to_translate->minx(), domain_to_translate->miny(), g->gety());
                int maxP1 = trans_2D_to_lin(domain_to_translate->maxx(), domain_to_translate->maxy(), g->gety());

                //Getting the stripeID of the points
                std::pair<int, int> minP = lin_to_offset_stripeID(minP1, cellsInStripe);
                std::pair<int, int> maxP = lin_to_offset_stripeID(maxP1, cellsInStripe);


                //Check if StripeID of minP is below the current stripeID
                if (minP.second < stripeID) {

                    if (maxP.second > stripeID) {
                        //all values of the current stripeID will be used
                        Domain allValues(0, cellsInStripe);
                        return allValues;
                    }
                    else if (maxP.second == stripeID) {
                        //Upper maximum StripeID is the same as the current stripe!
                        //Take the offset of maxP
                        Domain specificValues(0, maxP.first);
                        return specificValues;
                    }
                    else {
                        //Upper limit reached. These stripes are not in the domain

                        return Domain();
                    }
                }
                else if(minP.second == stripeID){
                    //Currently in that stripe so we need to take the Offset
                    Domain offsetValues(minP.first, cellsInStripe);
                    return offsetValues;
                }
                else if (minP.second > stripeID) {
                    //This means that the data will not be processed
                    std::cout << "Stripe will not be considered, not in domain. Stripe: " << stripeID << std::endl;
                    return Domain();
                }
            }
                break;
            case 3:
                handle_error("3 dimensions not supported yet.");
                break;
        }
    }



    static void iterate_compute(void* args, char * cur_stripe, Meta * meta_info, int stripe_id, char* load, char** before, char** after) {
        int numberOfBlocks = meta_info->no_of_data_devices;
        int numberOfCellsInABlock = (meta_info->blocksize / sizeof(long));
        int no_of_cells_in_a_stripe = numberOfCellsInABlock * numberOfBlocks;
        //Debug:
        long sizeOfStripe = meta_info->datastripesize;
        long sizeOfStripe2 = numberOfBlocks * meta_info->blocksize;
        long no_stripes = meta_info->no_of_stripes - (meta_info->prefetch_after + meta_info->prefetch_before + 1);//Metadata, Partities
        long no_of_total_cells = no_of_cells_in_a_stripe * no_stripes;
        std::cout << "I have access to " << no_stripes << " stripes. " << "This is stripe# :" << stripe_id << std::endl;
//
//            std::cout << "Each stripe has " << no_of_cells_in_a_stripe << " Cells to access. Testing access. " << std::endl;
//            std::cout << "There are " << no_of_total_cells << " amount of cells and this is " <<  no_of_total_cells*sizeof(long) << " Byte." << std::endl;

        double *cellpointer = (double *) cur_stripe;
        double **prefetch_before = (double **) before;
        double **prefetch_after = (double **) after;

//        scratchpad = (long long unsigned *)get_scratchpad();

        if(1 != meta_info->prefetch_after){
            handle_error("prefetch is not working correctly");
        }

        //Set up prefetch pointers so one can use them properly
        prefetch_after = (double **) malloc(sizeof(double *) * meta_info->prefetch_after);
        prefetch_before = (double **) malloc(sizeof(double *) * meta_info->prefetch_before);

//        prefetch_after = new double*[meta_info->prefetch_after]; //(sizeof(double *) * meta_info->prefetch_after);
//

        for(int i = 0; i < meta_info->prefetch_before; i++)
        {
            if((*before) != NULL)
            {
                prefetch_before[i] = (double*)(before[i]);

                for (int j = 0; j <704; ++j) {
                    std::cout << prefetch_before[0][j];
                }std::cout << std::endl;

            }
            else{
                std::cout << "Prefetch before is null." << std::endl;
            }
        }

        for(int i = 0; i < meta_info->prefetch_after; i++)
        {
            if((*after) != NULL)
            {
                prefetch_after[i] = (double*)(after[i]);
                for (int j = 0; j <704; ++j) {
                    std::cout << prefetch_after[0][j];
                }std::cout << std::endl;
            }
            else{
                std::cout << "Prefetch after is null." << std::endl;
            }
        }



//        for (int j = 0; j < meta_info->prefetch_after; ++j) {
//            prefetch_after[j] = (double *) after[j];
//            std::cout << prefetch_after[j][4] << std::endl;
//            if(*after == NULL){
//                std::cout << "After is null." << std::endl;
//            }
//        }
//        for (int k = 0; k < meta_info->prefetch_before; ++k) {
//            prefetch_before[k] = (double *) after[k];
//            std::cout << prefetch_before[k][4] << std::endl;
//            if(*before == NULL){
//                std::cout << "prefetch is null" << std::endl;
//            }
//        }


        //Get the Metainformation of the application
        Stencil<double> *currentStencil = static_cast<Stencil<double> *>(args);
        Domain *currentDomain = currentStencil->getDomain();
        Grid<double> *currentGrid = currentStencil->getGrid();
        //Get the domain to work in
        Domain valuesToProcess = translate_Domain_To_Linear_Cell<double>(currentDomain, currentGrid, stripe_id,
                                                                         no_of_cells_in_a_stripe);

        std::cout << std::endl;
        currentGrid->config_GridForTxHPC(stripe_id, (double *) cellpointer, prefetch_before, prefetch_after,
                                         no_of_cells_in_a_stripe);

        if ((currentGrid->initalized) && (meta_info->counter % 2 == 0)) {
            //PROCESS DOMAIN
            if (valuesToProcess.getNumberDim() != 0) {
                //If the domain exists, this stripe does contain values to work in
                for (int i = valuesToProcess.minx(); i < valuesToProcess.maxx(); ++i) {
                    //Translating back from Cell:Stripe 2D Array
                    int cell = trans_2D_to_lin(stripe_id, i, no_of_cells_in_a_stripe);
                    std::pair<int, int> point2d = trans_lin_to_2D(cell, currentGrid->gety());
                    std::cout << "Working on point X:" << point2d.first << " Y:" << point2d.second << std::endl;
                    cellpointer[i] = currentStencil->kernel2d_TxHPC(point2d.first, point2d.second, *currentGrid);


                }
            }
        }
        else {
            //INIT DOMAIN
            if(stripe_id == (no_stripes-1)){
                //Last stripe
                currentGrid->initalized = true;
            }
            if (valuesToProcess.getNumberDim() != 0) {
                //Not the whole domain needs to be initialized
                if (valuesToProcess.maxx() == no_of_cells_in_a_stripe){
                    if(valuesToProcess.minx() == 0){
                        //all values in this stripe are in the domain - so I am not going to init these
                    }
                    else{
                        for (int i = 0; i < valuesToProcess.minx(); ++i) {
                            cellpointer[i] = currentStencil->getinitFunction();
                        }
                    }
                }else{
                    //values at the end need to be initialized
                    if(valuesToProcess.minx() == 0){
                        for (int i = valuesToProcess.maxx(); i < no_of_cells_in_a_stripe; ++i) {
                            cellpointer[i] = currentStencil->getinitFunction();
                        }
                    }
                    else{
                        //Values at the end and values at the beginning need to be init.
                        for (int i = valuesToProcess.maxx(); i < no_of_cells_in_a_stripe; ++i) {
                            cellpointer[i] = currentStencil->getinitFunction();
                        }
                        for (int j = 0; j < valuesToProcess.minx(); ++j) {
                            cellpointer[j] = currentStencil->getinitFunction();
                        }
                    }
                }
            }else{
                //The whole domain needs to be initialized
                for (int i = 0; i < no_of_cells_in_a_stripe; ++i) {
                    double tmp = currentStencil->getinitFunction();
                    cellpointer[i] = tmp;
                }
            }
        }
        if(print_flag){
            int firstcell = trans_2D_to_lin(stripe_id, 0, no_of_cells_in_a_stripe);
            std::pair<int, int> point2d = trans_lin_to_2D(firstcell, currentGrid->gety());
            int lastcell = trans_2D_to_lin(stripe_id, no_of_cells_in_a_stripe, no_of_cells_in_a_stripe);
            std::pair<int, int> point2d2 = trans_lin_to_2D(lastcell, currentGrid->gety());

            int lastcellToProcess = trans_2D_to_lin(stripe_id, valuesToProcess.maxx(), no_of_cells_in_a_stripe);
            std::pair<int, int> point2dLastCellToProcess = trans_lin_to_2D(lastcellToProcess, currentGrid->gety());
            std::cout << "This stripe contains: P1[" << point2d.second << "][" << point2d.first << "]" << " - " << "P2[" << point2d2.second  << "][" << point2d2.first << "] Last Cell To Process: [" << point2dLastCellToProcess.second << "][" << point2dLastCellToProcess.first << "]" << std::endl;


            for (int i = 0; i < no_of_cells_in_a_stripe; ++i) {
                std::cout << cellpointer[i];
                shitcounter++;
                if(currentGrid->gety() == shitcounter){
                    shitcounter = 0;
                    std::cout << std::endl;
                }
//                    if((i+1) == valuesToProcess.maxx()){
//                        std::cout <<std::endl<< "The following values aren't processed in this stripe:";
//                    }
            }
            std::cout << std::endl;
        }
    }

    static void control_function(void * args, meta * meta_info) {
        Stencil<double> *currentStencil = static_cast<Stencil<double> *>(args);
        if(meta_info->counter == (currentStencil->getIterations() -1)) //Counter starts at 0
        {
            meta_info->end = true;
        }
    };
}

#endif //FASD_PHYSIS_API_H
