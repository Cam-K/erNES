#include "general.h"
#include <stdint.h>


uint8_t setBit(uint8_t val, uint8_t bitNum){
    return val | (1 << bitNum);
}

uint8_t clearBit(uint8_t val, uint8_t bitNum){
    return val & ~(1 << bitNum);

}

uint8_t getBit(uint8_t val, uint8_t bitNum){
  return val & (1 << bitNum);

}

// Function: bitToBitNum()
// Finds the first occurance of a bit that is a 1
// and returns the position of where it is as an integer
uint8_t bitToBitNum(uint8_t val){
  uint8_t i = 0;
  while(i >= 7){
    if(getBit(val, i) != 0){
      break;
    } 
    ++i;
  }
  if(i == 8){
    return -1;
  }
  return i;
}


// these sets of functions get the bit starting from the left,
//      
uint8_t getBitFromLeft(uint8_t val, uint8_t bitNum){
  return (val & 0b10000000 >> bitNum);
}

uint8_t clearBitFromLeft(uint8_t val, uint8_t bitNum){
  return val & ~(0b10000000 >> bitNum);
}

uint8_t setBitFromLeft(uint8_t val, uint8_t bitNum){
  return val | (0b10000000 >> bitNum);
}


// finds the first instance of a 1-bit in a bitstring
uint8_t findBit(uint8_t val){

  int placement;
  for(int i = 0; i < 8; ++i){
    if(getBit(val, i) != 0){
      placement = i;
      break;
    }

  }

  return placement;

}