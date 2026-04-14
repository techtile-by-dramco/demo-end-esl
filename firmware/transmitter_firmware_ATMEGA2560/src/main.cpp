#include <Arduino.h>

// Include the library
#include <TM1637Display.h>
#include "MAX2871.h"
#include <avr/interrupt.h>
#include <EEPROM.h>

// Define the connections pins
#define CLK 50
#define DIO 51



// Create a display object of type TM1637Display
TM1637Display display = TM1637Display(CLK, DIO);
MAX2871 max;

// Create an array that turns all segments ON
const uint8_t allON[] = { 0xff, 0xff, 0xff, 0xff };

// Create an array that turns all segments OFF
const uint8_t allOFF[] = { 0x00, 0x00, 0x00, 0x00 };

// Create an array that sets individual segments per digit to display the word "dOnE"
const uint8_t done[] = {
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,          // d
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,  // O
  SEG_C | SEG_E | SEG_G,                          // n
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G           // E
};

// Create degree celsius symbol
const uint8_t celsius[] = {
  SEG_A | SEG_B | SEG_F | SEG_G,  // Degree symbol
  SEG_A | SEG_D | SEG_E | SEG_F   // C
};


void setup_interrupt() {
    cli(); // interrupts uit

    TCCR1A = 0;
    TCCR1B = 0;

    // CTC mode
    TCCR1B |= (1 << WGM12);

    // Prescaler 64
    TCCR1B |= (1 << CS11) | (1 << CS10);

    // 16 MHz / 64 = 250 kHz → 1 ms = 250 ticks
    OCR1A = 249;

    // Enable compare interrupt
    TIMSK1 |= (1 << OCIE1A);

    sei(); // interrupts aan
}

uint16_t update_counter = 0;

bool update = false;

bool freq_update = false;


uint8_t state = 1;

ISR(TIMER1_COMPA_vect) {
    if(update_counter >= 1000) { // Update every 500 ms
        update = true;
        update_counter = 0;
    } else {
        update_counter++;
    }
}


uint16_t default_frequency = 915;

void setup() {

  Serial.begin(115200);

  pinMode(2, INPUT);
  pinMode(3, INPUT);

  // attachInterrupt(digitalPinToInterrupt(2), ISR_button1, RISING);
  // attachInterrupt(digitalPinToInterrupt(3), ISR_button2, RISING);

  setup_interrupt();

  // Set the brightness to 5 (0=dimmest 7=brightest)
  display.setBrightness(7);

  pinMode(5, OUTPUT);
  digitalWrite(5, LOW);

  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);

  // Get default frequency from EEPROM
  EEPROM.get(0, default_frequency);

  // Initialize the MAX2871
  max.init();

  delay(100);

  //  Set the frequency
  max.setFreq(default_frequency);

  //  Disable PLL output
  max.controlOutput(0);
}


void updateEEPROM() {
  EEPROM.put(0, default_frequency);
}


void loop() {

  // bool b1 = digitalRead(2) == HIGH;
  // bool b2 = digitalRead(3) == HIGH;

  // static bool last_b1 = false;
  // static bool last_b2 = false;

  // // detect rising edge (indrukken)
  // bool b1_pressed = b1 && !last_b1;
  // bool b2_pressed = b2 && !last_b2;

  // // combinatie
  // if (b1 && b2) {
  //     state = 1;
  //     max.controlOutput(0);
  // }
  // // else if (b1 && b2 && state == 1) {
  // //     state = 0;
  // //     freq_update = true;
  // //     max.setFreq(default_frequency);
  // // }
  // else if (b1_pressed && state == 1) {
  //     state = 0;
  //     freq_update = true;
  //     max.setFreq(default_frequency);
  //     delay(100);
  // }
  // else if (b2_pressed && state == 1) {
  //     state = 0;
  //     freq_update = true;
  //     max.setFreq(default_frequency);
  //     delay(100);
  // }
  // else if (b1_pressed && state == 0) {
  //     state = 0;
  //     default_frequency += 1;
  //     freq_update = true;
  //     max.setFreq(default_frequency);
  //     updateEEPROM();
  // }
  // else if (b2_pressed && state == 0) {
  //     state = 0;
  //     default_frequency -= 1;
  //     freq_update = true;
  //     max.setFreq(default_frequency);
  //     updateEEPROM();
  // }

  // // update vorige toestand
  // last_b1 = b1;
  // last_b2 = b2;


  bool b1 = digitalRead(2) == HIGH;
  bool b2 = digitalRead(3) == HIGH;

  static bool last_b1 = false;
  static bool last_b2 = false;
  static bool last_both = false;

  // edge detectie
  bool b1_pressed = b1 && !last_b1;
  bool b2_pressed = b2 && !last_b2;
  bool both_pressed = b1 && b2;
  bool both_edge = both_pressed && !last_both;

  // combinatie (één keer!)
  if (both_edge) {
      state = 1;
      max.controlOutput(0);
  }
  else if (b1_pressed && state == 1) {
      state = 0;
      freq_update = true;
      max.setFreq(default_frequency);
  }
  else if (b2_pressed && state == 1) {
      state = 0;
      freq_update = true;
      max.setFreq(default_frequency);
  }
  else if (b1_pressed && state == 0) {
      default_frequency += 1;
      freq_update = true;
      max.setFreq(default_frequency);
      updateEEPROM();
  }
  else if (b2_pressed && state == 0) {
      default_frequency -= 1;
      freq_update = true;
      max.setFreq(default_frequency);
      updateEEPROM();
  }

  // update states
  last_b1 = b1;
  last_b2 = b2;
  last_both = both_pressed;

  if(update) {
    switch (state)
    {
    case 0:
      if(freq_update) {
        display.clear();
        display.showNumberDec(default_frequency);
        freq_update = false;
      }
      break;
    case 1:
      uint8_t dash[] = {0x40, 0x40, 0x40, 0x40}; // midden segment

      for (int i = 0; i < 4; i++) {
          uint8_t data[] = {0, 0, 0, 0};
          data[i] = 0x40;
          display.setSegments(data);
          delay(150);
      }
      break;
    default:
      break;
    }
    update = false;
  }
}