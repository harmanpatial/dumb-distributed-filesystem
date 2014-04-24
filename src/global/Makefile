
CC=g++
CFLAGS=-c -Winline -Wall -Werror -pedantic-errors
LDFLAGS=

SOURCES = ddfs_status.cpp ddfs_global.cpp
INCLUDE = ddfs_status.h ddfs_global.h ../logger/ddfs_logger.h ../cluster/ddfs_cluster.h
OBJLIBS	= ../ddfs_global.o
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
