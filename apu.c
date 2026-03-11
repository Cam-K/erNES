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


#include "apu.h"

void initApu(APU* apu){
  populateLengthCounterTable(apu);
}

void resetApu(APU* apu){

  for(int i = 0; i < 4; ++i){
    apu->pulse1[i] = 0;
    apu->pulse2[i] = 0;
    apu->noise[i] = 0;
    apu->dmc[i] = 0;

  }

  apu->control = 0;
  apu->status = 0;
  apu->frameCounter = 0;


}

void populateLengthCounterTable(APU* apu){
  apu->lengthCounterTable[0] = 10;    // 00
  apu->lengthCounterTable[1] = 254;   // 01
  apu->lengthCounterTable[2] = 20;    // 02
  apu->lengthCounterTable[3] = 2;     // 03
  apu->lengthCounterTable[4] = 40;    // 04
  apu->lengthCounterTable[5] = 4;     // 05
  apu->lengthCounterTable[6] = 80;    // 06
  apu->lengthCounterTable[7] = 6;     // 07
  apu->lengthCounterTable[8] = 160;   // 08
  apu->lengthCounterTable[9] = 8;     // 09
  apu->lengthCounterTable[10] = 60;   // 0A
  apu->lengthCounterTable[11] = 10;   // 0B
  apu->lengthCounterTable[12] = 14;   // 0C
  apu->lengthCounterTable[13] = 12;   // 0D
  apu->lengthCounterTable[14] = 26;   // 0E
  apu->lengthCounterTable[15] = 14;   // 0F
  apu->lengthCounterTable[16] = 12;   // 10
  apu->lengthCounterTable[17] = 16;   // 11
  apu->lengthCounterTable[18] = 24;   // 12
  apu->lengthCounterTable[19] = 18;   // 13
  apu->lengthCounterTable[20] = 48;   // 14
  apu->lengthCounterTable[21] = 20;   // 15
  apu->lengthCounterTable[22] = 96;   // 16
  apu->lengthCounterTable[23] = 22;   // 17
  apu->lengthCounterTable[24] = 192;  // 18
  apu->lengthCounterTable[25] = 24;   // 19
  apu->lengthCounterTable[26] = 72;   // 1A
  apu->lengthCounterTable[27] = 26;   // 1B
  apu->lengthCounterTable[28] = 16;   // 1C
  apu->lengthCounterTable[29] = 28;   // 1D
  apu->lengthCounterTable[30] = 32;   // 1E
  apu->lengthCounterTable[31] = 30;   // 1F
 


}

