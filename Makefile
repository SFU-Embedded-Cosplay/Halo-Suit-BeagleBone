# This is the Makefile for the beaglebone server for the Mjonir armour
# so that the makefile doesn't get commited use this command
# $ git update-index --assume-unchanged Makefile
# use this command to undo the above if there is an important change in the makefile
# $ git update-index --no-assume-unchanged Makefile

#-----Macros---------------------

OUTFILE = suitcontroller
OUTDIR = /usr/bin
TESTDIR = .
INFILES = main.c json/serializer.c json/parser.c beagleblue/beagleblue.c json-parser/json.c json-builder/json-builder.c halosuit/halosuit.c halosuit/logger.c config/config.c halosuit/automation.c halosuit/stateofcharge.c mockHardware/mockHalosuit.c mockHardware/mockBluetooth.c

CROSS_COMPILE = arm-linux-gnueabi-
COMPILER = gcc
CC_C = $(CROSS_COMPILE)$(COMPILER)
CFLAGS = -g -pthread -D_POSIX_C_SOURCE=200809L -std=c99 #-Werror -Wall
# this will make the compiler look for header files in the include folder
INCLUDE = -Iinclude -Ijson-parser -Ijson-builder

all: host

host: 
	$(COMPILER) $(CFLAGS) $(INCLUDE) $(INFILES) -o $(OUTDIR)/$(OUTFILE) -lm -lbluetooth
	cp python_scripts/readflow.py $(OUTDIR)/

# doesn't work yet
# this is the monument to my Failure to get cross compiling working
#target: 
#	$(CC_C) $(CFLAGS) $(INCLUDE) $(INFILES) -o $(OUTDIR)/$(OUTFILE) -lm -lbluetooth

clean: 
	rm $(OUTDIR)/$(OUTFILE)
	rm $(OUTDIR)/readflow.py

install:
	./install.sh

enable:
	cp ./suitcontrol.service /etc/systemd/system/suitcontrol.service
	systemctl enable /etc/systemd/system/suitcontrol.service

disable:
	systemctl disable /etc/systemd/system/suitcontrol.service

localTest:	
	$(COMPILER) $(CFLAGS) $(INCLUDE) $(INFILES) -o $(TESTDIR)/$(OUTFILE) -lm -lbluetooth -D MOCK_HARDWARE -D MOCK_BLUETOOTH
