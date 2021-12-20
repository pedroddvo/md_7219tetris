#include <SPI.h>
#include <MD_MAX72xx.h>
#include "C:\Users\Poliveira\Desktop\DEV\Arduino\md_7219 Tetris\game2.h"

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW

#define MAX_DEVICES 4
#define CS_PIN 3
#define CLK_PIN 13
#define DATA_PIN 10

#define MOVE_RIGHT_PIN 7

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);


void setup() {
  pinMode(MOVE_RIGHT_PIN, INPUT);

  mx.begin();
  Serial.begin(9600);
  mx.control(0, MAX_DEVICES-1, MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
  for (int i = 0; i < 8; i++) {
    mx.setColumn(i, 0b11111111);
    delay(100);
    mx.setColumn(i, 0b00000000);
    delay(100);
  }
  game_setup(&mx, &Serial);
}


void loop() {
  game_tick(millis());
}
