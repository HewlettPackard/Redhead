noinst_HEADERS += \
 jerasure/include/cauchy.h \
 jerasure/include/galois.h \
 jerasure/include/jerasure.h \
 jerasure/include/liberation.h \
 jerasure/include/reed_sol.h \
 gf-complete/include/gf_int.h \
 gf-complete/include/gf_complete.h \
 gf-complete/include/gf_rand.h \
 gf-complete/include/gf_method.h \
 gf-complete/include/gf_general.h \

jerasure_sources = \
 jerasure/src/cauchy.c \
 jerasure/src/galois.c \
 jerasure/src/jerasure.c \
 jerasure/src/liberation.c \
 jerasure/src/reed_sol.c \
 gf-complete/src/gf_wgen.c \
 gf-complete/src/gf_method.c \
 gf-complete/src/gf_w16.c \
 gf-complete/src/gf.c \
 gf-complete/src/gf_w32.c \
 gf-complete/src/gf_w64.c \
 gf-complete/src/gf_w128.c \
 gf-complete/src/gf_general.c \
 gf-complete/src/gf_w4.c \
 gf-complete/src/gf_rand.c \
 gf-complete/src/gf_w8.c \

r64tm_sources = \
 raid6/src/r6.c \

app_sources = \
 r6_emu.c

libec_jerasure_generic_la_SOURCES = ${jerasure_sources}

CC = gcc

CFLAGS = -O2 -Wall -lm -lpthread  -msse2 -DINTEL_SSE2 -msse3 -DINTEL_SSE3 -mssse3 -DINTEL_SSSE3 -msse4.1 -DINTEL_SSE4 -msse4.2 -DINTEL_SSE4 -mpclmul -DINTEL_SSE4_PCLMUL

neon_sources = 

DFLAG = 

JFLAG = 

MFLAG = -DSHM

FFLAG = -DFAILURE

SOURCES = \
 $(libec_jerasure_generic_la_SOURCES) \
 $(r64tm_sources) \
 $(app_sources) \
 $(neon_sources) 

DIREC = \
 -Ijerasure/include \
 -Igf-complete/include \
 -Iraid6/include 

EXECUTABLE = r6_emu_shm

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE):
	$(CC) $(DIREC) $(CFLAGS) $(DFLAG) $(JFLAG) $(MFLAG) $(FFLAG) $(LDFLAGS) $(SOURCES) -o $@

clean:
	rm $(EXECUTABLE)