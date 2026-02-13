
#include <SDL2/SDL_audio.h>
#include <stdint.h>

typedef struct _APU {
  

  // pulse 1 channel register ($4000-$4003)
  uint8_t pulse1_0;
  uint8_t pulse1_1;
  uint8_t pulse1_2;
  uint8_t pulse1_3;

    // pulse 2 channel registers ($4004-$4007)
  uint8_t pulse2_0;
  uint8_t pulse2_1;
  uint8_t pulse2_2;
  uint8_t pulse2_3;


  // triangle channel registers ($4008-$400B)
  uint8_t triangle_0;
  uint8_t triangle_1;
  uint8_t triangle_2;
  uint8_t triangle_3;

  // noise channel registers ($4008-$400B)
  uint8_t noise_0;
  uint8_t noise_1;
  uint8_t noise_2;
  uint8_t noise_3;

  // dmc channel registers ($4010-$4013)
  uint8_t dmc_0;
  uint8_t dmc_1;
  uint8_t dmc_2;
  uint8_t dmc_3;

  uint8_t control;
  uint8_t status;
  uint8_t frameCounter;
  

  


} APU;