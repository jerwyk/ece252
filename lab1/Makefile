CC = gcc       # compiler
CFLAGS = -Wall -g -std=c99 # compilation flags
LD = gcc       # linker
LDFLAGS = -g   # debugging symbols in build
LDLIBS = -lz   # link with libz

# For students 
LIB_UTIL =  ../com/png.o ./starter/png_util/crc.o
SRCS   = pnginfo.c ../com/png.c pnginfo.c
OBJS1   = pnginfo.o  $(LIB_UTIL) 
OBJS2 = findpng.o ../com/png.o

TARGETS= pnginfo findpng

all: ${TARGETS}

pnginfo: $(OBJS1) 
	$(LD) -o $@ $^ $(LDLIBS) $(LDFLAGS) 

findpng: $(OBJS2) 
	$(LD) -o $@ $^ $(LDLIBS) $(LDFLAGS) 

%.o: %.c 
	$(CC) $(CFLAGS) -o $@ -c $<

%.d: %.c
	gcc -MM -MF $@ $<

.PHONY: clean
clean:
	rm -f *.d *.o $(TARGETS) 
