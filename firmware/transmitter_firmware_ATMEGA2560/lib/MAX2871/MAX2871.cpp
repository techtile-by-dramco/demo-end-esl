#include "Arduino.h"
#include "MAX2871.h"

// composition of MAX2871 Registers
// Register 0
unsigned long INT = 0x0; // Enables fractional-N mode
unsigned long NDIV;      // Integer part from N-Divider
unsigned long FRAC;
unsigned long ADDR0 = 0x0;

// Register 1
unsigned long CPL = 0x3;       // Charge pump liniarity 30%
unsigned long CPT = 0x00;      // Charge pump test mode = normal mode
unsigned long PHASE = 0x1;     // Phase Value (recomened)
unsigned long MODULUS = 0xFA0; // 4000 for max resolution
unsigned long ADDR1 = 0x1;

// Register 2
unsigned long LDS = 0x1;
// 1 if fPFD > 32 MHz
unsigned long SDN = 0x0;
// noise mode = Low-noise mode
unsigned long MUX = 0x4;
// MUX pin configuration = serial out MSB-bit in register 5
unsigned long DBR = 0x0;
// reference doubler is disabled
unsigned long RDIV2 = 0x0;
// reference divide-by-2 is disabled
unsigned long RCNT = 0x0;
// reference divide Value is unused
unsigned long REG4DB = 0x0;
// double buffer mode disabled
unsigned long CP = 0x00;
// charge pump current = 0.32 mA (1.36/RSET * (1 + CP[3:0]) RSET = 5k1)
unsigned long LDF = 0x0;
// lock dtect function = Frac-N lock detect
unsigned long LDP = 0x0;
// lock detect precision = 10ns
unsigned long PDP = 0x1;
// phase detector polarity set poitive
unsigned long SHDN = 0x0;
// power down mode = normal mode
unsigned long TRI = 0x0;
// charge pump output high-impedance mode disabled
unsigned long RST = 0x0;
// counter reset mode = normal operation
unsigned long ADDR2 = 0x2;

// Register 3
unsigned long VCO_MS = 0x0;
// VCO maual selction: unused
unsigned long VAS_SHDN = 0x0;
// VAS enabled
unsigned long VAS_TEMP = 0x1;
// VAS temperature compensation enabled
unsigned long CSM = 0x0;
// Cycle slip mode disabled
unsigned long MUTEDEL = 0x0;
// mute delay mode disabled
unsigned long CDM = 0x1;
// Fast-lock mode enabled
unsigned long CDIV = 0x0;
// clock divider value unused
unsigned long ADDR3 = 0x3;

// Register 4
unsigned long RES = 0x3;
// Reserved
unsigned long SDLDO = 0x0;
// LDO endabled
unsigned long SDDIV = 0x0;
// VCO Divider enabled
unsigned long SDREF = 0x0;
// Reference input enabled
unsigned long FB = 0x1;
// VCO to N counter mode is NOT divided
unsigned long DIVA;
unsigned long BS = 0x30FF;
// shoud be choosen so that fPFD/BS = 50kH or less
unsigned long SDVCO = 0x0;
// VCO enabled
unsigned long MTLD = 0x0;
// RFOUT Mute until Lock detet mode disabled
unsigned long BDIV = 0x0;
// RFOUTB is divided (so it's the same as RFOUTA)
unsigned long RFB_EN = 0x1;
// RFOUTB enabled
unsigned long BPWR = 0x3;
// RFOUTB = 5 dBm
unsigned long RFA_EN = 0x1;
// RFOUTA enabled
unsigned long APWR = 0x3;
// RFOUTA = 5dBm
unsigned long ADDR4 = 0x4;

// Register 5
unsigned long VAS_DLY = 0x3;
// 0x0 if VAS_TEMP = 0, 0x3 if VAS_TEMP = 1
unsigned long SDPLL = 0x0;
// PLL enabled
unsigned long F01 = 0x1;
// if F = 0 then int
unsigned long LD = 0x3;
// Lock-Detect pin function = HIGH
unsigned long MUX_MSB = 0x01;
// MSB of MUX
unsigned long ADCS = 0x0;
// ADC normal operation (ADC isn't used)
unsigned long ADCM = 0x0;
// ADC disabled
unsigned long ADDR5 = 0x5;

unsigned long long FreqOUT = 50000000;
unsigned long long FreqOUTold = 0;
unsigned long long FMIN = 23499999;
unsigned long long FMAX = 6000000001;

unsigned long composedRegisterValue;

MAX2871::MAX2871(){

}


void MAX2871::init(){

  pinMode(CLOCKPIN, OUTPUT);
  pinMode(DATAPIN, OUTPUT);
  pinMode(LE, OUTPUT);
  pinMode(CE, OUTPUT);

  pinMode(MUXPIN, INPUT);
  pinMode(RFOUT_EN, OUTPUT);
  pinMode(LD, INPUT);

  digitalWrite(CLOCKPIN, LOW);
  digitalWrite(DATAPIN, LOW);
  digitalWrite(LE, HIGH);
  digitalWrite(CE, HIGH);

  digitalWrite(RFOUT_EN, HIGH);
}


void MAX2871::setFreq(double freq_mhz){


  uint8_t _adiv = 4;
  uint8_t _mod = 250;
  uint8_t _ref_freq = 100;


  // double freq_mhz = 980.5;

  double frac_vco_freq = (double)freq_mhz * _adiv / _ref_freq;

  uint32_t INT = (uint32_t)frac_vco_freq;
  double frac_part = frac_vco_freq - INT;


  reg0 = 0x00000000 | 0 << 31 | INT << 15 | (uint32_t)(frac_part * _mod) << 3 | ADDR0;

  reg1 = 0x280107D1; 
  reg2 = 0x92005F42;
  reg3 = 0x00001F23;
  reg4 = 0x63AFF1C4; //3 dBm
  // reg4 = 0x63AFF104; // -5 dBm
  reg5 = 0x00440005;

  updateAllRegs(reg0, reg1, reg2, reg3, reg4, reg5);
  
}

void MAX2871::controlOutput(uint8_t state) {

    reg4 &= ~(1 << 8);        // clear bit 7
    reg4 |= (state << 8);     // zet nieuwe waarde

    WriteMAX2871(reg4);
}

void MAX2871::updateAllRegs(uint32_t reg0, uint32_t reg1, uint32_t reg2, uint32_t reg3, uint32_t reg4, uint32_t reg5){
  WriteMAX2871(reg5);
  WriteMAX2871(reg4);
  WriteMAX2871(reg3);
  WriteMAX2871(reg2);
  WriteMAX2871(reg1);
  WriteMAX2871(reg0);
}

// /////////////////////////////////////////////////////////////////////
// SUBROUTINES FOR THE MAX2871.
// /////////////////////////////////////////////////////////////////////

void MAX2871::WriteMAX2871(unsigned long data)
// Writes 32 Bit value to register of MAX2871
{
  digitalWrite(LE, LOW);

  shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, ((data & 0xFF000000) >> 24));
  shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, ((data & 0x00FF0000) >> 16));
  shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, ((data & 0x0000FF00) >> 8));
  shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, (data & 0x000000FF));

  digitalWrite(LE, HIGH);
  // delay(50);
}

void MAX2871::readMAX2871(){
  digitalWrite(LE, LOW);

  shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, 0x00);
  shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, 0x00);
  shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, 0x00);
  shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, 0b110);

  digitalWrite(LE, HIGH);

  digitalWrite(LE, LOW);

  shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, 0x00);
  shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, 0x00);
  shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, 0x00);
  shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, 0x00); 
  shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, 0x00); 

  digitalWrite(LE, HIGH);

}

void MAX2871::CalculateRegisterValues() // calculates values of NDIV, FRAC & DIVA
{
  double rest;

  if (FreqOUT >= 3000000000)
  {
    DIVA = 0;
    NDIV = FreqOUT / 100000000;
    rest = FreqOUT % 100000000;
    FRAC = rest / 100000000.0 * 4000.0;
  }
  else if ((FreqOUT < 3000000000) && (FreqOUT >= 1500000000))
  {
    DIVA = 1;
    NDIV = FreqOUT * 2 / 100000000;
    rest = FreqOUT * 2 % 100000000;
    FRAC = rest / 100000000.0 * 4000.0;
  }
  else if ((FreqOUT < 1500000000) && (FreqOUT >= 750000000))
  {
    DIVA = 2;
    NDIV = FreqOUT * 4 / 100000000;
    rest = FreqOUT * 4 % 100000000;
    FRAC = rest / 100000000.0 * 4000.0;
  }
  else if ((FreqOUT < 750000000) && (FreqOUT >= 375000000))
  {
    DIVA = 3;
    NDIV = FreqOUT * 8 / 100000000;
    rest = FreqOUT * 8 % 100000000;
    FRAC = rest / 100000000.0 * 4000.0;
  }
  else if ((FreqOUT < 375000000) && (FreqOUT >= 187500000))
  {
    DIVA = 4;
    NDIV = FreqOUT * 16 / 100000000;
    rest = FreqOUT * 16 % 100000000;
    FRAC = rest / 100000000.0 * 4000.0;
  }
  else if ((FreqOUT < 187500000) && (FreqOUT >= 93750000))
  {
    DIVA = 5;
    NDIV = FreqOUT * 32 / 100000000;
    rest = FreqOUT * 32 % 100000000;
    FRAC = rest / 100000000.0 * 4000.0;
  }
  else if ((FreqOUT < 93750000) && (FreqOUT >= 46875000))
  {
    DIVA = 6;
    NDIV = FreqOUT * 64 / 100000000;
    rest = FreqOUT * 64 % 100000000;
    FRAC = rest / 100000000.0 * 4000.0;
  }
  else
  {
    DIVA = 7;
    NDIV = FreqOUT * 128 / 100000000;
    rest = FreqOUT * 128 % 100000000;
    FRAC = rest / 100000000.0 * 4000.0;
  }

  Serial.println(NDIV);
  Serial.println(FRAC);
}

void MAX2871::ProgramMAX2871() // compose register value of register 0 and 4
{
  CalculateRegisterValues();

  composedRegisterValue = INT << 31 | NDIV << 15 | FRAC << 3 | ADDR0;

  WriteMAX2871(0x804B0000);
  Serial.println(composedRegisterValue, HEX);

  composedRegisterValue = RES << 29 | SDLDO << 28 | SDDIV << 27 | SDREF << 26 | FB << 23 | DIVA << 20 | BS << 12 | SDVCO << 11 | MTLD << 10 | BDIV << 9 | RFB_EN << 8 | BPWR << 6 | RFA_EN << 5 | APWR << 3 | ADDR4;

  WriteMAX2871(0x608C8024);
  Serial.println(composedRegisterValue, HEX);
}


// /////////////////////////////////////////////////////////////////////
// END OF FILE.
// /////////////////////////////////////////////////////////////////////
