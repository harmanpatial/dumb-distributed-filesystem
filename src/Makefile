#Simple makefile to build the shared library.

CC=g++
CFLAGS= -fPIC -pedantic
LDFLAGS= -shared -Wl

MAKE = /usr/bin/make
RM = /bin/rm
CP = /bin/cp
ECHO = /bin/echo
PWD = /bin/pwd

SRC_DIR1	= logger
SRC_DIR2	= network
SRC_DIR3	= cluster
SRC_DIR4	= global
TARGET		= libddfs.so
OBJS		= ./global/ddfs_status.o ./logger/ddfs_fileLogger.o \
			./network/ddfs_udpConnection.o ./cluster/ddfs_cluster.o \
			./global/ddfs_global.o 
OBJLIBS		= 
LIBS		= -L.

.PHONY: project_code

project_code:
	$(MAKE) -C $(SRC_DIR1)
	$(MAKE) -C $(SRC_DIR2)
	$(MAKE) -C $(SRC_DIR3)
	$(MAKE) -C $(SRC_DIR4)
	$(MAKE) lib

lib : $(TARGET)

$(TARGET) :
	@echo "*** DDFS building targets *****"
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(TARGET) $(OBJS)
	$(CP) $(TARGET) ../

clean :
	$(ECHO) 
	-$(RM) -f ../$(TARGET)
	-$(RM) -f $(TARGET)
	$(MAKE) clean -C $(SRC_DIR1)
	$(MAKE) clean -C $(SRC_DIR2)
	$(MAKE) clean -C $(SRC_DIR3)
	$(MAKE) clean -C $(SRC_DIR4)

force_look :
	true