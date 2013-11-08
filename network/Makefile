
CC=g++
CFLAGS=-c -Wall
LDFLAGS=

SOURCES = ddfs_udpConnection.cpp
INCLUDE = ddfs_status.h ../logges/ddfs_logger.h ddfs_network.h
OBJLIBS	= 
LIBS	= -L.
RM	= /bin/rm

# Objs are all the sources, with .cpp replaced by .o
OBJS := $(SOURCES:.cpp=.o)

all : $(ECHO) $(OBJS) 

clean:
	$(RM) -f $(OBJS)
