
CC=g++
CFLAGS= -g -c -std=c++11 -Winline -Wall -Werror -pedantic-errors -pthread
LDFLAGS= -fpic # -v

SOURCES = ddfs_simplefilesystem.cpp
INCLUDE = ddfs_simplefilesystem.hpp  ddfs_cluster.h ddfs_clusterMember.h \
		  ddfs_clusterMemberPaxos.h ddfs_clusterPaxos.h \
		  ../logger/ddfs_logger.h ../global/ddfs_status.h
OBJLIBS	= ../ddfs_fileSystem.o
LIBS	= 
RM	= /bin/rm

# Objs are all the sources, with .cpp replaced by .o
OBJS := $(SOURCES:.cpp=.o) $(LIBS)

# To obtain object files
%.o: %.cpp
	$(CC) $(LDFLAGS) $(CFLAGS) $(LIBS) $< -o $@

all : $(ECHO) $(OBJS) $(LIBS)

clean:
	$(RM) -f $(OBJS)
