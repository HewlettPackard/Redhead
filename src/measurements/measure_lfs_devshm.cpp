#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <cmath>

using namespace std;

#define LFS_DIR "/lfs"
#define DEV_SHM "/dev/shm"
#define BYTES_IN_KB 1024L
#define MAX_PATH_LENGTH 1024
#define MAX_STRING_LENGTH 2048
#define BILLION 1000000000L
#define TIME_SPENT(start, end) (BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec)

enum stats {
    FLAGS = 0,
    MINOR_FAULTS = 1,
    MINOR_FAULTS_CHILDREN = 2,
    MAJOR_FAULTS = 3
};

void usage() {
    static char usageStr[] = "\n-----------------------------------------USAGE-----------------------------------------\n"
            "pageFault numPages \n"
            "testSize    - The amount of data to test. Format is: num_[KB/MB/GB]. \n"
            "\n\n";
    fprintf(stderr, usageStr);
    exit(1);
}


/*
    Check if a flag has been changed after testing
 */
void
test_flag(const char *flagStr, int flagVal, int oldFlags, int changedFlags)
{
    int pretestFlagVal = flagVal & oldFlags;

    if(1 == (flagVal & changedFlags)) {
        printf("%s changed to %s\n", flagStr, !pretestFlagVal ? "TRUE" : "FALSE");
    }
}

/**
	Given a string that contains the quantity, parse and calculate the appropriate size

	1KB = 1024 bytes, 1MB = 1024KB, 1GB = 1024MB
	so a quantity string "2_GB" will be calculated as 2 * 1024 * 1024 * 1024 bytes
*/
uint64_t getTestSize(char *quantityStr) {

    //duplicate the quantity string and use strsep to extract the quantity from the size
    char *str = strdupa(quantityStr);
    char *quantity = strsep(&str, "_");
    char *size = strsep(&str, "_");

    double sizeFactor = 0;

    if(strncasecmp(size, "KB", 2) == 0) {
        sizeFactor = 1;
    }else if(strncasecmp(size, "MB", 2) == 0) {
        sizeFactor = 2;
    }else if(strncasecmp(size, "GB", 2) == 0) {
        sizeFactor = 3;
    }else{
        printf("Size %s is not recognized or is too big! Try something smaller", quantityStr);
        exit(EXIT_FAILURE);
    }

    return atof(quantity) * pow(BYTES_IN_KB, sizeFactor);
}//end getTestSize


/*
    Check whether process flags have changed after testing
 */
void
show_changed_flags(int oldFlags, int changedFlags)
{
    test_flag("CSIGNAL", CSIGNAL, oldFlags, changedFlags);
    test_flag("CLONE_IO", CLONE_IO, oldFlags, changedFlags);
    test_flag("CLONE_VM", CLONE_VM, oldFlags, changedFlags);
    test_flag("CLONE_FS", CLONE_FS, oldFlags, changedFlags);
    test_flag("CLONE_VFORK", CLONE_VFORK, oldFlags, changedFlags);
    test_flag("CLONE_FILES", CLONE_FILES, oldFlags, changedFlags);
    test_flag("CLONE_NEWNS", CLONE_NEWNS, oldFlags, changedFlags);
    test_flag("CLONE_PTRACE", CLONE_PTRACE, oldFlags, changedFlags);
    test_flag("CLONE_SETTLS", CLONE_SETTLS, oldFlags, changedFlags);
    test_flag("CLONE_PARENT", CLONE_PARENT, oldFlags, changedFlags);
    test_flag("CLONE_THREAD", CLONE_THREAD, oldFlags, changedFlags);
    test_flag("CLONE_NEWUTS", CLONE_NEWUTS, oldFlags, changedFlags);
    test_flag("CLONE_NEWIPC", CLONE_NEWIPC, oldFlags, changedFlags);
    test_flag("CLONE_NEWPID", CLONE_NEWPID, oldFlags, changedFlags);
    test_flag("CLONE_NEWNET", CLONE_NEWNET, oldFlags, changedFlags);
    test_flag("CLONE_SIGHAND", CLONE_SIGHAND, oldFlags, changedFlags);
    test_flag("CLONE_SYSVSEM", CLONE_SYSVSEM, oldFlags, changedFlags);
    test_flag("CLONE_NEWUSER", CLONE_NEWUSER, oldFlags, changedFlags);
    test_flag("CLONE_DETACHED", CLONE_DETACHED, oldFlags, changedFlags);
    test_flag("CLONE_UNTRACED", CLONE_UNTRACED, oldFlags, changedFlags);
    test_flag("CLONE_CHILD_SETTID", CLONE_CHILD_SETTID, oldFlags, changedFlags);
    test_flag("CLONE_PARENT_SETTID", CLONE_PARENT_SETTID, oldFlags, changedFlags);
    test_flag("CLONE_CHILD_CLEARTID", CLONE_CHILD_CLEARTID, oldFlags, changedFlags);
} //end show_changed_flags


/*
    Initialize and randomize page access order
 */
uint64_t*
initialize_access_order(unsigned long numPages, int pageSz)
{
    uint64_t *accessOrder = new uint64_t[numPages];

    // Init access order
    for(uint64_t i = 0 ; i < numPages; i++) {
        accessOrder[i] = i*pageSz;
    }

    // Shuffle access order;
    for(uint64_t i = 0 ; i < numPages; i++) {
        int r = rand() % numPages;
        uint64_t tmp = accessOrder[i];
        accessOrder[i] = accessOrder[r];
        accessOrder[r] = tmp;
    }

    return accessOrder;
} //end initialize_access_order


/*
    Testing faulting the given number of pages and measure the memory access times. Returns the total time
 */
uint64_t
test_page_faults(const char *dir, uint64_t totalSz, unsigned long numPages, uint64_t *accessOrder)
{
    char *testFile;
    //appending LFS dir to filename
    if (asprintf(&testFile, "%s/test_%lu", dir, numPages) == -1) {
        testFile = NULL;
        perror("unable to concatenate file name to LFS dir");
        exit(EXIT_FAILURE);
    }

    // Create file
    int fd = open(testFile,O_RDWR|O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(fd < 0)
        perror("open failed");

    // Grow the size
    int res = ftruncate(fd,  totalSz);
    if(res < 0)
        perror("ftruncate failed");

    // Close file
    if(close(fd) < 0)
        perror("\n Close failed");

    // Reopen file
    fd = open(testFile,O_RDWR);
    if(fd < 0)
        perror("open failed");

    // Mmap file
    char * mappedMemory = (char *) mmap(0, totalSz, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

    if(MAP_FAILED == mappedMemory)
        perror("Mmap failed");


    struct timespec start, end;
    long accessTime;
    uint64_t totalTime = 0;

    char *outFile;
    FILE *outFp;

    //appending LFS dir to filename
    if (asprintf(&outFile, "%s/accessTime", dir) == -1) {
        outFile = NULL;
        perror("unable to concatenate file name to LFS dir");
        exit(EXIT_FAILURE);
    }

    outFp = fopen(outFile, "w");
    if(NULL == outFp) {
        perror("Unable to open file for output");
        exit(EXIT_FAILURE);
    }



    // Touch each page.. introduce false dependence to prevent vectorization
    for(uint64_t i = 0; i < numPages; i++){
        clock_gettime(CLOCK_MONOTONIC, &start);
        mappedMemory[accessOrder[i]] += i;
        clock_gettime(CLOCK_MONOTONIC, &end);

        accessTime = TIME_SPENT(start, end);
        totalTime += accessTime;

        fprintf(outFp, "Access time: %lu\n", accessTime);
    }

    printf("Testing on %s finished, results in %s\n", dir, outFile);


    // Unmap
    if(munmap(mappedMemory, totalSz) == -1)
 
      perror("\n Unmap failed");

    // Close file
    if(close(fd) == -1)
        perror("\n Close failed");

    fclose(outFp);

    return totalTime;

} //end test_page_faults


/*
    Extract fields 9-12 from /proc/<pid>/stat to analyze before and after testing
 */
unsigned int*
get_proc_stat(pid_t pid)
{

    char statFile[MAX_PATH_LENGTH],
         statStr[MAX_STRING_LENGTH],
            *strPointer;
    FILE *fp;
    unsigned int *statArray = new unsigned int[4];

    sprintf (statFile, "/proc/%u/stat", (unsigned) pid);
    if ((fp = fopen (statFile, "r")) == NULL) {
        printf("Cannot open file for read\n");
        exit(EXIT_FAILURE);
    }

    if ((strPointer= fgets (statStr, 2048, fp)) == NULL) {
        fclose (fp);
        printf("Error reading stat file");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < 8; i++)
        strsep(&strPointer, " ");

    statArray[FLAGS] = stoul(strsep(&strPointer, " "));
    statArray[MINOR_FAULTS] = stoul(strsep(&strPointer, " "));
    statArray[MINOR_FAULTS_CHILDREN] = stoul(strsep(&strPointer, " "));
    statArray[MAJOR_FAULTS] = stoul(strsep(&strPointer, " "));

    fclose(fp);

    return statArray;
} //end get_proc_stat



int main(int argc, char **argv)
{
    if(argc < 2)
        usage();

    long pageSz = sysconf(_SC_PAGESIZE);
    uint64_t testSize = getTestSize(argv[1]);
    uint64_t numPages = testSize / pageSz;

/*    int delta = 0;*/
    uint64_t totalSz = numPages * pageSz;

    printf("Number of pages: %8lu\n", numPages);

    // this array holds the order in which we access different pages.
    uint64_t *accessOrder = initialize_access_order(numPages, pageSz);

    //get the stats of the process before test
    pid_t pid = getpid();
    unsigned int *pretestStats = get_proc_stat(pid);
    uint64_t LFS_Time = test_page_faults(LFS_DIR, totalSz, numPages, accessOrder);
    unsigned int *posttestStats = get_proc_stat(pid);

    printf("Average page access time in /lfs: %lu\n", LFS_Time / numPages);

    uint64_t DEV_SHM_Time = test_page_faults(DEV_SHM, totalSz, numPages, accessOrder);
    printf("Average page access time in /dev/shm: %lu\n", DEV_SHM_Time / numPages);


/*    printf("Minor faults: %u\n", posttestStats[MINOR_FAULTS] - pretestStats[MINOR_FAULTS]);
    printf("Minor faults of children: %u\n", posttestStats[MINOR_FAULTS_CHILDREN] - pretestStats[MINOR_FAULTS_CHILDREN]);
    printf("Major faults: %u\n", posttestStats[MAJOR_FAULTS] - pretestStats[MAJOR_FAULTS]);
*/

/*    if(posttestStats[FLAGS] != pretestStats[FLAGS]) {
        show_changed_flags(pretestStats[FLAGS], abs((int)(posttestStats[FLAGS] - pretestStats[FLAGS])));
    }

    printf("%s\n",  (DEV_SHM_Time * delta) < LFS_Time ? "FAIL" : "PASS");*/

    delete[] pretestStats;
    delete[] posttestStats;
    delete[] accessOrder;

    return 0;
} //end main

