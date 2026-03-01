CC = gcc
CFLAGS = `sdl2-config --cflags` -I. -g -O1 
LDFLAGS = `sdl2-config --libs` -lcjson -lm 
TARGET = ernes
BUILDDIR = ./build


all: prebuild $(BUILDDIR)/general.o $(BUILDDIR)/memory.o $(BUILDDIR)/cpu.o $(BUILDDIR)/ppu.o $(BUILDDIR)/main.o $(BUILDDIR)/apu.o
	$(CC) $(BUILDDIR)/general.o $(BUILDDIR)/cpu.o $(BUILDDIR)/memory.o $(BUILDDIR)/ppu.o $(BUILDDIR)/main.o $(BUILDDIR)/apu.o -o $(BUILDDIR)/$(TARGET) $(LDFLAGS)

$(BUILDDIR)/cpu.o: cpu.c 
	$(CC) -o $(BUILDDIR)/cpu.o $(CFLAGS) -c cpu.c 

$(BUILDDIR)/memory.o: memory.c 
	$(CC) -o $(BUILDDIR)/memory.o $(CFLAGS) -c memory.c

$(BUILDDIR)/main.o: main.c
	$(CC) -o $(BUILDDIR)/main.o $(CFLAGS) -c main.c 

$(BUILDDIR)/ppu.o: ppu.c
	$(CC) -o $(BUILDDIR)/ppu.o $(CFLAGS) -c ppu.c

$(BUILDDIR)/general.o: general.c
	$(CC) -o $(BUILDDIR)/general.o $(CFLAGS) -c general.c

$(BUILDDIR)/apu.o: apu.c
	$(CC) -o $(BUILDDIR)/apu.o $(CFLAGS) -c apu.c

clean:
	rm -r ./build

prebuild: 
	-mkdir $(BUILDDIR)



windows: CC=x86_64-w64-mingw32-gcc-posix 
windows: TARGET=ernes.exe 
windows: all
	
	


