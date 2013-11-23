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
SRC_DIR2	= global
SRC_DIR3	= network
TARGET		= libddfs.so
OBJS		= ./global/ddfs_status.o ./logger/ddfs_fileLogger.o \
		  ./global/ddfs_global.o ./network/ddfs_udpConnection.o
OBJLIBS		= 
LIBS		= -L.

.PHONY: project_code

project_code:
	$(MAKE) -C $(SRC_DIR1)
	$(MAKE) -C $(SRC_DIR2)
	$(MAKE) -C $(SRC_DIR3)
	$(MAKE) library

library : $(TARGET)

$(TARGET) : 
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(TARGET) $(OBJS)
	$(CP) $(TARGET) ../

clean :
	$(ECHO) 
	-$(RM) -f ../$(TARGET)
	-$(RM) -f $(TARGET)
	$(MAKE) clean -C $(SRC_DIR1)
	$(MAKE) clean -C $(SRC_DIR2)
	$(MAKE) clean -C $(SRC_DIR3)

force_look :
	true
