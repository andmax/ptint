#
# Makefile
#

#SHELL=/bin/csh

CXX = g++
#CXX = icpc

# Win
#HOME = ../..
#APP = ptint.exe
#RM = del

# Linux
APP = iptint
RM = rm -f

LDIR = $(HOME)/lcgtk

KDIR = $(LDIR)/glslKernel

INCLUDES = -I$(LDIR) -I$(KDIR)

LIBDIR = -L$(LDIR)/lib

OBJS = ftrackball.o volume.o transferFunction.o illuminationControl.o render_volume_gpu.o

SRCS = $(OBJS:.o=.cc)

DEBUGFLAGS = #-g
OPTFLAGS = -O3 -ffast-math

ICPCFLAGS = -D_GLIBCXX_GTHREAD_USE_WEAK=0 -pthread

FLAGS = $(DEBUGFLAGS) \
	$(OPTFLAGS) \
	-Wall -Wno-deprecated \
	$(INCLUDES) \
#	$(ICPCFLAGS)

# Win

#LIBS =	-lGLee -lglslKernel -lglut32 -lglu32 -lopengl32 -mwindows

# Linux

LIBS =	-lglut -lGL -lGLU -lGLee -lXext \
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
	$(RM) *.o *~ $(APP) .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
