#include <Arduino.h>
#include "Adafruit_ThinkInk.h"
#include <EEPROM.h>

#define EPD_DC        A2//10
#define EPD_CS        A3//9
#define EPD_BUSY      D1//7 // can set to -1 to not use a pin (will wait a fixed delay)
#define SRAM_CS       A1//6
#define EPD_RESET     D9//8  // can set to -1 and share with microcontroller Reset!
#define EPD_SPI       &SPI // primary SPI

#define SPI1_SCK      A4
#define SPI1_MISO     A5
#define SPI1_MOSI     A6

#define KEEP_BOOST_ON D10

//  Frame number (saved in EEPROM)
uint8_t frame_number;
int eeAddress = 0;   // Location we want the data to be put.

//  Static void functions
static void fillDitherPercent(uint8_t percent);

//  EPD object
ThinkInk_213_Mono_BN display(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY, EPD_SPI);


//  Configure SYSTEM clock - NXP
extern "C" void SystemClock_Config(void){
 RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}


void setup() {

  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  pinMode(LED_BUILTIN, OUTPUT);   //  Set buildin LED as output
  pinMode(EPD_CS, OUTPUT);        //  Set SS as output

  // SPI config
  SPI.setMOSI(SPI1_MOSI);
  SPI.setMISO(SPI1_MISO);
  SPI.setSCLK(SPI1_SCK);

  //  OVERRULE - Keep BOOST converter ON
  pinMode(KEEP_BOOST_ON, OUTPUT);
  digitalWrite(KEEP_BOOST_ON, HIGH);

  Serial.println("Adafruit EPD full update test in mono");
  display.begin(THINKINK_MONO);
}

void loop() {

  Serial.println("hello");
  
  //  Keep boost on for 3V3 supply voltage during EPD update
  digitalWrite(KEEP_BOOST_ON, HIGH);

  //  Get frame number for EEPROM
  uint8_t new_frame_number;
  frame_number = EEPROM.read(eeAddress);

  //  Decide with update will be performed
  switch (frame_number){
    case 0:
        fillDitherPercent(5);
        new_frame_number = 1;
        break;

    case 1:
        fillDitherPercent(10);
        new_frame_number = 2;
        break;

    case 2:
        fillDitherPercent(25);
        new_frame_number = 3;
        break;

    case 3:
        fillDitherPercent(50);
        new_frame_number = 4;
        break;

    case 4:
        fillDitherPercent(75);
        new_frame_number = 5;
        break;

    case 5:
        fillDitherPercent(100);
        new_frame_number = 0;
        break;
    
    // Default
    default:
      break;
  }

  //  Store case number in EEPROM
  EEPROM.put(eeAddress, new_frame_number);

  //  Disable 3V3 --> DEADLOCK
  digitalWrite(KEEP_BOOST_ON, LOW);
  delay(500);
}

const int W = display.width();
const int H = display.height();

void fillDitherPercent(uint8_t percent) {
  // Clamp input to valid range
  if (percent > 100) percent = 100;

  // Invert percentage:
  // 0%  = full white
  // 100% = full black
  percent = 100 - percent;

  // Clear internal display buffer
  display.clearBuffer();

  // Error accumulator for dithering (kept across lines)
  uint16_t acc = 0;

  // Process the display line by line
  for (int y = 0; y < H; y++) {

    // One scanline bitmap (1 bit per pixel)
    uint8_t line[(W + 7) / 8];

    // Initialize line to all white pixels (1 = white)
    memset(line, 0xFF, sizeof(line));

    // Generate horizontal dithering pattern
    for (int x = 0; x < W; x++) {

      // Accumulate desired black pixel percentage
      acc += percent;

      // When threshold is reached, place a black pixel
      if (acc >= 100) {
        acc -= 100;

        // Clear the corresponding bit (0 = black)
        line[x >> 3] &= ~(1 << (7 - (x & 7)));
      }
    }

    // Draw the generated scanline to the display
    display.drawBitmap(
      0, y,     // X, Y position
      line,     // Bitmap data (1 scanline)
      W, 1,     // Width, Height
      EPD_BLACK // Draw color
    );
  }

  display.display();
}