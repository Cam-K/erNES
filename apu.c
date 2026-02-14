#include "apu.h"


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


