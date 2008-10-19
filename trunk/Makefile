#
# Makefile
#

#SHELL=/bin/csh

CXX = g++
#CXX = icpc

# Win
HOME = ../..
APP = ptint.exe
RM = del

# Linux
#APP = ptint
#RM = rm -f \#*

GDIR = $(HOME)/lcgtk/geomTypes
EDIR = $(HOME)/lcgtk/errHandle
KDIR = $(HOME)/lcgtk/glslKernel
PDIR = ../psiGammaTable

INCLUDES = -I$(GDIR) -I$(EDIR) -I$(KDIR) -I$(PDIR)

LIBDIR = -L$(KDIR)

OBJS = ptVol.o appVol.o ptGLut.o tfGLut.o ptint.o

SRCS = $(OBJS:.o=.cc)

DEBUGFLAGS = -g
OPTFLAGS = -O3 -ffast-math

ICPCFLAGS = -D_GLIBCXX_GTHREAD_USE_WEAK=0 -pthread

FLAGS = $(DEBUGFLAGS) \
	$(OPTFLAGS) \
	-Wall -Wno-deprecated \
	$(INCLUDES) \
#	$(ICPCFLAGS)

# Win

LIBS =	-lGLee -lglslKernel -lglut32 -lglu32 -lopengl32 -mwindows

# Linux

#LIBS =	-lglut -lGL -lGLU -lXext \
#	-lXmu -lX11 -lm -lXi \
#	-lglslKernel \
#	$(ICPCFLAGS)

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
	$(RM) *.o *~ $(APP) .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
