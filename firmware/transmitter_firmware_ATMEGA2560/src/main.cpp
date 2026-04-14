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

bool freq_update = false;

// Display state: 0 = show frequency, 1 = animation
uint8_t state = 1;

uint16_t default_frequency = 915;

void setup() {

  Serial.begin(115200);

  pinMode(2, INPUT);
  pinMode(3, INPUT);

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

uint8_t animIndex = 0;
unsigned long lastAnimTime = 0;
const unsigned long animInterval = 150;

// Button lockout variables [Prevent accidental frequency changes when starting the device]
unsigned long state_change_time = 0;
const unsigned long lockout_time = 500;

// Button lockout variables [Prevent accidental frequency changes when pressing both buttons together]
unsigned long button_lock_time = 0;
bool buttons_locked = false;

int freq_backup = 0;
unsigned long change_time = 0;
bool freq_changed = false;

// Button states
static bool last_b1 = false;
static bool last_b2 = false;
static bool last_both = false;

void updateEEPROM() {
  EEPROM.put(0, default_frequency);
}


void loop() {

  // Poll button states
  bool b1 = digitalRead(2) == HIGH;
  bool b2 = digitalRead(3) == HIGH;

  // edge detectie
  bool b1_pressed = b1 && !last_b1;
  bool b2_pressed = b2 && !last_b2;
  bool both_pressed = b1 && b2;
  bool both_edge = both_pressed && !last_both;

  // Handle button presses and state changes
  if (!buttons_locked) {
    // combinatie (één keer!)
    if (both_edge) {
        // state = 1;
        // max.controlOutput(0);
      state = 1;

      if (freq_changed && (millis() - change_time < 1000)) {

          // rollback laatste wijziging
          default_frequency = freq_backup;

          max.setFreq(default_frequency);
          updateEEPROM();

          freq_changed = false;
      }

      max.controlOutput(0);

      // Lock buttons for a short period to prevent accidental changes
      buttons_locked = true;
      button_lock_time = millis();
    }
    else if (b1_pressed && state == 1) {
        state = 0;
        state_change_time = millis();
        freq_update = true;
        max.setFreq(default_frequency);
    }
    else if (b2_pressed && state == 1) {
        state = 0;
        state_change_time = millis();
        freq_update = true;
        max.setFreq(default_frequency);
    }
    else if (b1_pressed && state == 0) {
        if (millis() - state_change_time < lockout_time) return;

        freq_backup = default_frequency;

        state = 0;
        default_frequency += 1;
        freq_update = true;
        max.setFreq(default_frequency);
        updateEEPROM();

        change_time = millis();
        freq_changed = true;
    }
    else if (b2_pressed && state == 0) {
        if (millis() - state_change_time < lockout_time) return;

        freq_backup = default_frequency;
        
        state = 0;
        default_frequency -= 1;
        freq_update = true;
        max.setFreq(default_frequency);
        updateEEPROM();
        
        change_time = millis();
        freq_changed = true;
    }
  }
  else {
    // Check if lockout period has passed
    if (millis() - button_lock_time >= 500) {
        buttons_locked = false;
    }
  }

  // update states
  last_b1 = b1;
  last_b2 = b2;
  last_both = both_pressed;

  // Control display based on state
  switch (state){
    case 0:
      if (freq_update) {
        display.clear();
        display.showNumberDec(default_frequency);
        freq_update = false;
      }
      break;

    case 1:
      if (millis() - lastAnimTime >= animInterval) {
        lastAnimTime = millis();

        // Serial.println("update anim");

        uint8_t data[4] = {0, 0, 0, 0};
        data[animIndex] = 0x40;   // midden segment

        display.setSegments(data);

        animIndex++;
        if (animIndex >= 4) {
          animIndex = 0;
        }
      }
      break;
    default:
      break;
    }
}