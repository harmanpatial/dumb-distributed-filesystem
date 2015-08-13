#Makefile to build DDFS shared library.

CC=g++
CFLAGS= -std=c++11 -dynamiclib -pedantic
LDFLAGS= -shared -Wl

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
TARGET		= libddfs.so.1
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
	$(CC) $(LDFLAGS) $(CFLAGS) $(INCLUDE) -o $(TARGET) $(OBJS)
	$(CP) $(TARGET) ../
	@echo "*** DDFS Build Process Complete *****"

clean :
	$(ECHO) 
	-$(RM) -f ../$(TARGET)
	-$(RM) -f $(TARGET)
	$(MAKE) clean -C $(SRC_DIR1)
	$(MAKE) clean -C $(SRC_DIR2)
	$(MAKE) clean -C $(SRC_DIR3)
	$(MAKE) clean -C $(SRC_DIR4)
	$(MAKE) clean -C $(SRC_DIR5)

force_look :
	true
