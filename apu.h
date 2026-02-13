
#include <SDL2/SDL_audio.h>
#include <stdint.h>

typedef struct _APU {
  

  // pulse 1 channel registers ($4000-$4003)
  uint8_t pulse1[4];

    // pulse 2 channel registers ($4004-$4007)
  uint8_t pulse2[4];


  // triangle channel registers ($4008-$400B)
  uint8_t triangle[4];


  // noise channel registers ($4008-$400B)
  uint8_t noise[4];


  // dmc channel registers ($4010-$4013)
  uint8_t dmc[4];

  uint8_t control;
  uint8_t status;
  uint8_t frameCounter;
  

  


} APU;

void initApu(APU*);