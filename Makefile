CC = gcc
# Uncomment for cross compiling the windows port
# CC = x86_64-w64-mingw32-gcc-posix
CFLAGS = `sdl2-config --cflags` -I. -g -O1 
LDFLAGS = `sdl2-config --libs` -lcjson -lm 
TARGET = ernes
BUILDDIR = ./build


all: prebuild general.o memory.o cpu.o ppu.o main.o apu.o
	$(CC) $(BUILDDIR)/general.o $(BUILDDIR)/cpu.o $(BUILDDIR)/memory.o $(BUILDDIR)/ppu.o $(BUILDDIR)/main.o $(BUILDDIR)/apu.o -o $(BUILDDIR)/$(TARGET) $(LDFLAGS)

cpu.o: cpu.c 
	$(CC) -o $(BUILDDIR)/cpu.o $(CFLAGS) -c cpu.c 

memory.o: memory.c 
	$(CC) -o $(BUILDDIR)/memory.o $(CFLAGS) -c memory.c

main.o: main.c
	$(CC) -o $(BUILDDIR)/main.o $(CFLAGS) -c main.c 

ppu.o: ppu.c
	$(CC) -o $(BUILDDIR)/ppu.o $(CFLAGS) -c ppu.c

general.o: general.c
	$(CC) -o $(BUILDDIR)/general.o $(CFLAGS) -c general.c

apu.o: apu.c
	$(CC) -o $(BUILDDIR)/apu.o $(CFLAGS) -c apu.c

clean:
	rm -r ./build

prebuild: 
	-mkdir $(BUILDDIR)



windows: CC=x86_64-w64-mingw32-gcc-posix 
windows: TARGET=ernes.exe 
windows: all
	
	


