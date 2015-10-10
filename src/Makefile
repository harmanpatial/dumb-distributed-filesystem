#Makefile to build DDFS shared library.

CC=g++
CFLAGS= -std=c++11 -pedantic -O2 -pthread
LDFLAGS= -shared -Wl,-soname,libddfs.so.1

MAKE = /usr/bin/make
RM = /bin/rm
CP = /bin/cp
ECHO = /bin/echo
PWD = /bin/pwd
INCLUDE = -I./logger/ -I./global/ -I./network/ -I./cluster/

SRC_DIR1	= logger
SRC_DIR2	= global
SRC_DIR3	= network
SRC_DIR4	= cluster
SRC_DIR5	= filesystem
TARGET		= libddfs.so
TARGET1		= libddfs.so.1
LIBRARY_PATH= /usr/local/lib/
OBJS		= ./global/ddfs_status.o ./logger/ddfs_fileLogger.o \
			 ./cluster/ddfs_clusterMessagesPaxos.o \
			./cluster/ddfs_clusterMemberPaxos.o \
			./cluster/ddfs_clusterPaxos.o \
			./cluster/ddfs_clusterPaxosInstance.o \
			./global/ddfs_global.o \
			./filesystem/ddfs_simplefilesystem.o
OBJLIBS		= 
LIBS		= -L.

.PHONY: project_code

project_code:
	@echo "*** DDFS Compiling targets *****"
	$(MAKE) -C $(SRC_DIR1)
	$(MAKE) -C $(SRC_DIR2)
	$(MAKE) -C $(SRC_DIR3)
	$(MAKE) -C $(SRC_DIR4)
	$(MAKE) -C $(SRC_DIR5)
	@echo "*** DDFS Compilation Completed *****"
	$(MAKE) lib

lib : $(TARGET)

$(TARGET) :
	@echo "*** DDFS Building Dynamic Library *****"
	$(CC) $(CFLAGS) $(LDFLAGS) $(INCLUDE) -o $(TARGET) $(OBJS)
	$(CP) $(TARGET) ../$(TARGET)
	$(CP) $(TARGET) ../$(TARGET1)
	$(CP) ../$(TARGET) $(LIBRARY_PATH)
	$(CP) ../$(TARGET1) $(LIBRARY_PATH)
	@echo "*** DDFS Build Process Complete *****"

clean :
	$(ECHO) 
	-$(RM) -f ../$(TARGET)
	-$(RM) -f ../$(TARGET1)
	-$(RM) -f $(TARGET)
	$(MAKE) clean -C $(SRC_DIR1)
	$(MAKE) clean -C $(SRC_DIR2)
	$(MAKE) clean -C $(SRC_DIR3)
	$(MAKE) clean -C $(SRC_DIR4)
	$(MAKE) clean -C $(SRC_DIR5)

force_look :
	true
