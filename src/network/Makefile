
CC=g++
CFLAGS= -g -c -std=c++11 -Winline -Wall -Werror -pedantic-errors -pthread
LDFLAGS= -fpic # -v

SOURCES = 
INCLUDE = ddfs_status.h ../logges/ddfs_logger.h ddfs_network.h
OBJLIBS	= ../ddfs_fileLogger.o
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
