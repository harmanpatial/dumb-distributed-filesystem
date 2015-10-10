
CC=g++
CFLAGS= -std=c++11
#-fpic -- This is only required to build shared library 

INCLUDE = -I/home/harman/ddfs/src/global/
LIBS	= -L/usr/local/lib/ -lddfs
RM	= /bin/rm

# Objs are all the sources, with .cpp replaced by .o

all : $(ECHO)
	$(CC) $(CFLAGS) $(INCLUDE) $(LIBS) -o test1 test1.cpp
	

clean:
	$(RM) -f test1 
