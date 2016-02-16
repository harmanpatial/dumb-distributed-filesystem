
CC=g++
CFLAGS= -std=c++11
#-fpic -- This is only required to build shared library 

INCLUDE = -I/home/harman/ddfs/src/global
LIBS	= -L/usr/local/lib
RM	= /bin/rm

# Objs are all the sources, with .cpp replaced by .o

all : $(ECHO)
	$(CC) $(CFLAGS) $(INCLUDE) $(LIBS) test1.cpp -o test1 -lddfs
	

clean:
	$(RM) -f test1 
