# Makefile, ECE252  
# Yiqing Huang, 2018/11/02

CC = gcc 
CFLAGS = -Wall -std=c99 -g # "curl-config --cflags" output is empty  
LD = gcc
LDFLAGS = -std=c99 -g 
LDLIBS = -lcurl -lz # "curl-config --libs" output 

SRCS   = paster.c crc.c zutil.c util.c ../../com/png.c
OBJS  = paster.o crc.o zutil.o util.o ../../com/png.o
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
