# TAS

Resilience for The Machine.

To use this code run the following commands. :high_brightness:

```bash
cd bin
cmake ../.
make
./run_applications %APPLICATION_TYPE %ITERATION_SIZE %OPTION_GRIDSIZE

#To plot the graph (Activate X11, gnuplot must be installed (I use VM45))
gnuplot Script_%APPLICATION_Name --persist
```

APPLICATION_TYPE:
* jf = Jacobi Iteration
* gf = GameOfLife
* gf_computation = GameOfLife With More Computation
* sm = SparseMatrix
* ac = AccessTimes
* jf_scale = Jacobi Iteration Scaling Up to 1TB

%ITERATION_SIZE
* 1
* 100
* 1000 (Everything above will take too much time.)

%OPTION_GRIDSIZE
Use this parameter if application jf_scale was chosen to set the gridsize: (Use the right numbers)
* 1GB = 11 180
* 10GB = 35356
* 100GB = 111 804
* 250GB = 176776
* 500GB = 250 000
* 1000TB = 353 553

__Onkar will deliver here:__
*   /include
     * -r6_utils.h 
*   /benchmarks 
     * -access.h 
     * -timeForEachStep.h
     * -overheadOfReconstructAFile.h
*   /tests
     * /unit_test
       * -memory_availability.h
       * -SoC_tests.h
       * -hello_world.h
*   /docs
     * /RAID_6
       * -technicalPaper.pdf //Transactional HPC
       * -overallPaper.pdf //Paper that summerizes everything
       * -poster_onkar.pdf
*   /literature
     * -screaming_fast_galois_field_codes.pdf //Paper: which libpmem functions should be used

__Mesut will deliver here:__
*   /include
    * /fasd
    * /physis_api.h
    * /portable_pointer
*   /benchmarks 
    * -diffusion.h
    * -fibonacci.h
    * -wave.h
    * -access.h
    * -crash_recover.h 
    * -wipe_memory.h
*   /example
    * -helloWorld.h
    * -diffusion.h
*   /docs
    * /stencils
      * -60 page - thesis
      * -15 pages paper
*   /literature
    * -physis
    * -GVR


    iajsdpfojasdpiofjpioasdjpfiodasjoifjaposdjspodf
