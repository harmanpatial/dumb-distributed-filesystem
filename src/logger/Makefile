
CC=g++
CFLAGS=-c -Wall
LDFLAGS=

SOURCES = ddfs_fileLogger.cpp
OBJLIBS	= 
LIBS	= -L. -lc

# Objs are all the sources, with .cpp replaced by .o
OBJS := $(SOURCES:.cpp=.o)

all : $(OBJS)

clean :
	-$(RM) -f $(OBJS) $(OBJLIBS)
