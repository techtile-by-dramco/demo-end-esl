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
static void setup_esl(String price);

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
    // ESL layout with price 1.2 dollar
    case 0:
      setup_esl("$1.20");
      new_frame_number = 1;
      break;
    
    // ESL layout with price 1.4 dollar
    case 1:
      setup_esl("$1.40");
      new_frame_number = 2;
      break;
    
    // PhD Jarne
    case 2:
      display.clearBuffer();
      display.setTextSize(2);
      display.setTextColor(EPD_BLACK);
      display.setCursor((display.width() - 220) / 2, (display.height() - 24) / 4);
      display.print("PhD PUBLIC DEFENCE");
      display.setTextSize(3);
      display.setCursor((display.width() - 200) / 2, (display.height() - 10) / 2);
      display.print("JARNE");
      display.setCursor((display.width() - 200) / 2, (display.height() - 35) );
      display.print("VAN MULDERS");
      display.display();
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


void setup_esl(String price){
    display.clearBuffer();
    display.setTextSize(1); // Smaller text size for a compact look
    display.setTextColor(EPD_BLACK);

    // Draw a border to mimic an ESL frame
    display.drawRect(0, 0, display.width(), display.height(), EPD_BLACK);

    // Set product information
    display.setCursor(20, 10);
    display.setTextSize(3);
    display.print("PRODUCT NAME");
    display.setCursor(0, 30);
    display.fillRect(10, 38, display.width()-20, 2, EPD_BLACK);
    display.setCursor(140, 68);
    display.setTextSize(3);
    display.print(price);
    display.drawRect(10, 55, display.width()/2-10, display.height()/2-5, EPD_BLACK);
    display.setCursor(28, 75);
    display.setTextSize(2);
    display.print("PICTURE");

    // Display
    display.display();
}