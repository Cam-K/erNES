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

#include <SDL2/SDL_audio.h>
#include <stdint.h>

typedef struct _CTimer{
  unsigned int timer : 11;
} ChannelTimer;

typedef struct _APU {
  

  // pulse 1 channel registers ($4000-$4003)
  uint8_t pulse1[4];
  uint8_t pulse1LengthCounter;
  ChannelTimer pulse1Timer;

    // pulse 2 channel registers ($4004-$4007)
  uint8_t pulse2[4];
  uint8_t pulse2LengthCounter;
  ChannelTimer pulse2Timer;


  // triangle channel registers ($4008-$400B)
  uint8_t triangle[4];
  uint8_t triangleLengthCounter;
  ChannelTimer triangleTimer;


  // noise channel registers ($4008-$400B)
  uint8_t noise[4];
  uint8_t noiseLengthCounter;


  // dmc channel registers ($4010-$4013)
  uint8_t dmc[4];

  uint8_t control;
  uint8_t status;
  uint8_t frameCounter;

  uint8_t lengthCounterTable[32];
  

  


} APU;

void resetApu(APU*);
void populateLengthCounterTable(APU*);