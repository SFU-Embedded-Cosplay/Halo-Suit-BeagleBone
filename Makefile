# This is the Makefile for the beaglebone server for the Mjonir armour
# the makefile is ignored by git so that we don't thrash each others git files

#-----Macros---------------------

OUTFILE = server
OUTDIR = .
INFILES = server.c listener.c processor.c queue.c watchdog.c

CROSS_COMPILE = arm-linux-gnueabi-
COMPILER = gcc
CC_C = $(CROSS_COMPILE)$(COMPILER)
CFLAGS = -Wall -g -std=c99 -pthread -Werror -D_POSIX_C_SOURCE=200809L 

all: host target

host: 
	$(COMPILER) $(CFLAGS) $(INFILES) -o $(OUTDIR)/$(OUTFILE)_Host -lm

target: 
	$(CC_C) $(CFLAGS) $(INFILES) -o $(OUTDIR)/$(OUTFILE)_Target -lm

clean: 
	rm $(OUTDIR)/$(OUTFILE)_Host
	rm $(OUTDIR)/$(OUTFILE)_Target


