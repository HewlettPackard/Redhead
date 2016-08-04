/*
 * Code to probe hardware for SIMD support for Intel and Arm processors,
 * and produces a Makefile for Jerasure with the correct extension flags
 * based on these support.
 *
 * Currently, the Makefile is hardcoded specifically for Jerasure
 * and will need to be modified accordingly if there is a need for extensions.
 *
 * (c) Copyright 2016, Hewlett-Packard Development Company, LP
 *
 * */

#include <stdio.h>

#if __linux__
#include <elf.h>
#include <link.h>
#include <string.h>

#if __arm__
#include <asm/hwcap.h>
#endif //__arm__

#endif //__linux__

#define MAX_BUF_LEN 1024

//Flags for probing neon
int neon_support = 0;


//Flags for probing Intel
int sse2_support = 0;
int sse3_support = 0;
int ssse3_support = 0;
int sse4_1_support = 0;
int sse4_2_support = 0;
int pclmul_support = 0;


#ifdef __x86_64__

//probe the cpu by doing a "cpuid" instruction. Desired values are stored in ECX and EDX registers
static void do_cpuid(unsigned int *EAX,
                     unsigned int *EBX,
                     unsigned int *ECX,
                     unsigned int *EDX)
{
        int id = *EAX;

        asm("movl %4, %%eax;"
            "cpuid;"
            "movl %%eax, %0;"
            "movl %%ebx, %1;"
            "movl %%ecx, %2;"
            "movl %%edx, %3;"
                : "=r" (*EAX), "=r" (*EBX), "=r" (*ECX), "=r" (*EDX)
                : "r" (id)
                : "eax", "ebx", "ecx", "edx");
}


/* http://en.wikipedia.org/wiki/CPUID#EAX.3D1:_Processor_Info_and_Feature_Bits */

#define CPUID_PCLMUL    (1 << 1)
#define CPUID_SSE42     (1 << 20)
#define CPUID_SSE41     (1 << 19)
#define CPUID_SSSE3     (1 << 9)
#define CPUID_SSE3      (1)
#define CPUID_SSE2      (1 << 26)

//probe the intel chip and checks for the extensions supported
int probe_intel()
{
    unsigned int EAX = 1, EBX, ECX, EDX;

    do_cpuid(&EAX, &EBX, &ECX, &EDX);

    if ((ECX & CPUID_PCLMUL) != 0)
        pclmul_support = 1;

    if ((ECX & CPUID_SSE42) != 0)
        sse4_2_support = 1;

    if ((ECX & CPUID_SSE41) != 0)
        sse4_1_support = 1;

    if ((ECX & CPUID_SSSE3) != 0)
        ssse3_support = 1;

    if ((ECX & CPUID_SSE3) != 0)
        sse3_support = 1;

    if ((EDX & CPUID_SSE2) != 0)
        sse2_support = 1;

    return 0;
}//end probe_intel

#else // __x86_64__

int probe_intel()
{
    return 0;
}
#endif // __x86_64__

#ifdef __linux__

//getting hardware capabilities through auxval
static unsigned long get_hw_capabilities(void)
{
    unsigned long result = 0;

    FILE *fp = fopen("/proc/self/auxv", "r");
    if (fp) {
        ElfW(auxv_t) entry;
        while (fread(&entry, sizeof(entry), 1, fp)) {
            if (entry.a_type == AT_HWCAP) {
                result = entry.a_un.a_val;
                break;
            }
        }
        fclose(fp);
    }
    return result;
}//end get_hw_capabilities

#endif //__linux__

int probe_arm()
{
#if __arm__ && __linux__
    neon_support = (get_hw_capabilities() & HWCAP_NEON) == HWCAP_NEON;
#endif
    return 0;
}



int main(int argc, char* argv[])
{
    probe_intel();
    probe_arm();

    int debug, jerasure, memory, fail, simd;
	sscanf(argv[1], "%d", &debug);
	sscanf(argv[2], "%d", &jerasure);
	sscanf(argv[3], "%d", &memory);
	sscanf(argv[4], "%d", &fail);
	sscanf(argv[5], "%d", &simd);

    char buffer[MAX_BUF_LEN]; //buffer to store the appended C flags for SIMD support

    FILE *fp;

    fp = fopen("Makefile", "w");

    if(fp) {
        //headers
        fprintf(fp, "%s += \\\n %s \\\n %s \\\n %s \\\n %s \\\n %s \\\n "
                    "%s \\\n %s \\\n %s \\\n %s \\\n %s \\\n\n",
        "noinst_HEADERS",
        "jerasure/include/cauchy.h",
        "jerasure/include/galois.h",
        "jerasure/include/jerasure.h",
        "jerasure/include/liberation.h",
        "jerasure/include/reed_sol.h",
        "gf-complete/include/gf_int.h",
        "gf-complete/include/gf_complete.h",
        "gf-complete/include/gf_rand.h",
        "gf-complete/include/gf_method.h",
        "gf-complete/include/gf_general.h");

        //jerasure sources
        fprintf(fp, "%s = \\\n %s \\\n %s \\\n %s \\\n %s \\\n %s \\\n %s \\\n "
                    "%s \\\n %s \\\n %s \\\n %s \\\n %s \\\n "
                    "%s \\\n %s \\\n %s \\\n %s \\\n %s \\\n\n",
        "jerasure_sources",
        "jerasure/src/cauchy.c",
        "jerasure/src/galois.c",
        "jerasure/src/jerasure.c",
        "jerasure/src/liberation.c",
        "jerasure/src/reed_sol.c",
        "gf-complete/src/gf_wgen.c",
        "gf-complete/src/gf_method.c",
        "gf-complete/src/gf_w16.c",
        "gf-complete/src/gf.c",
        "gf-complete/src/gf_w32.c",
        "gf-complete/src/gf_w64.c",
        "gf-complete/src/gf_w128.c",
        "gf-complete/src/gf_general.c",
        "gf-complete/src/gf_w4.c",
        "gf-complete/src/gf_rand.c",
        "gf-complete/src/gf_w8.c");


        fprintf(fp, "%s = \\\n %s \\\n\n%s = \\\n "
                    "%s\n\n%s\n\n%s\n\n",
        "r64tm_sources",
        "raid6/src/r6.c",
        "app_sources",
        "r6_emu.c",
        "libec_jerasure_generic_la_SOURCES = ${jerasure_sources}",
        "CC = gcc");


        //C Flags
        snprintf(buffer, MAX_BUF_LEN, "%s", "CFLAGS = -O2 -Wall -lm -lpthread ");

#ifdef __x86_64__
   if(simd == 1)
   {
        if(sse2_support)
            strncat(buffer, " -msse2 -DINTEL_SSE2", strlen(" -msse2 -DINTEL_SSE2"));

        if(sse3_support)
            strncat(buffer, " -msse3 -DINTEL_SSE3", strlen(" -msse3 -DINTEL_SSE3"));

        if(ssse3_support)
            strncat(buffer, " -mssse3 -DINTEL_SSSE3", strlen(" -mssse3 -DINTEL_SSSE3"));

        if(sse4_1_support)
            strncat(buffer, " -msse4.1 -DINTEL_SSE4", strlen(" -msse4.1 -DINTEL_SSE4"));

        if(sse4_2_support)
            strncat(buffer, " -msse4.2 -DINTEL_SSE4", strlen(" -msse4.2 -DINTEL_SSE4"));

        if(pclmul_support)
            strncat(buffer, " -mpclmul -DINTEL_SSE4_PCLMUL", strlen(" -mpclmul -DINTEL_SSE4_PCLMUL"));
   }

#endif
        strcat(buffer, "\n\n");
        fprintf(fp, "%s", buffer);


	if(debug == 1)
	{
		fprintf(fp, "DFLAG = -DDEBUG\n\n");
	}
	else
	{
		fprintf(fp, "DFLAG = \n\n");
	}

	if(jerasure == 1)
        {
                fprintf(fp, "JFLAG = -DJERASURE\n\n");
        }
        else
        {
                fprintf(fp, "JFLAG = \n\n");
        }

	switch(memory)
	{
		case 1:
			fprintf(fp, "MFLAG = -DLFS\n\n");
			break;
		case 2:
			fprintf(fp, "MFLAG = -DSHM\n\n");
			break;
		case 4:
			fprintf(fp, "MFLAG = -DNFS\n\n");
			break;
		case 3:
			fprintf(fp, "MFLAG = -DDRAM\n\n");
			break;
		default:
			fprintf(fp, "MFLAG = DRAM\n\n");
			break;
	}

	if(fail == 1)
        {
               fprintf(fp, "FFLAG = -DFAILURE\n\n");
        }
        else
        {
               fprintf(fp, "FFLAG = \n\n");
        }

        fprintf(fp, "%s = \\\n %s \\\n "
                    "%s \\\n %s \n\n",
        "SOURCES",
        "$(libec_jerasure_generic_la_SOURCES)",
        "$(r64tm_sources)",
        "$(app_sources)");

	switch(memory)
        {
                case 1:
        		fprintf(fp, "%s = \\\n %s \\\n "
                		    "%s \\\n %s \n\n%s\n\n",
        		"DIREC",
        		"-Ijerasure/include",
        		"-Igf-complete/include",
        		"-Iraid6/include",
       	 		"EXECUTABLE = r6_emu_lfs");
                        break;
                case 2:
        		fprintf(fp, "%s = \\\n %s \\\n "
                		    "%s \\\n %s \n\n%s\n\n",
        		"DIREC",
        		"-Ijerasure/include",
        		"-Igf-complete/include",
        		"-Iraid6/include",
        		"EXECUTABLE = r6_emu_shm");
                        break;
                case 4:
        		fprintf(fp, "%s = \\\n %s \\\n "
                		    "%s \\\n %s \n\n%s\n\n",
        		"DIREC",
        		"-Ijerasure/include",
        		"-Igf-complete/include",
        		"-Iraid6/include",
        		"EXECUTABLE = r6_emu_nfs");
                        break;
                case 3:
        		fprintf(fp, "%s = \\\n %s \\\n "
                		    "%s \\\n %s \n\n%s\n\n",
        		"DIREC",
        		"-Ijerasure/include",
        		"-Igf-complete/include",
        		"-Iraid6/include",
        		"EXECUTABLE = r6_emu_dram");
                        break;
                default:
        		fprintf(fp, "%s = \\\n %s \\\n "
                		    "%s \\\n %s \n\n%s\n\n",
        		"DIREC",
        		"-Ijerasure/include",
        		"-Igf-complete/include",
        		"-Iraid6/include",
        		"EXECUTABLE = r6_emu");
                        break;
        }



        //targets
        fprintf(fp, "%s\n\n%s\n\t%s"
                    "\n\n%s\n\t%s",
        "all: $(SOURCES) $(EXECUTABLE)",
        "$(EXECUTABLE):",
        "$(CC) $(DIREC) $(CFLAGS) $(DFLAG) $(JFLAG) $(MFLAG) $(FFLAG) $(LDFLAGS) $(SOURCES) -o $@",
        "clean:",
        "rm $(EXECUTABLE)");

        fclose(fp);
    }else {
        perror("Makefile could not be generated");
    }

    return 0;
}//end main
