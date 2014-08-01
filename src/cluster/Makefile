
CC=g++
CFLAGS= -c -std=c++11 -Winline -Wall -Werror -pedantic-errors
LDFLAGS=

SOURCES = ddfs_clusterMessagesPaxos.cpp ddfs_clusterPaxosInstance.cpp ddfs_clusterMemberPaxos.cpp ddfs_clusterPaxos.cpp
INCLUDE = ddfs_clusterMessagesPaxos.h ddfs_clusterPaxosInstance.h ddfs_cluster.h ddfs_clusterMember.h \
		  ddfs_clusterMemberPaxos.h ddfs_clusterPaxos.h \
		  ../logger/ddfs_logger.h ../global/ddfs_status.h
OBJLIBS	= ../ddfs_cluster.o
LIBS	= 
RM	= /bin/rm

# Objs are all the sources, with .cpp replaced by .o
OBJS := $(SOURCES:.cpp=.o) $(LIBS)

# To obtain object files
%.o: %.cpp
	$(CC) $(CFLAGS) $(LIBS) $< -o $@

all : $(ECHO) $(OBJS) $(LIBS)

clean:
	$(RM) -f $(OBJS)
