CC = gcc       # compiler
CFLAGS = -Wall -g -std=c99 # compilation flags
LD = gcc       # linker
LDFLAGS = -g   # debugging symbols in build
LDLIBS = -lz   # link with libz

# For students 
LIB_UTIL =  ../com/png.o ../com/crc.o
SRCS   = pnginfo.c ../com/png.c
OBJS   = pnginfo.o $(LIB_UTIL) 

TARGETS= pnginfo 

all: ${TARGETS}

pnginfo: $(OBJS) 
	$(LD) -o $@ $^ $(LDLIBS) $(LDFLAGS) 

%.o: %.c 
	$(CC) $(CFLAGS) -o $@ -c $<

%.d: %.c
	gcc -MM -MF $@ $<

.PHONY: clean
clean:
	rm -f *.d *.o $(TARGETS) 
