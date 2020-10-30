# Makefile, ECE252  
# Yiqing Huang

CC = gcc 
CFLAGS = -Wall -std=c99 -g # "curl-config --cflags" output is empty  
LD = gcc
LDFLAGS = -std=c99 -g 
LDLIBS = -lcurl -lz -pthread# "curl-config --libs" output 

SRCS    = main.c png.c producer.c queue.c
OBJS    = main.o png.o producer.o queue.o
TARGETS = paster2

all: ${TARGETS}

paster2: $(OBJS) 
	$(LD) -o $@ $^ $(LDLIBS) $(LDFLAGS)

%.o: %.c 
	$(CC) $(CFLAGS) -c $< 

%.d: %.c
	gcc -MM -MF $@ $<

-include $(SRCS:.c=.d)

.PHONY: clean
clean:
	rm -f *~ *.d *.o *~ $(TARGETS) *.png
