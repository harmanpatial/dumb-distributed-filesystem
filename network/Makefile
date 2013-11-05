
CC=g++
CFLAGS=-c -Wall
LDFLAGS=

DIRS	= network
EXE	= ddfs
OBJS	= ddfs.o
OBJLIBS	= network.a logger.a
LIBS	= -L. -lnetwork -llogger

all : $(EXE)

ddfs : $(OBJS) $(OBJLIBS)
	$(ECHO) $(LD) -o $(EXE) $(OBJS) $(LIBS)
	$(LD) -o $(EXE) $(OBJS) $(LIBS)

logger.a : force_look
	$(ECHO) looking into subdir : $(MAKE) $(MFLAGS)
	cd logger; $(MAKE) $(MFLAGS)

network.a : force_look
	$(ECHO) looking into subdira : $(MAKE) $(MFLAGS)
	cd network; $(MAKE) $(MFLAGS)

clean :
	$(ECHO) cleaning up in .
	-$(RM) -f $(EXE) $(OBJS) $(OBJLIBS)
	-for d in $(DIRS); do (cd $$d; $(MAKE) clean ); done

force_look :
	true
