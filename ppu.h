/*

    ernes, a Nintendo Entertainment System emulator
    Copyright (C) 2026  Cameron Kelly

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.



*/


#pragma once
#include <stdint.h>
#include "memory.h"
#include "general.h"
#include <stdint.h>
#include <SDL2/SDL.h>




enum DeviceTypePPU {CHRRom, CHRRam, VRam};
typedef struct _Mem Mem;
typedef struct _Bus Bus;

typedef struct _PPUBus {

  Mem* memArr;
  int numOfBlocks;
  


} PPUBus;

struct VComponent {
  unsigned int courseX : 5;
  unsigned int courseY : 5;
  unsigned int nameTableSelect : 2;
  unsigned int fineY: 3;
};


// union between a 16 bit integer and VComp to act as the dual purpose register for holding both scroll position
// and the current VRAM address
union VRegister {
  uint16_t vreg;
  struct VComponent vcomp;

};

typedef struct _SecondaryOAM {
  uint8_t data[32];

  // amount of sprites found from sprite evaluation
  int spriteCounter;

} SecondaryOAM;

typedef struct _OamIndices{
  unsigned int n : 6;
  unsigned int m : 2;

} OAMIndices;

typedef struct _SpriteShifter{

  uint8_t xCoordinate;
  uint8_t bitPlaneLo;
  uint8_t bitPlaneHi;
  int bgPriorityFlag;
  uint8_t attributeData;


} SpriteShifter;

typedef struct _SpriteLatches{
  uint8_t yCoordinate;
  uint8_t tileNumber;
  uint8_t attributeData;
  uint8_t xCoordinate;
  uint8_t bitPlaneLo;
  uint8_t bitPlaneHi;

} SpriteLatch;

typedef struct _PPU {

  // internal ppu registers
  uint8_t ctrl;
  uint8_t mask;
  uint8_t status;
  uint8_t oamaddr;
  uint8_t oamdata;
  uint8_t oamdma;
  uint8_t yScroll;
  uint8_t xScroll;
  uint16_t addr;
  uint8_t data;
  

  // 0 -  vertical arrangement (horizontal mirroring)
  // 1 - horizontal arrangement (vertical mirroring)
  // 2 - one screen, lower bank
  // 3 - one screen, upper bank
  int mirroring;
  
  int dotx;
  int scanLine;

  // separate scanline variable for sprites because sprites are delayed by one scanline
  int scanLineSprites;


  // flag is set when the ppu is rendering the prerender scanline (scanline 261)
  int prerenderScanlineFlag;

  // CHR-ROM or RAM cartridge contents
  // 8192 array of bytes 
  uint8_t* chrrom;

  // if chrrom is writeable or not, 1 if it is writeable
  int flagChrRam;

  // should be initialized as [256]
  // used for sprites
  uint8_t* oam;

  // used to index into OAM
  OAMIndices oamIndex;


  PPUBus* ppubus;

  // 32 index array of bytes
  uint8_t* paletteram;

  // 0 - 1st read
  // 1 - 2nd read
  // this register is used when demultiplexing PPUSCROLL and PPUADDR
  int wregister; 
  

  // NOTE: The following registers require an understanding of https://www.nesdev.org/wiki/PPU_scrolling

  // fineX scroll component
  uint8_t xregister;
  
  // during rendering, used to hold the address of the currently fetched nametable address, outside of rendering, used to hold the VRAM address for reading/writing PPUDATA/PPUADDR
  union VRegister vregister;

  // during rendering, used to hold the scroll position (top left tile, or zero-th tile in the nametable).
  // outside of rendering, used to temporarily hold the VRAM address before transferring it to vregister
  union VRegister tregister;



  // 16-bit shift registers used for storing pattern table data
  uint16_t bitPlane1;
  uint16_t bitPlane2;



  // attribute data shift registers
  uint16_t attributeData1;
  uint16_t attributeData2;


  // latches that transfer into the appropriate shift registers every 8 PPU cycles
  uint8_t nameTableByteLatch;
  uint8_t attritebuteDataLatch;
  uint8_t bitPlaneLoLatch;
  uint8_t bitPlaneHiLatch;


  // variable to track whether the nes is in vertical blanking or not
  int vblank;

  // variable to track whether the nes is in horizontal blanking or not
  int hblank; 
  
  int frames;
  
  // scanline buffer gets appended to the framebuffer at the end of each rendering cycle
  //
  // the framebuffer gets parsed to the screen when a complete frame is drawn
  // (stores the 24-bit RGB value in an array)
  // frameBuffer should be initialized as [WINDOW_HEIGHT][WINDOW_WIDTH]
  uint32_t** frameBuffer;

  // nes palette to 24-bit RGB color
  // http://www.romdetectives.com/Wiki/index.php?title=NES_Palette

  uint32_t palette[0x40];

  int mapper;

  // used to select the bank for CNROM games
  int bankSelect;


  // a copy of the MMC1 registers from the Bus struct, stored here so that
  // they can be accessed in functions that only pass a PPU* pointer
  MMC1 mmc1Copy;

  // used to store the results of a sprite evaluation
  SecondaryOAM secondaryOam;
  SpriteShifter spriteShifters[8];

  // latch used to transfer data from secondaryOam to spriteShifters, ready for rendering on the next scanline
  SpriteLatch spriteLatch;
  int spriteLatchCounter;


  // flag raised when the first entry in the sprite shifter corresponds to sprite zero in the OAM
  int spriteZeroInRangeFlag;



  // state machine for keeping track of sprite evaluation step (used in multiple places)
  int spriteEvaluationStateMachine;



} PPU;

void initPpu(PPU*, int);
void resetPpu(PPU*, int);

void populatePalette(PPU*);


// render scanline merely parses the internal registers of the PPU and renders a scanline to a memory buffer
void renderScanline(PPU*);
void appendScanline(PPU*);
void vblankToggle(PPU*);
void vblankStart(Bus*);
void vblankEnd(Bus*);


void printNameTable(Bus*);

uint8_t findAndReturnAttributeByte(uint16_t, uint8_t);

void dmaTransfer(Bus*);

void renderScanlineForeground(PPU*);

int spriteEvaluation(PPU*, uint8_t*, int);

int getEightSixteen(PPU*);

// draws the completed framebuffer to screen in SDL
void drawFrameBuffer(PPU*, SDL_Renderer*, SDL_Texture*);


void incrementCourseX(PPU*);
void incrementY(PPU*);
void fetchFirstTwoTiles(PPU*);

void fillTempV(uint16_t*, struct VComponent); 
void copyMmc1(MMC1*, MMC1*);

void prerenderScanline(Bus*);

void tickPpu(Bus*);

uint8_t parseSpriteShifter(PPU*, int);