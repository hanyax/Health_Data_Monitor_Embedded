#include "structures.h"
#include "TimerOne.h"

// values
unsigned char temperatureRawBuf[8];
unsigned char bloodPressRawBuf[16];
unsigned char pulseRateRawBuf[8];
unsigned char respirationRateRawBuf[8];

unsigned char tempCorrectedBuf[8];
unsigned char bloodPressCorrectedBuf[16];
unsigned char pulseRateCorrectedBuf[8];
unsigned char respirationRateCorrectedBuf[8];

unsigned short functionSelectValue;
unsigned short measurementSelection = 0b111;

// pointer values
unsigned short* batteryState_ptr = &batteryState;

void timerInterrupt() {
  if (timer >= 19) {
    timer  = 0; 
  } else {
    timer = timer + 1;
  }
  if (sysRed) {
    if (alarmAcknowledge) {
          alarmAcknowledge = alarmAcknowledge - 1;
    }
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println(F("TFT LCD test"));

  #ifdef USE_Elegoo_SHIELD_PINOUT
    Serial.println(F("Using Elegoo 2.4\" TFT Arduino Shield Pinout"));
  #else
    Serial.println(F("Using Elegoo 2.4\" TFT Breakout Board Pinout"));
  #endif

  Serial.print("TFT size is "); Serial.print(tft.width()); Serial.print("x"); Serial.println(tft.height());

  tft.reset();

  uint16_t identifier = tft.readID();
  if(identifier == 0x9325) {
    Serial.println(F("Found ILI9325 LCD driver"));
  } else if(identifier == 0x9328) {
    Serial.println(F("Found ILI9328 LCD driver"));
  } else if(identifier == 0x4535) {
    Serial.println(F("Found LGDP4535 LCD driver"));
  }else if(identifier == 0x7575) {
    Serial.println(F("Found HX8347G LCD driver"));
  } else if(identifier == 0x9341) {
    Serial.println(F("Found ILI9341 LCD driver"));
  } else if(identifier == 0x8357) {
    Serial.println(F("Found HX8357D LCD driver"));
  } else if(identifier==0x0101) {
      identifier=0x9341;
      Serial.println(F("Found 0x9341 LCD driver"));
  }
  else if(identifier==0x1111) {
      identifier=0x9328;
      Serial.println(F("Found 0x9328 LCD driver"));
  }
  else {
    Serial.print(F("Unknown LCD driver chip: "));
    Serial.println(identifier, HEX);
    Serial.println(F("If using the Elegoo 2.8\" TFT Arduino shield, the line:"));
    Serial.println(F("  #define USE_Elegoo_SHIELD_PINOUT"));
    Serial.println(F("should appear in the library header (Elegoo_TFT.h)."));
    Serial.println(F("If using the breakout board, it should NOT be #defined!"));
    Serial.println(F("Also if using the breakout, double-check that all wiring"));
    Serial.println(F("matches the tutorial."));
    identifier=0x9328;
  }
  tft.begin(identifier);
  drawTop();
  pinMode(13, OUTPUT);
  Timer1.initialize(250000);
  Timer1.attachInterrupt(timerInterrupt, 250000);
  Serial1.begin(9600);

  // measure tcb
  mData = {temperatureRawBuf, bloodPressRawBuf, pulseRateRawBuf, respirationRateRawBuf, &measurementSelection};
  meas = {&measure, &mData};

  // compute tcb
  cData = {temperatureRawBuf, bloodPressRawBuf, pulseRateRawBuf, respirationRateRawBuf, &measurementSelection, tempCorrectedBuf,  bloodPressCorrectedBuf, pulseRateCorrectedBuf, respirationRateCorrectedBuf};
  comp = {&compute, &cData};

  // display tcb
  dData = {tempCorrectedBuf, bloodPressCorrectedBuf, pulseRateCorrectedBuf, respirationRateCorrectedBuf, batteryState_ptr};
  disp = {&displayF, &dData};

  // warning tcb
  wData = {temperatureRawBuf, bloodPressRawBuf, pulseRateRawBuf, respirationRateRawBuf, batteryState_ptr};
  warn = {&warningAlarm, &wData};

  // status tcb
  sData = {batteryState_ptr};
  stat = {&statusF, &sData};
  
  // keypad tcb
  key = {&keyPad, &dData};

  // flash tcb
  fla = {&flash, &dData};

  insert(&key);
  
  temp_rb = {1, 0, 0, {0,0,0,0,0,0,0,0}};
  pulse_rb = {1, 0, 0, {0,0,0,0,0,0,0,0}};
  resp_rb = {1, 0, 0, {0,0,0,0,0,0,0,0}};
  
}

void loop() {
  scheduler();
  flash(&dData);
  prev = cur;
}

