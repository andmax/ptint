#
# Makefile
#

#SHELL=/bin/csh

CXX = g++
#CXX = icpc

GDIR = $(HOME)/lcgtk/geomTypes
EDIR = $(HOME)/lcgtk/errHandle
KDIR = $(HOME)/lcgtk/glslKernel
VDIR = $(HOME)/andmaxcodes/offVol
PDIR = $(HOME)/andmaxcodes/psiGammaTable

INCLUDES = -I$(GDIR) -I$(EDIR) -I$(KDIR) -I$(VDIR) -I$(PDIR)

LIBDIR = -L$(KDIR)

OBJS = ptVol.o appVol.o ptGLut.o tfGLut.o ptint.o

SRCS = $(OBJS:.o=.cc)

APP = ptint

DEBUGFLAGS = -g
OPTFLAGS = -O3 -ffast-math

ICPCFLAGS = -D_GLIBCXX_GTHREAD_USE_WEAK=0 -pthread

FLAGS = $(DEBUGFLAGS) \
	$(OPTFLAGS) \
	-Wall -Wno-deprecated \
	$(INCLUDES) \
	$(ICPCFLAGS)

LIBS =	-lglut -lGL -lGLU -lXext \
	-lXmu -lX11 -lm -lXi \
	-lglslKernel \
	$(ICPCFLAGS)

#-----------------------------------------------------------------------------

$(APP): $(OBJS)
	@echo "Linking ..."
	$(CXX) $(FLAGS) -o $(APP) $(OBJS) $(LIBDIR) $(LIBS)

depend:
	rm -f .depend
	$(CXX) -M $(FLAGS) $(SRCS) > .depend

.cc.o: $*.h
	@echo "Compiling ..."
	$(CXX) $(FLAGS) -c $*.cc

clean:
	rm -f *.o *~ \#* $(APP) .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
