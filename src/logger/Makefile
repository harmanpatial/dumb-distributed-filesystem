
CC=g++
CFLAGS=-c -std=c++11 -Wall
LDFLAGS=

SOURCES = ddfs_fileLogger.cpp
OBJLIBS	= 
LIBS	=

# Objs are all the sources, with .cpp replaced by .o
OBJS := $(SOURCES:.cpp=.o) $(LIBS)

# To obtain object files
%.o: %.cpp
	$(CC) $(CFLAGS) $(LIBS) $< -o $@

all : $(ECHO) $(OBJS) $(LIBS)

clean:
	$(RM) -f $(OBJS)
