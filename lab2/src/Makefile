# Makefile, ECE252  

CC = gcc 
CFLAGS = -Wall -std=c99 -g # "curl-config --cflags" output is empty  
LD = gcc
LDFLAGS = -std=c99 -g 
LDLIBS = -lcurl -lz -pthread # "curl-config --libs" output 

SRCS   = paster.c crc.c zutil.c util.c png.c
OBJS  = paster.o crc.o zutil.o util.o png.o
TARGETS= paster

all: ${TARGETS}

paster: $(OBJS) 
	$(LD) -o $@ $^ $(LDLIBS) $(LDFLAGS) 

%.o: %.c 
	$(CC) $(CFLAGS) -o $@ -c $< 

%.d: %.c
	gcc -MM -MF $@ $<

.PHONY: clean
clean:
	rm -f *~ *.d *.o $(TARGETS) *.png
