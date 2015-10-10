
CC=g++
CFLAGS=-g -c -std=c++11 -Winline -Wall -Werror -pedantic-errors -pthread
LDFLAGS= -fpic # -v
IMPR = -fno-default-inline -Wctor-dtor-privacy

SOURCES = ddfs_status.cpp ddfs_global.cpp
INCLUDE = -I. -I../logger/
INCLUDE_FILES = -Iddfs_global.hpp  -Iddfs_status.hpp -I../logger/ddfs_logger.hpp -I../cluster/ddfs_cluster.hpp
OBJLIBS	= ../ddfs_global.o
LIBS	= 
RM	= /bin/rm

# Objs are all the sources, with .cpp replaced by .o
OBJS := $(SOURCES:.cpp=.o) $(LIBS)

# To obtain object files
#%.o: %.cpp
#	$(CC) $(LDFLAGS) $(CFLAGS) $(IMPR) $(INCLUDE) $(LIBS) $< -o $@

%.o: %.cpp
	$(CC) $(LDFLAGS) $(CFLAGS) -c $<

all : $(ECHO) $(OBJS) $(LIBS)

clean:
	$(RM) -f $(OBJS)
