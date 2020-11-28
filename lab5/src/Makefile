# Makefile, ECE252  
# Yiqing Huang <yqhuang@uwaterloo.ca>

CC = gcc 
CFLAGS_XML2 = $(shell xml2-config --cflags)
CFLAGS_CURL = $(shell curl-config --cflags)
CFLAGS = -Wall $(CFLAGS_XML2) $(CFLAGS_CURL) -std=gnu99 -g
LD = gcc
LDFLAGS = -std=gnu99 -g 
LDLIBS_XML2 = $(shell xml2-config --libs)
LDLIBS_CURL = $(shell curl-config --libs)
LDLIBS = $(LDLIBS_XML2) $(LDLIBS_CURL) -lz -pthread -lrt -D _BSD_SOURCE

SRCS   = crawler.c png.c util.c findpng3.c
OBJS3  = crawler.o png.o util.o findpng3.o
TARGETS= findpng3

all: ${TARGETS}

findpng3: $(OBJS3) 
	$(LD) -o $@ $^ $(LDLIBS) $(LDFLAGS) 

%.o: %.c 
	$(CC) $(CFLAGS) -c $< 

-include $(SRCS:.c=.d)

.PHONY: clean
clean:
	rm -f *~ *.d *.o $(TARGETS) *.png *.html *.txt
