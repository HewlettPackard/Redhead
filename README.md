# TAS

Resilience for The Machine.

To use this code run the following commands. :high_brightness:

```bash
cd bin
cmake ../.
make
./run_applications %APPLICATION_TYPE %ITERATION_SIZE %MEMORY_TYPE

#To plot the graph (Activate X11, gnuplot must be installed (I use VM45))
gnuplot Script_%APPLICATION_Name --persist
```

APPLICATION_TYPE:
* jf = Jacobi Iteration
* gf = GameOfLife
* gf_IT = GameOfLife With More Computation
* sm = SparseMatrix
* ac = AccessTimes
* jf_scale = Jacobi Iteration Scaling Up to 1TB

%ITERATION_SIZE
* 1
* 100
* 1000 (Everything above will take too much time.)

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
