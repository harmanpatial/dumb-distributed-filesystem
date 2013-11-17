#Simple makefile to build the shared library.

CC=g++
CFLAGS= -fPIC -pedantic
LDFLAGS= -shared -Wl

RM = /bin/rm
CP = /bin/cp

DIRS	= network
TARGET	= ddfs.so
OBJS	= ./global/ddfs_status.o ./logger/ddfs_fileLogger.o ./global/ddfs_global.o ./network/ddfs_udpConnection.o
OBJLIBS	= 
LIBS	= -L.

all : $(TARGET)

$(TARGET) : 
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(TARGET) $(OBJS)
	$(CP) $(TARGET) ../

clean :
	$(ECHO) 
	-$(RM) -f ../$(TARGET)
	-$(RM) -f $(TARGET)

force_look :
	true
