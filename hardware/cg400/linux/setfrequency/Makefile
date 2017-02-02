#	This is Linux makefile for SIDDK sample. 
#	All source code for Windows and Linux versions are the same except for 
#	the API functions. Windows version uses sicommon_plx_win.cpp while the 
#	Linux version uses sicommon_plx_lnx.cpp.
#
CC=g++
INCLUDEDIR = ../common
CFLAGS = -Wall -I$(INCLUDEDIR) -DLINUX -DSIDEF_PLX_SIDDK

TARGET = setfrequency

all: release

release:
#	1.compile all files
	$(CC) -c $(CFLAGS) ../common/sicommon_plx_lnx.cpp
	$(CC) -c $(CFLAGS) siabstract.cpp
	$(CC) -c $(CFLAGS) cg400_lib.cpp
	$(CC) -c $(CFLAGS) user_test.cpp
	
#	2.link object files to generate executable
	$(CC) -o $(TARGET) *.o

debug:
#	1.compile all files
	$(CC) -c $(CFLAGS) -g ../common/sicommon_plx_lnx.cpp
	$(CC) -c $(CFLAGS) -g siabstract.cpp
	$(CC) -c $(CFLAGS) -g cg400_lib.cpp
	$(CC) -c $(CFLAGS) -g user_test.cpp
	
#	2.link object files to generate executable
	$(CC) -o $(TARGET) -g *.o
			
clean:
	rm -f *.o *~ core .depend $(TARGET)
	
	
