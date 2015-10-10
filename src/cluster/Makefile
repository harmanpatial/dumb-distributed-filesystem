
CC=g++
CFLAGS= -g -c -std=c++11 -Winline -Wall -Werror -pedantic-errors -pthread
LDFLAGS= -fpic #-v
IMPR = -fno-default-inline -Wctor-dtor-privacy 

SOURCES = ddfs_clusterMessagesPaxos.cpp ddfs_clusterPaxos.cpp ddfs_clusterMemberPaxos.cpp ddfs_clusterPaxosInstance.cpp
INCLUDE = ddfs_clusterMessagesPaxos.hpp ddfs_clusterPaxosInstance.hpp ddfs_cluster.hpp ddfs_clusterMember.hpp \
		  ddfs_clusterMemberPaxos.hpp ddfs_clusterPaxos.hpp \
		  ../logger/ddfs_logger.hpp ../global/ddfs_status.hpp
OBJLIBS	= ../ddfs_cluster.o
LIBS	= 
RM	= /bin/rm

# Objs are all the sources, with .cpp replaced by .o
OBJS := $(SOURCES:.cpp=.o) $(LIBS)

# To obtain object files
%.o: %.cpp
	$(CC) $(LDFLAGS) $(CFLAGS) $(IMPR) $(LIBS) $< -o $@

all : $(ECHO) $(OBJS) $(LIBS)

clean:
	$(RM) -f $(OBJS)
