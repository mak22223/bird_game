#include <GyverOLED.h>
#include <charMap.h>
#include <icons_7x7.h>
#include <icons_8x8.h>
#include "timer.hpp"
#include "bird_character.hpp"

#define BTN_PIN 2
#define OLED_CS 10
#define OLED_RST 9
#define OLED_DC 8

/* Input button flags */
#define JUMP_BUTTON_FLAG 0x01

/* Game time periods */
#define GAME_TICK_PERIOD 10
#define DISPLAY_REFRESH_PERIOD 33
#define USER_INPUT_CHECK_PERIOD 10

/* Game rule constants */
#define MAX_HEIGHT (16U << 8)
#define MIN_HEIGHT (50U << 8)
#define JUMP_ACCEL (2U << 8)
#define GRAVITY_ACCEL (2U << 4)

GyverOLED<SSD1306_128x64, OLED_BUFFER, OLED_SPI, OLED_CS, OLED_DC, OLED_RST> oled;

Timer gameTimer(GAME_TICK_PERIOD);
Timer userInputTimer(USER_INPUT_CHECK_PERIOD);

Bird bird;

uint8_t inputFlags = 0;

void moveBird(uint8_t input)
{
  int posY = bird.getPosY();

  if (inputFlags & JUMP_BUTTON_FLAG) {
    bird.setVelY(-(JUMP_ACCEL));
  }

  posY = posY + bird.getVelY();
  bird.setVelY(bird.getVelY() + GRAVITY_ACCEL);

  if (posY < MAX_HEIGHT) {
    posY = MAX_HEIGHT;
    bird.setVelY(0);
  }
  if (posY > MIN_HEIGHT) {
    posY = MIN_HEIGHT;
    bird.setVelY(0);
  }
  bird.setPosY(posY);
}

void drawEnvironment()
{
  // static uint8_t state = 0;
  // oled.dot(10 - state, (MIN_HEIGHT >> 8))
}

void setup() {
  Serial.begin(115200);
  SPI.begin();
  pinMode(BTN_PIN, INPUT_PULLUP);

  oled.init();
  oled.clear();   // очистить дисплей (или буфер)
  oled.update();  // обновить. Только для режима с буфером! OLED_BUFFER

  bird.setPosXY(4, 45 << 8);

  // oled.home();            // курсор в 0,0
  // oled.print("Hello!");   // печатай что угодно: числа, строки, float, как Serial!
  // oled.update();
  // delay(5000);

  // oled.drawBitmap(48, 16, bitmap_bird, 16, 13, BITMAP_NORMAL, BUF_ADD);
  //oled.drawBitmap(90, 16, bitmap_32x32, 32, 32);  // по умолч. нормал и BUF_ADD
  // x, y, имя, ширина, высота, BITMAP_NORMAL(0)/BITMAP_INVERT(1), BUF_ADD/BUF_SUBTRACT/BUF_REPLACE
  
  // oled.update();
}

void loop() {
  if (userInputTimer.check()) {
    static bool lastBtnState = HIGH;

    userInputTimer.reset();

    bool userBtnState = digitalRead(BTN_PIN);
    if (lastBtnState != userBtnState) {
      lastBtnState = userBtnState;
      if (userBtnState == LOW) {
        inputFlags |= JUMP_BUTTON_FLAG;
        Serial.println("Btn pressed");
      }
    }
  }

  if (gameTimer.check()) {
    gameTimer.reset();

    moveBird(inputFlags);

    inputFlags = 0;

    // static bool dir = true;
    // if (bird.getPosY() == 16 || bird.getPosY() == 45) {
    //   dir = !dir;
    // }
    // dir ? bird.setPosY(bird.getPosY() + 1) : bird.setPosY(bird.getPosY() - 1);
  }

  static long displayTimer = millis();
  if (millis() - displayTimer >= DISPLAY_REFRESH_PERIOD) {
    displayTimer = millis();

    oled.clear();
    oled.fastLineH((MIN_HEIGHT >> 8) + 11, 0, 127);
    // drawEnvironment();
    oled.drawBitmap(bird.getPosX(), bird.getPosY() >> 8, (uint8_t *)bird.getBitmap(), bird.getSizeX(), bird.getSizeY(), BITMAP_NORMAL, BUF_ADD);
    // static bool flag = false;
    // if (flag) {
    //   oled.drawBitmap(charPosX, charPosY, bitmap_bird, 16, 11, BITMAP_NORMAL, BUF_ADD);
    // } else {
    //   oled.rect(charPosX, charPosY, charPosX + 16, charPosY + 11);
    // }
    // flag = !flag;
    oled.update();
  }
}
