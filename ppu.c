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


#include "ppu.h"
#include "cpu.h"
#include "general.h"
#include "memory.h"
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <stdint.h>
#include <stdio.h>




// initPpu()
//   initializes PPU
void initPpu(PPU* ppu, int banks){

  ppu->oam = calloc(256, sizeof(uint8_t));
  ppu->paletteram = calloc(32, sizeof(uint8_t));
  
  ppu->ppubus = calloc(1, sizeof(PPUBus));

  if(banks > 0){
    ppu->ppubus->memArr = calloc(banks, sizeof(Mem));
  } else if(banks == 0){
    ppu->ppubus->memArr = calloc(1, sizeof(Mem));
  }
  ppu->ppubus->numOfBlocks = banks;
  ppu->frameBuffer = malloc(sizeof(uint32_t*) * FRAMEBUFFER_HEIGHT);
  
  for(int i = 0; i < FRAMEBUFFER_HEIGHT; ++i){
    ppu->frameBuffer[i] = calloc(FRAMEBUFFER_WIDTH, sizeof(uint32_t));
  } 

  
  ppu->ctrl = 0;
  ppu->mask = 0;
  ppu->status = 0;
  ppu->oamaddr = 0;
  ppu->oamdata = 0;
  ppu->addr = 0;
  ppu->oamdma = 0;

  ppu->scanLine = 0;
  ppu->frames = 0;
  ppu->renderer = 0;
  ppu->win = 0;
  ppu->texture = 0;


  
}


// powerFlag is used to denote whether the power is used
// for a power up as opposed to just a reset
void resetPpu(PPU* ppu, int powerFlag){
  ppu->ctrl = 0;
  ppu->mask = 0;
  ppu->status = (getBit(ppu->status, 7) | 0);
  if(powerFlag == 1){
    ppu->oamaddr = 0;
    ppu->addr = 0;
    ppu->status = 0b10100000;
  }

  ppu->wregister = 0;
  ppu->vblank = 0;
  ppu->xScroll = 0;
  ppu->yScroll = 0;
  ppu->scanLine = 0;
  ppu->dotx = 0;
  ppu->frames = 0;

  ppu->xregister = 0;
  ppu->vregister.vreg = 0;
  ppu->tregister.vreg = 0;

  ppu->bitPlane1 = 0;
  ppu->bitPlane2 = 0;
  ppu->data = 0;

  ppu->scanLineSprites = -1;

  ppu->prerenderScanlineFlag = 0;
  ppu->attributeData1 = 0;
  ppu->attributeData2 = 0;

  ppu->hblank = 0;
  ppu->flagChrRam = 0;

  ppu->attritebuteDataLatch = 0;
  ppu->nameTableByteLatch = 0;
  ppu->bitPlaneHiLatch = 0;
  ppu->bitPlaneLoLatch = 0;
  ppu->secondaryOam.spriteCounter = 0;
  ppu->oamIndex.n = 0;
  ppu->oamIndex.m = 0;
  ppu->spriteZeroOnNextScanline = 0;
  ppu->spriteZeroOnThisScanline = 0;

  ppu->spriteEvaluationStateMachine = 0;
  ppu->spriteLatchCounter = 0;

  ppu->frameRendering.freq = SDL_GetPerformanceFrequency();
  
  for(int i = 0; i < 8; ++i){

    resetSpriteShifter(&ppu->spriteShifters[i]);

  }
  

}

void resetSpriteShifter(SpriteShifter* spriteShifter){
  spriteShifter->attributeData = 0;
  spriteShifter->bgPriorityFlag = 0;
  spriteShifter->bitPlaneHi = 0;
  spriteShifter->bitPlaneLo = 0;
  spriteShifter->xCoordinate = 0;


}

// dmaTransfer()
//   used to facilitate the dma transfer between CPU memory and PPU OAM (copies a page of 0xff bytes into oam)
// inputs:
//   bus - machine to act upon
void dmaTransfer(Bus* bus){
  uint16_t addr;

  for(int i = 0; i <= 0xff; ++i){
    addr = (((uint16_t)bus->ppu->oamdma) << 8) | i;

    // cycles the rest of the system 255 times
    bus->ppu->oam[i] = readBus(bus, addr);
  }
  // cycles to perform so that a total of 513 or 514 cycles are used on the
  // dma transfer.
  
  for(int i = 0; i < 258; ++i){
    tickPpu(bus);
    tickPpu(bus);
    tickPpu(bus);
  }
  if(bus->cpu->cycles % 2 != 0){
    tickPpu(bus);
    tickPpu(bus);
    tickPpu(bus);
  }
  

}

// populatePalette()
//   hard-coding 24-bit rgb values for each of the nes' 64 colours
//   table for nes to 24bit color conversion
void populatePalette(PPU* ppu){

  
  ppu->palette[0]  = 0x626262;
  ppu->palette[1]  = 0x001fb2;
  ppu->palette[2]  = 0x2404c8;
  ppu->palette[3]  = 0x5200b2;
  ppu->palette[4]  = 0x730076;
  ppu->palette[5]  = 0x800024;
  ppu->palette[6]  = 0x730b00;
  ppu->palette[7]  = 0x522800;
  ppu->palette[8]  = 0x244400;
  ppu->palette[9]  = 0x005700;
  ppu->palette[10] = 0x005c00;
  ppu->palette[11] = 0x005324;
  ppu->palette[12] = 0x003c76;
  ppu->palette[13] = 0x000000;
  ppu->palette[14] = 0x000000;
  ppu->palette[15] = 0x000000;
  
  ppu->palette[16] = 0xababab;
  ppu->palette[17] = 0x0d57ff;
  ppu->palette[18] = 0x4b30ff;
  ppu->palette[19] = 0x8a13ff;
  ppu->palette[20] = 0xbc08d6;
  ppu->palette[21] = 0xd21269;
  ppu->palette[22] = 0xc72e00;
  ppu->palette[23] = 0x9d5400;
  ppu->palette[24] = 0x607b00;
  ppu->palette[25] = 0x209800;
  ppu->palette[26] = 0x00a300;
  ppu->palette[27] = 0x009942;
  ppu->palette[28] = 0x007db4;
  ppu->palette[29] = 0x000000;
  ppu->palette[30] = 0x000000;
  ppu->palette[31] = 0x000000;
  
  ppu->palette[32] = 0xffffff;
  ppu->palette[33] = 0x53aeff;
  ppu->palette[34] = 0x9085ff;
  ppu->palette[35] = 0xd365ff;
  ppu->palette[36] = 0xff57ff;
  ppu->palette[37] = 0xff5dcf;
  ppu->palette[38] = 0xff7757;
  ppu->palette[39] = 0xfa9e00;
  ppu->palette[40] = 0xbdc700;
  ppu->palette[41] = 0x7ae700;
  ppu->palette[42] = 0x43f611;
  ppu->palette[43] = 0x26ef7e;
  ppu->palette[44] = 0x2cd5f6;
  ppu->palette[45] = 0x4e4e4e;
  ppu->palette[46] = 0x000000;
  ppu->palette[47] = 0x000000;
  
  ppu->palette[48] = 0xffffff;
  ppu->palette[49] = 0xb6e1ff;
  ppu->palette[50] = 0xced1ff;
  ppu->palette[51] = 0xe9c3ff;
  ppu->palette[52] = 0xffbcff;
  ppu->palette[53] = 0xffbdf4;
  ppu->palette[54] = 0xffc6c3;
  ppu->palette[55] = 0xffd59a;
  ppu->palette[56] = 0xe9e681;
  ppu->palette[57] = 0xcef481;
  ppu->palette[58] = 0xb6fb9a;
  ppu->palette[59] = 0xa9fac3;
  ppu->palette[60] = 0xa9f0f4;
  ppu->palette[61] = 0xb8b8b8;
  ppu->palette[62] = 0x000000;
  ppu->palette[63] = 0x000000;
  


}




// pollControllers()
//    polls controllers for SDL
void pollControllers(Bus* bus){


  SDL_Event event;

  // polls for events at the end of each prerender scanline (once per frame)
  while (SDL_PollEvent(&event)) {
  switch (event.type) {
    case SDL_QUIT:
      freeAndExit(bus);
      break;

    case SDL_KEYDOWN:
      switch(event.key.keysym.sym){
        case SDLK_x:
          bus->controller1.sdlButtons = setBit(bus->controller1.sdlButtons, 0);
          break;
        case SDLK_z:
          bus->controller1.sdlButtons = setBit(bus->controller1.sdlButtons, 1);
          break;
        case SDLK_RSHIFT:
          bus->controller1.sdlButtons = setBit(bus->controller1.sdlButtons, 2);
          break;
        case SDLK_RETURN:
          bus->controller1.sdlButtons = setBit(bus->controller1.sdlButtons, 3);
          break;
        case SDLK_UP:
          bus->controller1.sdlButtons = setBit(bus->controller1.sdlButtons, 4);
          break;
        case SDLK_DOWN:
          bus->controller1.sdlButtons = setBit(bus->controller1.sdlButtons, 5);
          break;
        case SDLK_LEFT:
          bus->controller1.sdlButtons = setBit(bus->controller1.sdlButtons, 6);
          break;
        case SDLK_RIGHT:
          bus->controller1.sdlButtons = setBit(bus->controller1.sdlButtons, 7);
          break;
        case SDLK_r:
            reset(bus->cpu, bus);
            resetPpu(bus->ppu, 0);
            resetApu(bus->apu);
          break;

      }
      break;
    case SDL_KEYUP:
      switch(event.key.keysym.sym){
        case SDLK_x:
          bus->controller1.sdlButtons = clearBit(bus->controller1.sdlButtons, 0);
          break;
        case SDLK_z:
          bus->controller1.sdlButtons = clearBit(bus->controller1.sdlButtons, 1);
          break;
        case SDLK_RSHIFT:
          bus->controller1.sdlButtons = clearBit(bus->controller1.sdlButtons, 2);
          break;
        case SDLK_RETURN:
          bus->controller1.sdlButtons = clearBit(bus->controller1.sdlButtons, 3);
          break;
        case SDLK_UP:
          bus->controller1.sdlButtons = clearBit(bus->controller1.sdlButtons, 4);
          break;
        case SDLK_DOWN:
          bus->controller1.sdlButtons = clearBit(bus->controller1.sdlButtons, 5);
          break;
        case SDLK_LEFT:
          bus->controller1.sdlButtons = clearBit(bus->controller1.sdlButtons, 6);
          break;                  

        case SDLK_RIGHT:
          bus->controller1.sdlButtons = clearBit(bus->controller1.sdlButtons, 7);
          break;
      }
      break;
    }
  }

}


// drawFramebuffer()
//   draws Framebuffer to background layer in sdl 
//   also converts the nes colour palette to rgb values when reading from the frame buffer
void drawFrameBuffer(Bus* bus){
  //printf("drawing framebuffer \n");
  uint32_t *pixels;
  int pitch;
  double elasped_ms;
  const double target_fps = 60.0;
  const double target_frame_time = 1000.0 / target_fps;

  SDL_RenderClear(bus->ppu->renderer);
  SDL_LockTexture(bus->ppu->texture, NULL, (void**)&pixels, &pitch);

  for(int i = 0; i < FRAMEBUFFER_HEIGHT; ++i){
    for(int j = 0; j < FRAMEBUFFER_WIDTH; ++j){
      pixels[(i * FRAMEBUFFER_WIDTH) + j] = bus->ppu->frameBuffer[i][j];

    }
  }

  SDL_UnlockTexture(bus->ppu->texture);
  SDL_RenderCopy(bus->ppu->renderer, bus->ppu->texture, NULL, NULL);
  SDL_RenderPresent(bus->ppu->renderer);


  // we poll controllers here because this way we can do it one time exactly 
  // on scanline 261 and dot 340 only
  pollControllers(bus);


  // delay until 16.6667 ms has elasped
  bus->ppu->frameRendering.frame_end = SDL_GetPerformanceCounter();
    
  elasped_ms = (bus->ppu->frameRendering.frame_end - bus->ppu->frameRendering.frame_start) * 1000.0 / bus->ppu->frameRendering.freq;
  if(elasped_ms < target_frame_time){
    SDL_Delay((uint32_t)(target_frame_time - elasped_ms));
  }

}

void printNameTable(Bus* bus){

  printf("-------------------------------- \n");
  for(int i = 0; i < 0x1f; ++i){
    printf("%x ", i);
    for(int j = 0; j < 0x20; ++j){
      
      printf("%x ", readPpuBus(bus->ppu, 0x2000 + j + (32 * i)));

    }
    printf("\n");
  }
  printf("-------------------------------- \n");

}


// copyMmc1()
//   meant to copy a set of MMC1 registers from one struct to another
void copyMmc1(MMC1* src, MMC1* dest){
  dest->chrBank0.reg = src->chrBank0.reg;
  dest->chrBank1.reg = src->chrBank1.reg;
  dest->control.reg = src->control.reg;
  dest->prgBank.reg = src->prgBank.reg;
  
}




// function to parse the approriate sprite shifter, with index number
uint8_t parseSpriteShifter(PPU* ppu, int index){
  uint8_t bitLo;
  uint8_t bitHi;
  uint8_t bitsCombined;

   if(ppu->spriteShifters[index].xCoordinate == 0){
    bitLo = getBitFromLeft(ppu->spriteShifters[index].bitPlaneLo, 0);
    bitHi = getBitFromLeft(ppu->spriteShifters[index].bitPlaneHi, 0);
    bitHi = bitHi >> 6;
    bitLo = bitLo >> 7;
    bitsCombined = bitLo | bitHi;
    ppu->spriteShifters[index].bitPlaneLo <<= 1;
    ppu->spriteShifters[index].bitPlaneHi <<= 1;    
   } else {
    bitsCombined = 0;
   }

   return bitsCombined;


}

// spriteOutputProcess()
//     enumerates through the sprite shifters (filled during the previous line in spriteEvaluationAndProcess()) to find the pixel to output.
//     the first sprite shifter to output a non-zero value is selected.
uint8_t spriteOutputProcess(PPU* ppu, uint8_t finalBackgroundPixel, uint8_t* spritePriority){

  uint8_t finalSpritePixel = 0;
  uint8_t bitsCombined = 0;
  for(int i = 0; i < 8; ++i){

      // we use the bitsCombined variable as a state-machine of sorts.
      // if bitsCombined == 0, then we know that we haven't found the first non-zero output from a 
      // sprite-shifter yet.
      if(bitsCombined == 0){
        bitsCombined = parseSpriteShifter(ppu, i);
      } else {
        // needed to shift the sprite shifters after a non-zero bit has been found
        parseSpriteShifter(ppu, i);
      }
  
      // sprite zero hit
      if(i == 0 && ppu->spriteZeroOnThisScanline == 1){
        if(bitsCombined != 0 && finalBackgroundPixel != 0){
          ppu->status = setBit(ppu->status, 6);
          ppu->spriteZeroOnThisScanline = 0;

        }
      }
      
      // if the sprite pixel hasn't been found yet and one was just found
      if(finalSpritePixel == 0 && bitsCombined != 0){

        // finalSpritePixel is a 4-bit value, combining the attribute data with the two bits
        // that were just found from the sprite shifters. 
        finalSpritePixel = bitsCombined | (ppu->spriteShifters[i].attributeData << 2);
        *spritePriority = ppu->spriteShifters[i].bgPriorityFlag;
        // there is no break here because despite finding the first bit that is non-zero, we still need
        // to iterate through the sprite shifters to shift the shift registers by one, to keep everything in line
      }
  }
  
  for(int i = 0; i < 8; ++i){
    // decrement all sprite shifter's x-coordinate by one because this is how we keep track
    // of how far the beam is across the screen in relation to when the sprite shifter should
    // be outputting something
    if(ppu->spriteShifters[i].xCoordinate > 0){
      ppu->spriteShifters[i].xCoordinate--;
    }
  }

  // no sprite rendering on dotx <= 8
  if(ppu->dotx <= 8 && getBit(ppu->mask, 2) == 0){
    finalSpritePixel = 0;

  }

  return finalSpritePixel;


}

// priorityMux()
//     selects background, foreground, or backdrop pixel to display and paints this to the framebuffer
void priorityMux(PPU* ppu, uint8_t finalSpritePixel, uint8_t finalBackgroundPixel, uint8_t spritePriority){

    uint8_t finalPixel;

      // priority mux then selects a pixel
      if((finalSpritePixel & 0b11) == 0){
        if((finalBackgroundPixel & 0b11) == 0){
          // draw the backdrop colour
          finalPixel = 0;

        } else if ((finalBackgroundPixel & 0b11) != 0){
          finalPixel = finalBackgroundPixel;
        }

      } else if((finalSpritePixel & 0b11) != 0){
        if((finalBackgroundPixel & 0b11) != 0){
          if(spritePriority == 0){
            finalPixel = setBit(finalSpritePixel, 4);
          } else {
            
            finalPixel = finalBackgroundPixel;

          }
        } else {
          finalPixel = setBit(finalSpritePixel, 4);

        }

      }
      ppu->frameBuffer[ppu->scanLine][ppu->dotx - 1] = ppu->palette[readPpuBus(ppu, 0x3f00 + finalPixel)];

}


// shiftShiftRegisters()
//     shifts PPU internal shift registers
void shiftShiftRegisters(PPU* ppu){

  ppu->bitPlane1 = ppu->bitPlane1 << 1;
  ppu->bitPlane2 = ppu->bitPlane2 << 1;
  ppu->attributeData1 = ppu->attributeData1 << 1;
  ppu->attributeData2 = ppu->attributeData2 << 1;

}

// fetchAndFillLatches()
//     performs PPU VRAM fetches to fill the internal latches on the corresponding dot with the v-register
void fetchAndFillLatches(PPU* ppu, uint16_t tempV){

  uint16_t patternTableOffset = 0;
  if(ppu->dotx % 8 == 1){
    #if TICKPPUDEBUGLOG == 1
    printf("\t fetching nametable byte at %x \n", 0x2000 + tempV);
    #endif


    ppu->nameTableByteLatch = readPpuBus(ppu, 0x2000 + tempV);

  } else if(ppu->dotx % 8 == 3){
    #if TICKPPUDEBUGLOG == 1
      printf("\t fetching attribute data at %x \n", 0x23c0 | (tempV & 0x0c00) | ((tempV >> 4) & 0x38) | ((tempV >> 2) & 0x07));
      #endif
      ppu->attritebuteDataLatch = readPpuBus(ppu, 0x23c0 | (tempV & 0x0c00) | ((tempV >> 4) & 0x38) | ((tempV >> 2) & 0x07));

      // finds which quadrant v resides in and returns the appropriate 2-bit number from the byte
      ppu->attritebuteDataLatch = findAndReturnAttributeByte(tempV,  ppu->attritebuteDataLatch);

  } else if(ppu->dotx % 8 == 5){

    // fetch pattern table tile lo byte
    if(getBit(ppu->ctrl, 4) == 0){
      patternTableOffset = 0;
    } else if (getBit(ppu->ctrl, 4) != 0){
      patternTableOffset = 0x1000;
    }
    ppu->bitPlaneLoLatch = readPpuBus(ppu, patternTableOffset + (ppu->nameTableByteLatch << 4) + ppu->vregister.vcomp.fineY);

    #if TICKPPUDEBUGLOG == 1
    printf("\t fetching pattern tile lo byte: %x \n", bus->ppu->bitPlaneLoLatch);
    #endif
  } else if(ppu->dotx % 8 == 7){

    if(getBit(ppu->ctrl, 4) == 0){
      patternTableOffset = 0;
    } else if (getBit(ppu->ctrl, 4) != 0){
      patternTableOffset = 0x1000;
    }
    ppu->bitPlaneHiLatch = readPpuBus(ppu, patternTableOffset + (ppu->nameTableByteLatch << 4) + ppu->vregister.vcomp.fineY + 8);

    #if TICKPPUDEBUGLOG == 1
    printf("\t fetching pattern tile hi byte: %x \n", bus->ppu->bitPlaneHiLatch);
    #endif

  } else if(ppu->dotx % 8 == 0){
    #if TICKPPUDEBUGLOG == 1
    printf("\t incrementing course x \n");        
    #endif

    // if statement to account for unused tile fetch (occurs from dots 249-255), do not latch the contents of this
    if(ppu->dotx != 256){
      #if TICKPPUDEBUGLOG == 1
      printf("\t transferring data into shift registers for nametable tile %x \n", 0x2000 + tempV);
      #endif

      // sets or clears the latched data into the attributedata shift register
      if(getBit(ppu->attritebuteDataLatch , 0) == 1){
        ppu->attributeData1 = ppu->attributeData1 | 0x00ff;
      } else if(getBit(ppu->attritebuteDataLatch, 0) == 0){
        ppu->attributeData1 = ppu->attributeData1 & 0xff00;
      }

      if(getBit(ppu->attritebuteDataLatch , 1) == 0b10){
        ppu->attributeData2 = ppu->attributeData2 | 0x00ff;
      } else if(getBit(ppu->attritebuteDataLatch, 1) == 0){
        ppu->attributeData2 = ppu->attributeData2 & 0xff00;
      }

    
      ppu->bitPlane1 = ppu->bitPlane1 | (uint16_t)ppu->bitPlaneLoLatch;
      ppu->bitPlane2 = ppu->bitPlane2 | (uint16_t)ppu->bitPlaneHiLatch;
    }
    
  }


}


// backgroundOutputProcess()
//     fetches bits from attribute and background shift registers to 
//     come up with the final background pixel to be outputted. 
uint8_t backgroundOutputProcess(PPU* ppu){
  uint16_t currentAttributeData;
  uint16_t currentAttributeDataLo;
  uint16_t currentAttributeDataHi;
  uint16_t bitLo_16;
  uint16_t bitHi_16;
  uint8_t bitsCombinedBackground = 0;
  uint8_t finalBackgroundPixel = 0;



    
    
  // ***** Background output process *****
  if(ppu->scanLine >= 0 && ppu->scanLine <= 239 && ppu->dotx >= 1 && ppu->dotx <= 256){
    #if TICKPPUDEBUGLOG == 1
      printf("\t drawing a pixel \n");

      #endif
      // fetch data from shift registers for the current pixel
      currentAttributeDataLo = 0;
      currentAttributeDataLo = (getBitFromLeft16bit(ppu->attributeData1, ppu->xregister));
      currentAttributeDataLo = currentAttributeDataLo >> (15 - ppu->xregister);
      
      currentAttributeDataHi = 0;
      currentAttributeDataHi = (getBitFromLeft16bit(ppu->attributeData2, ppu->xregister));
      currentAttributeDataHi = currentAttributeDataHi >> (15 - ppu->xregister);
      currentAttributeDataHi = currentAttributeDataHi << 1;
      

      currentAttributeData = currentAttributeDataLo | currentAttributeDataHi;


      // using data from the shift register, get the two bits from the bitplanes from the end of the shift registers
      // and bitLo_16 is a 16 bit integer
      bitLo_16 = getBitFromLeft16bit(ppu->bitPlane1, ppu->xregister);
      bitHi_16 = getBitFromLeft16bit(ppu->bitPlane2, ppu->xregister);

      // we are doing 15 - ppu->xregister because this will get us the bit in the first position
      bitLo_16 = bitLo_16 >> (15 - ppu->xregister);
      bitHi_16 = bitHi_16 >> (15 - ppu->xregister);
      bitHi_16 = bitHi_16 << 1;
      bitsCombinedBackground = bitLo_16 | bitHi_16;

      finalBackgroundPixel = bitsCombinedBackground | (currentAttributeData << 2);

      // if backgrounds are disabled in first 8px column, output a zero
      if(ppu->dotx <= 8 && getBit(ppu->mask, 1) == 0){
        finalBackgroundPixel = 0;
        bitsCombinedBackground = 0;

      }

      // find 24Bit rgb value and set the current pixel value to this
      // bus->ppu->frameBuffer[bus->ppu->scanLine][bus->ppu->dotx] = bus->ppu->palette[tempPalette[bitsCombined]];
      // printf("\t shifter value while drawing pixel Lo: %x Hi: %x \n", bus->ppu->bitPlane1, bus->ppu->bitPlane2);


  } else {
    finalBackgroundPixel = 0;
    bitsCombinedBackground = 0;

  }
  
  return finalBackgroundPixel;



}


// vRegisterLogic()
//     used to increment and reset the v-register at the correct dots and scanlines
void vRegisterLogic(PPU* ppu){
 if((ppu->scanLine >= 0 && ppu->scanLine <= 239) || ppu->scanLine == 261){

      if((ppu->dotx >= 1 && ppu->dotx <= 256) || (ppu->dotx >= 321 && ppu->dotx <= 336)){
        if(ppu->dotx % 8 == 0){

          incrementCourseX(ppu);

        }
      }
      if(ppu->dotx == 256){

      // printf("\t incrementing y \n");
        incrementY(ppu);


      } else if(ppu->dotx == 257){
        #if TICKPPUDEBUGLOG == 1
         printf("\t hori(v) = hori(t) \n");
         #endif
 
        // hori(v) = hori(t)
        ppu->vregister.vcomp.courseX = ppu->tregister.vcomp.courseX;
        ppu->vregister.vcomp.nameTableSelect = getBit(ppu->vregister.vcomp.nameTableSelect, 1) | getBit(ppu->tregister.vcomp.nameTableSelect, 0);
 
      }
    }
    
    
    if(ppu->scanLine == 261 && ppu->dotx >= 280 && ppu->dotx <= 304){
      // vert(v) = vert(t)
      // copy all y components from t to v
      #if TICKPPUDEBUGLOG == 1
      printf("\t vert(v) = vert(t) \n");
      #endif
      ppu->vregister.vcomp.courseY = ppu->tregister.vcomp.courseY;
      ppu->vregister.vcomp.fineY = ppu->tregister.vcomp.fineY;
      ppu->vregister.vcomp.nameTableSelect = getBit(ppu->vregister.vcomp.nameTableSelect, 0) | getBit(ppu->tregister.vcomp.nameTableSelect, 1);

    } 


}



// spriteEvaluationAndProcess()
//   not to be confused with spriteOutputProcess(), this function, over the course of a scanline,
//   performs sprite evaluation and does memory fetches and fills the sprite latches for display on the next scanline.
void spriteEvaluationAndProcess(PPU* ppu){

  uint16_t patternTableOffset;
  uint8_t tileIndex;

    if((ppu->scanLine >= 0 && ppu->scanLine <= 239)){
      if(ppu->dotx == 1){
        ppu->oamIndex.m = 0;
        ppu->oamIndex.n = 0;
        ppu->spriteEvaluationStateMachine = 0;
      }


      if(ppu->dotx >= 1 && ppu->dotx <= 64){

        // clear secondary OAM
        #if TICKPPUDEBUGLOG == 1
        printf("\t clearing oam \n");
        #endif
        ppu->secondaryOam.data[(ppu->dotx - 1) % 32] = 0xff;
      } else if(ppu->dotx >= 65 && ppu->dotx <= 256){

        // we now do a linear search through the OAM and if it's in range of the y, we copy it into secondary OAM.
        if(ppu->dotx == 65){
          ppu->spriteEvaluationStateMachine = 0;
          ppu->secondaryOam.spriteCounter = 0;
          ppu->spriteZeroOnNextScanline = 0;
        }
        
        if(ppu->spriteEvaluationStateMachine == 0){
          if(ppu->secondaryOam.spriteCounter <= 7){
            // copy y coordinate index n into oam into secondary oam
            ppu->secondaryOam.data[ppu->secondaryOam.spriteCounter * 4] = ppu->oam[ppu->oamIndex.n * 4];
            

          }

          if(getBit(ppu->ctrl, 5) == 0){
            // if y coordinate is in range of the scanline, copy the rest of the OAM contents into secondaryOAM
            if((ppu->secondaryOam.data[ppu->secondaryOam.spriteCounter * 4] <= (ppu->scanLine & 0xff)) && ((ppu->secondaryOam.data[ppu->secondaryOam.spriteCounter * 4] + 7) >= (ppu->scanLine & 0xff))){
              for(int i = 1; i < 4; ++i){
                ppu->secondaryOam.data[(ppu->secondaryOam.spriteCounter * 4) + i] = ppu->oam[(ppu->oamIndex.n * 4) + i]; 
              }

              if(ppu->oamIndex.n == 0){
                ppu->spriteZeroOnNextScanline = 1;

              }

              // increment spriteCounter because we now want to reserve the next slot in Secondary OAM
              ppu->secondaryOam.spriteCounter++;
            } else {
              ppu->secondaryOam.data[ppu->secondaryOam.spriteCounter * 4] = 0xff;
            }
            ppu->spriteEvaluationStateMachine = 1;

          } else {
            if((ppu->secondaryOam.data[ppu->secondaryOam.spriteCounter * 4] <= (ppu->scanLine & 0xff)) && ((ppu->secondaryOam.data[ppu->secondaryOam.spriteCounter * 4] + 15) >= (ppu->scanLine & 0xff))){
              for(int i = 1; i < 4; ++i){
                ppu->secondaryOam.data[(ppu->secondaryOam.spriteCounter * 4) + i] = ppu->oam[(ppu->oamIndex.n * 4) + i]; 
              }
              if(ppu->oamIndex.n == 0){
                ppu->spriteZeroOnNextScanline = 1;

              }
              ppu->secondaryOam.spriteCounter++;
            } else {
              ppu->secondaryOam.data[ppu->secondaryOam.spriteCounter * 4] = 0xff;
            }
            ppu->spriteEvaluationStateMachine = 1;

          }

        } else if(ppu->spriteEvaluationStateMachine == 1){
        
          ppu->oamIndex.n++;

          if(ppu->oamIndex.n == 0){
            ppu->spriteEvaluationStateMachine = 3;
          } else if(ppu->secondaryOam.spriteCounter < 8){
            ppu->spriteEvaluationStateMachine = 0;
          } 

        } else if(ppu->spriteEvaluationStateMachine == 2){
          if(ppu->oam[((ppu->oamIndex.m * 4) + ppu->oamIndex.n)] >= ppu->scanLine && ppu->oam[((ppu->oamIndex.m * 4) + ppu->oamIndex.n)] + 7 <= ppu->scanLine){
            ppu->status = clearBit(ppu->status, 5);
            // TODO: implement sprite overflow bug
            
          }

        } 

        if(ppu->dotx == 256){
          ppu->spriteZeroOnThisScanline = ppu->spriteZeroOnNextScanline;
        }
        
        


      }
    }


    // we now use the secondary OAM to transfer the data into lo and hi bitplanes, used to be stored into the 8 sprite latches
    if((ppu->scanLine >= 0 && ppu->scanLine <= 239) || ppu->scanLine == 261){
      if(ppu->dotx >= 257 && ppu->dotx <= 320){
        
        if(ppu->dotx >= 257 && ppu->dotx <= 319){
          ppu->spriteZeroOnThisScanline = ppu->spriteZeroOnNextScanline;
        }
        
        if(ppu->dotx == 257){
          ppu->spriteLatchCounter = 0;
        }

        // on first 4 cycles, read in yCoordinate, tilenumber, xcoordinate, and attributedata to the latch
        if(ppu->dotx % 8 == 1){

          ppu->spriteLatch.yCoordinate = ppu->secondaryOam.data[ppu->spriteLatchCounter];
          

        } else if(ppu->dotx % 8 == 2){

          ppu->spriteLatch.tileNumber = ppu->secondaryOam.data[ppu->spriteLatchCounter + 1];
        

        } else if(ppu->dotx % 8 == 3){


          ppu->spriteLatch.attributeData = ppu->secondaryOam.data[ppu->spriteLatchCounter + 2];




        } else if(ppu->dotx % 8 == 4){
            
            ppu->spriteLatch.xCoordinate = ppu->secondaryOam.data[ppu->spriteLatchCounter + 3];
          
          
          

        } else if(ppu->dotx % 8 == 5){
          // choose between 8x16 or 8x8 sprite mode
          if(getBit(ppu->ctrl, 5) == 0){


            if(getBit(ppu->ctrl, 3) != 0){
              patternTableOffset = 0x1000;
            } else {
              patternTableOffset = 0;
            }


            if((ppu->spriteLatch.yCoordinate <= (ppu->scanLine & 0xff)) && ((ppu->spriteLatch.yCoordinate + 8) >= (ppu->scanLine & 0xff))){
              if(getBit(ppu->spriteLatch.attributeData, 7) == 0){
                ppu->spriteLatch.bitPlaneLo = readPpuBus(ppu, patternTableOffset + (ppu->spriteLatch.tileNumber << 4) + (ppu->scanLine  - ppu->spriteLatch.yCoordinate));
      
              } else {
 
                ppu->spriteLatch.bitPlaneLo = readPpuBus(ppu, patternTableOffset + (ppu->spriteLatch.tileNumber << 4) + (7 - (ppu->scanLine - ppu->spriteLatch.yCoordinate)));
  
              }
            } else {
              ppu->spriteLatch.bitPlaneLo = 0;

            }


          } else {

            // in 8x16 mode, the patterntable offset is determined by the first bit of the tile number
            if((ppu->spriteLatch.tileNumber & 0b1) == 1){
              patternTableOffset = 0x1000;

            } else {
              patternTableOffset = 0;
            }

            if((ppu->spriteLatch.yCoordinate <= (ppu->scanLine & 0xff)) && ((ppu->spriteLatch.yCoordinate + 16) >= (ppu->scanLine & 0xff))){

              
              tileIndex = (ppu->scanLine & 0xff) - ppu->spriteLatch.yCoordinate;
              uint16_t tileNumberTemp;

              // choose between flipping the sprite vertically or not
              if(getBit(ppu->spriteLatch.attributeData, 7) == 0){

                if(tileIndex <= 7){

                  tileNumberTemp = ppu->spriteLatch.tileNumber & 0xfe;

                } else {

                  tileIndex -= 8;
                  tileNumberTemp = (ppu->spriteLatch.tileNumber & 0xfe) + 1;

                }
                
                ppu->spriteLatch.bitPlaneLo = readPpuBus(ppu, patternTableOffset + (tileNumberTemp << 4) + tileIndex);
              } else {

                if(tileIndex <= 7){
                  // if mirroring is enabled, fetch the second tile first

                  
                  tileNumberTemp = (ppu->spriteLatch.tileNumber & 0xfe) + 1;
                } else {

                  // minus 8 because we are now > 7, so we need to account for this offset and minus 8 to get the correct offset into memory
                  tileIndex -= 8;
                  tileNumberTemp = (ppu->spriteLatch.tileNumber & 0xfe);
                }
  
                
                ppu->spriteLatch.bitPlaneLo = readPpuBus(ppu, patternTableOffset + (tileNumberTemp << 4) + (7 - tileIndex));

              }
            } else {
              ppu->spriteLatch.bitPlaneLo = 0;

            }

          }


          if(getBit(ppu->spriteLatch.attributeData, 6) != 0){    

            // mirror the bits if necessary
            uint8_t bitPlaneTemp = 0;
            for(int i = 0; i < 8; ++i){
              if(getBitFromLeft(ppu->spriteLatch.bitPlaneLo, i) != 0){
                bitPlaneTemp = setBit(bitPlaneTemp, i);
              } else {
                bitPlaneTemp = clearBit(bitPlaneTemp, i);
              }
            }
            ppu->spriteLatch.bitPlaneLo = bitPlaneTemp;
          }

        } else if(ppu->dotx % 8 == 7){


          if(getBit(ppu->ctrl, 5) == 0){

            if(getBit(ppu->ctrl, 3) != 0){
              patternTableOffset = 0x1000;
            } else {
              patternTableOffset = 0;
            }
            if((ppu->spriteLatch.yCoordinate <= (ppu->scanLine & 0xff)) && ((ppu->spriteLatch.yCoordinate + 8) >= (ppu->scanLine & 0xff))){
              if(getBit(ppu->spriteLatch.attributeData, 7) == 0){
                ppu->spriteLatch.bitPlaneHi = readPpuBus(ppu, patternTableOffset + (ppu->spriteLatch.tileNumber << 4) + ((ppu->scanLine - ppu->spriteLatch.yCoordinate) + 8));
              } else {
                ppu->spriteLatch.bitPlaneHi = readPpuBus(ppu, patternTableOffset + (ppu->spriteLatch.tileNumber << 4) + ((7 - (ppu->scanLine - ppu->spriteLatch.yCoordinate)) + 8));

              }
            } else {
              ppu->spriteLatch.bitPlaneHi = 0;
            }


          } else {
                        
            if(getBit(ppu->spriteLatch.tileNumber, 0) == 1){
              patternTableOffset = 0x1000;

            } else {
              patternTableOffset = 0;
            }

            if((ppu->spriteLatch.yCoordinate <= (ppu->scanLine & 0xff)) && ((ppu->spriteLatch.yCoordinate + 16) >= (ppu->scanLine & 0xff))){
              uint16_t tileNumberTemp;
              tileIndex = (ppu->scanLine & 0xff) - ppu->spriteLatch.yCoordinate;


              // if sprite is vertically mirrored or not
              if(getBit(ppu->spriteLatch.attributeData, 7) == 0){

                if(tileIndex <= 7){

                  tileNumberTemp = ppu->spriteLatch.tileNumber & 0xfe;
      
                } else {

                  // minus 8 because we are now > 7, so we need to account for this offset and minus 8 to get the correct offset into memory
                  tileIndex -= 8;
                  tileNumberTemp = (ppu->spriteLatch.tileNumber & 0xfe) + 1;
                }
                ppu->spriteLatch.bitPlaneHi = readPpuBus(ppu, patternTableOffset + (tileNumberTemp << 4) + (tileIndex + 8));
              } else {
                if(tileIndex <= 7){
                  tileNumberTemp = (ppu->spriteLatch.tileNumber & 0xfe) + 1;
                } else {
                  tileIndex -= 8;
                  tileNumberTemp = (ppu->spriteLatch.tileNumber & 0xfe);
                }
              

                ppu->spriteLatch.bitPlaneHi = readPpuBus(ppu, patternTableOffset + (tileNumberTemp << 4) + (7 - tileIndex) + 8);
              }
            } else {
              ppu->spriteLatch.bitPlaneHi = 0;

            }

          }

          if(getBit(ppu->spriteLatch.attributeData, 6) != 0){    

            // mirror the bits if necessary
            uint8_t bitPlaneTemp = 0;
            for(int i = 0; i < 8; ++i){
              if(getBitFromLeft(ppu->spriteLatch.bitPlaneHi, i) != 0){
                bitPlaneTemp = setBit(bitPlaneTemp, i);
              } else {
                bitPlaneTemp = clearBit(bitPlaneTemp, i);
              }
            }
            ppu->spriteLatch.bitPlaneHi = bitPlaneTemp;
          }


        } else if(ppu->dotx % 8 == 0){

          // copy into sprite shifters
          // use >> 2 instead of / 4
          ppu->spriteShifters[ppu->spriteLatchCounter >> 2].xCoordinate = ppu->spriteLatch.xCoordinate;
          ppu->spriteShifters[ppu->spriteLatchCounter >> 2].bitPlaneLo = ppu->spriteLatch.bitPlaneLo;
          ppu->spriteShifters[ppu->spriteLatchCounter >> 2].bitPlaneHi = ppu->spriteLatch.bitPlaneHi;
          ppu->spriteShifters[ppu->spriteLatchCounter >> 2].attributeData = (ppu->spriteLatch.attributeData & 0b11);
          ppu->spriteShifters[ppu->spriteLatchCounter >> 2].bgPriorityFlag = ((ppu->spriteLatch.attributeData & 0b100000) >> 5);
          



          ppu->spriteLatchCounter += 4;
          
          
        }


      }

    }

}


#define TICKPPUDEBUGLOG 0


// tickPpu()
//     ticks the ppu forward one cycle (1 PPU cycle produces 1 dot between scanlines 0-239 and dots 1-256)
void tickPpu(Bus* bus){
  uint16_t tempV;

  // 4 bit value, with 2 lsb containing the pixel info and the upper 2 msb containing the attribute data
  uint8_t finalBackgroundPixel = 0;

  // 4 bit value, with 2 lsb containing the pixel info and the upper 2 msb containing the attribute data
  uint8_t finalSpritePixel = 0;
  uint8_t spritePriority = 0;



  #if TICKPPUDEBUGLOG == 1
  if(bus->ppu->dotx == 0){
    printf("scanline %d \n", bus->ppu->scanLine);
  }

  printf("dotx %d \n", bus->ppu->dotx);
  #endif


  if(bus->ppu->dotx == 0 && bus->ppu->scanLine == 0){

    bus->ppu->frameRendering.frame_start = SDL_GetPerformanceCounter();
  }

  // excludes the fineY component so that V can be used in our equations
  fillTempV(&tempV, bus->ppu->vregister.vcomp);
  
  //  correct order is OUTPUT -> SHIFT -> FETCH

  // ***** background output process ******
  if(getBit(bus->ppu->mask, 3) != 0){
    finalBackgroundPixel = backgroundOutputProcess(bus->ppu);
  }
    
  // ***** sprite output process ******* 
  if(getBit(bus->ppu->mask, 4) != 0){
    if(bus->ppu->scanLine >= 0 && bus->ppu->scanLine <= 239 && bus->ppu->dotx >= 1 && bus->ppu->dotx <= 256){

      finalSpritePixel = spriteOutputProcess(bus->ppu, finalBackgroundPixel, &spritePriority);

    } else {
      finalSpritePixel = 0;
    }

  }
  // ****** priority mux *******
  if(bus->ppu->scanLine >= 0 && bus->ppu->scanLine <= 239 && bus->ppu->dotx >= 1 && bus->ppu->dotx <= 256){
    
    priorityMux(bus->ppu, finalSpritePixel, finalBackgroundPixel, spritePriority);
  }


  // ********* shift the shift registers *********
  if(getBit(bus->ppu->mask, 3) != 0){
    if((bus->ppu->scanLine >= 0 && bus->ppu->scanLine <= 239) || (bus->ppu->scanLine == 261)){

      if((bus->ppu->dotx >= 1 && bus->ppu->dotx <= 256) || (bus->ppu->dotx >= 321 && bus->ppu->dotx <= 336)){
        #if TICKPPUDEBUGLOG == 1
        printf("\t shifting the shift registers \n");
        #endif
        shiftShiftRegisters(bus->ppu);
      }

    }
  }



    // ******** fetch data ******** 
 if(getBit(bus->ppu->mask, 3) != 0){
   if((bus->ppu->scanLine >= 0 && bus->ppu->scanLine <= 239) || (bus->ppu->scanLine == 261)){
      if((bus->ppu->dotx >= 1 && bus->ppu->dotx <= 256) || (bus->ppu->dotx >= 321 && bus->ppu->dotx <= 336)){

        fetchAndFillLatches(bus->ppu, tempV);

      }
    }
 }

  // ***** deal with v register ******
  if(getBit(bus->ppu->mask, 3) != 0 || getBit(bus->ppu->mask, 4) != 0){
    vRegisterLogic(bus->ppu);
  }
    
 


  // we reset this variable here due to an edge case from some games restarting rendering
  // after dotx 257 and trying to perform sprite evaluation and this counter
  // not being reset properly, thus causing memory corruption. 
  if(bus->ppu->dotx == 257){
    bus->ppu->spriteLatchCounter = 0;
  }
  // ****** Sprite Process ******
  // (this is all done inpreparation to output for the next scanline)
  if(getBit(bus->ppu->mask, 4) != 0){

    spriteEvaluationAndProcess(bus->ppu);

  }
  
  if(bus->ppu->scanLine == 261 && bus->ppu->dotx == 1){
      // clear vblank flag on scanline 261, dot 1
      bus->ppu->status = clearBit(bus->ppu->status, 7);
      #if TICKPPUDEBUGLOG == 1
     printf("clearing vblank flag \n");
     #endif
      // clear sprite zero flag on scanline 261, dot 1
      bus->ppu->status = clearBit(bus->ppu->status, 6);
      //printf("clearing sprite zero \n");
 
  } else if((bus->ppu->scanLine == 241) && (bus->ppu->dotx == 1)){

      // set vblank flag on scanline 241, dot 1
      bus->ppu->status = setBit(bus->ppu->status, 7);
      #if TICKPPUDEBUGLOG == 1
      printf("setting vblank flag \n");
      #endif
     if(getBit(bus->ppu->ctrl, 7) == 0b10000000){
      triggerNmi(bus->cpu);
     }  

  }

  if(bus->ppu->dotx == 340 && bus->ppu->scanLine == 261){


    drawFrameBuffer(bus);


  } 

  if(bus->ppu->dotx == 340){
    bus->ppu->dotx = 0;

    if(bus->ppu->scanLine == 261){
      bus->ppu->scanLine = 0;
    } else {
      bus->ppu->scanLine++;
    }

  } else {
    bus->ppu->dotx++;

  }
  
    

}




// incrementY()
// increments Y component of the v register
void incrementY(PPU* ppu){
  if(ppu->vregister.vcomp.fineY < 7){
    ppu->vregister.vcomp.fineY++;
  } else {
    ppu->vregister.vcomp.fineY = 0;
    if(ppu->vregister.vcomp.courseY == 29){
      ppu->vregister.vcomp.courseY = 0;

      // toggle Y nametable select bit
      ppu->vregister.vcomp.nameTableSelect = (getBit(ppu->vregister.vcomp.nameTableSelect, 1) ^ 0b10) | getBit(ppu->vregister.vcomp.nameTableSelect, 0);
  
    } else if(ppu->vregister.vcomp.courseY == 31){
      ppu->vregister.vcomp.courseY = 0;

    } else {
      ppu->vregister.vcomp.courseY++;

    }

  }
  


}

// fillTempV()
//   populates an unsigned integer with the individual components of a VComponent
//   into their respective bit positions
//  output:
//    tempV2 - unsigned integer to fill
//  input:
//    vreg - a struct VComponent to source the data from 
void fillTempV(uint16_t *tempV, struct VComponent vcomp){
    *tempV = 0;
    *tempV = vcomp.courseX;
    *tempV = *tempV | (((uint16_t)vcomp.courseY) << 5);
    *tempV = *tempV | (((uint32_t) vcomp.nameTableSelect) << 10);
}



void incrementCourseX(PPU* ppu){
    if(ppu->vregister.vcomp.courseX == 31){
      ppu->vregister.vcomp.courseX = 0;
      ppu->vregister.vcomp.nameTableSelect = getBit(ppu->vregister.vcomp.nameTableSelect, 1) | (getBit(ppu->vregister.vcomp.nameTableSelect, 0) ^ 0b01);

    } else {
      ppu->vregister.vcomp.courseX++;
    }
  
}





// findAndReturnAttributeByte()
//   finds which quadrant the beam resides in, then returns the appropriate 2-bit attribute data for the corresponding
//   quadrant
// inputs
//   x - x coordinate
//   y - y coordinate
//   attributeTableByte - attributeTabledata to select from
// outputs
//   uint8_t - 2 bit attribute data for corresponding quadrant
/*
 * 

     0  |  1
    ----------
     2  |  3


 */

uint8_t findAndReturnAttributeByte(uint16_t tempV, uint8_t attributeTableByte){

  int quadrant;

  uint8_t courseX = tempV & 0x1f;
  uint8_t courseY = ((tempV & 0x3e0) >> 5);

  if((courseX & 0b10) == 0){
    if((courseY & 0b10) == 0){
      quadrant = 0;

    } else if((courseY & 0b10) == 0b10) {
       quadrant = 2;

    }

  } else if((courseX & 0b10) == 0b10){
    if((courseY & 0b10) == 0){
      quadrant = 1;

    } else if((courseY & 0b10) == 0b10) {
       quadrant = 3;

    }

  }
  
  uint8_t atb = attributeTableByte;
  if(quadrant == 0){
    atb = atb & 0b11;
  } else if (quadrant == 1){
    atb = atb & 0b1100;
    atb = atb >> 2;
  } else if(quadrant == 2){
    atb = atb & 0b110000;
    atb = atb >> 4;
  } else if(quadrant == 3){
    atb = atb & 0b11000000;
    atb = atb >> 6;
  }


  return atb;

  
}