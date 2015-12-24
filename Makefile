# This is the Makefile for the beaglebone server for the Mjonir armour
# so that the makefile doesn't get commited use this command
# $ git update-index --assume-unchanged Makefile
# use this command to undo the above if there is an important change in the makefile
# $ git update-index --no-assume-unchanged Makefile

#-----Macros---------------------

OUTFILE = suitcontroller
OUTDIR = /usr/bin
INFILES = main.c lib/json/serializer.c lib/json/parser.c lib/beagleblue/beagleblue.c external/json-parser/json.c external/json-builder/json-builder.c lib/halosuit/halosuit.c lib/halosuit/logger.c lib/config/config.c lib/halosuit/automation.c lib/halosuit/stateofcharge.c

CROSS_COMPILE = arm-linux-gnueabi-
COMPILER = gcc
CC_C = $(CROSS_COMPILE)$(COMPILER)x
CFLAGS = -g -pthread -D_POSIX_C_SOURCE=200809L -std=c99 #-Werror -Wall
# this will make the compiler look for header files in the include folder
INCLUDE = -Iinclude -Iexternal/json-parser -Iexternal/json-builder

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

# this command will require root access since it installs bluetooth and python dependencies
# only works for systems that use apt-get (Ubuntu, Debian, etc).
install:
	./install.sh

enable:
	cp ./suitcontrol.service /etc/systemd/system/suitcontrol.service
	systemctl enable /etc/systemd/system/suitcontrol.service

disable:
	systemctl disable /etc/systemd/system/suitcontrol.service
	rm /etc/systemd/system/suitcontrol.service
