/*
  Morse.h - Library for flashing Morse code.
  Created by David A. Mellis, November 2, 2007.
  Released into the public domain.
*/
#ifndef MAX2871_h
#define MAX2871_h

#include "Arduino.h"


// PINS MAX2871
#define CLOCKPIN 13
#define DATAPIN 11
#define LE 10
#define CE 9

#define MUXPIN 12
#define RFOUT_EN 8
#define LOCKPIN  6


class MAX2871{
  public:
    MAX2871();

    void init();

    void setFreq(double freq_mhz);

    void controlOutput(uint8_t state);

    void updateAllRegs(uint32_t reg0, uint32_t reg1, uint32_t reg2, uint32_t reg3, uint32_t reg4, uint32_t reg5);

    void WriteMAX2871(unsigned long data);
    void readMAX2871();
    void CalculateRegisterValues();
    void ProgramMAX2871();



  private:

  uint32_t reg0;
  uint32_t reg1;
  uint32_t reg2;
  uint32_t reg3;
  uint32_t reg4;
  uint32_t reg5;
};

#endif