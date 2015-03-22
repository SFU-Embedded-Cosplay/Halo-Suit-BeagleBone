# This is the Makefile for the beaglebone server for the Mjonir armour
# so that the makefile doesn't get commited use this command
# $ git update-index --assume-unchanged Makefile
# use this command to undo the above if there is an important change in the makefile
# $ git update-index --no-assume-unchanged Makefile

#-----Macros---------------------

OUTFILE = suitcontroller
OUTDIR = /usr/bin
INFILES = main.c json/serializer.c json/parser.c beagleblue/beagleblue.c json/json.c json/json-builder.c halosuit/halosuit.c halosuit/logger.c config/config.c

CROSS_COMPILE = arm-linux-gnueabi-
COMPILER = gcc
CC_C = $(CROSS_COMPILE)$(COMPILER)
CFLAGS = -g -pthread -D_POSIX_C_SOURCE=200809L# -Werror -Wall
INCLUDE = -Iinclude

all: host

host: 
	$(COMPILER) $(CFLAGS) $(INCLUDE) $(INFILES) -o $(OUTDIR)/$(OUTFILE) -lm -lbluetooth
	cp python_scripts/readflow.py $(OUTDIR)/
# doesn't work yet
#target: 
#	$(CC_C) $(CFLAGS) $(INCLUDE) $(INFILES) -o $(OUTDIR)/$(OUTFILE) -lm -lbluetooth

clean: 
	rm $(OUTDIR)/$(OUTFILE)
	rm $(OUTDIR)/readflow.py
