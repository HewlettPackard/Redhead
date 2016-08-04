# jerasure plugin
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

CFLAGS = -O2 -Wall -lm -lpthread

LDFLAGS =

SOURCES =  \
        $(libec_jerasure_generic_la_SOURCES) \
	$(r64tm_sources) \
        $(app_sources) 

#OBJECTSc = $(SOURCES:.c=.o)

#OBJECTScc = $(SOURCES:.cc=.o)

#OBJECTScpp = $(SOURCES:.cpp=.o)

#OBJECTS = $(OBJECTSc) $(OBJECTScc) $(OBJECTcpp)

DIREC = \
	-Ijerasure/include \
	-Igf-complete/include \
	-Iraid6/include  

EXECUTABLE = r6_emu_dram


all: $(SOURCES) $(EXECUTABLE) 

$(EXECUTABLE):  
	$(CC) $(DIREC) $(CFLAGS) $(LDFLAGS) $(SOURCES) -o $@ 

clean:
	rm $(EXECUTABLE) 
#	rm -r /lfs/*
	rm -r /dev/shm/r6/*
#	rm -r lfs/*

