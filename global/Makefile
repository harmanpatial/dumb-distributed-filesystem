
CC=g++
CFLAGS=-c -Wall
LDFLAGS=

SOURCES = ddfs_status.cpp ddfs_global.cpp
INCLUDE = ddfs_status.h ddfs_global.h ../logger/ddfs_logger.h
OBJLIBS	= 
LIBS	= -L.
RM	= /bin/rm

# Objs are all the sources, with .cpp replaced by .o
OBJS := $(SOURCES:.cpp=.o)

all : $(ECHO) $(OBJS) 

clean:
	$(RM) -f $(OBJS)
