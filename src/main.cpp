#include <GyverOLED.h>
#include <charMap.h>
#include <icons_7x7.h>
#include <icons_8x8.h>
#include "timer.hpp"
#include "bird_character.hpp"

/* TODO:
 * - make x coord scalable as y coord
 * -
 */

#define BTN_PIN 2
#define OLED_CS 10
#define OLED_RST 9
#define OLED_DC 8

const unsigned char tree_1 [] PROGMEM = {
	0x02, 0x04, 0x3f, 0x10, 0x08
};

/* Input button flags */
#define JUMP_BUTTON_FLAG 0x01

/* Game time periods */
#define GAME_TICK_PERIOD 10
#define DISPLAY_REFRESH_PERIOD 33
#define USER_INPUT_CHECK_PERIOD 10

/* Game rule constants */
#define MAX_HEIGHT (16U)
#define MIN_HEIGHT (61U)
#define JUMP_ACCEL (1U << 8)
#define GRAVITY_ACCEL (1U << 4)

GyverOLED<SSD1306_128x64, OLED_BUFFER, OLED_SPI, OLED_CS, OLED_DC, OLED_RST> oled;

Timer gameTimer(GAME_TICK_PERIOD);
Timer userInputTimer(USER_INPUT_CHECK_PERIOD);

Bird bird;

long gameScore = 0;
uint8_t inputFlags = 0;

void moveBird(uint8_t input)
{
  int posY = bird.getPosY();

  if (inputFlags & JUMP_BUTTON_FLAG) {
    bird.setVelY(-(JUMP_ACCEL));
  }

  posY = posY + bird.getVelY();
  bird.setVelY(bird.getVelY() + GRAVITY_ACCEL);

  if (posY < (MAX_HEIGHT << 8)) {
    posY = (MAX_HEIGHT << 8);
    bird.setVelY(0);
  }
  if (posY > ((MIN_HEIGHT - bird.getSizeY()) << 8)) {
    posY = ((MIN_HEIGHT - bird.getSizeY()) << 8);
    bird.setVelY(0);
  }
  bird.setPosY(posY);
}

void drawEnvironment()
{
  static uint8_t state = 0;
  oled.drawBitmap(100 - state, MIN_HEIGHT - 6, tree_1, 5, 6);
  oled.drawBitmap(100 - 64 - state, MIN_HEIGHT - 6, tree_1, 5, 6);
  oled.drawBitmap(100 + 64 - state, MIN_HEIGHT - 6, tree_1, 5, 6);
  oled.dot(10 - state, MIN_HEIGHT + 1);
  oled.dot(12 - state, MIN_HEIGHT + 2);
  oled.dot(25 - state, MIN_HEIGHT + 1);
  oled.dot(24 - state, MIN_HEIGHT + 2);  
  oled.dot(40 - state, MIN_HEIGHT + 1);
  oled.dot(41 - state, MIN_HEIGHT + 2);
  oled.dot(55 - state, MIN_HEIGHT + 1);
  oled.dot(55 - state, MIN_HEIGHT + 2);
  oled.dot(64 + 10 - state, MIN_HEIGHT + 1);
  oled.dot(64 + 12 - state, MIN_HEIGHT + 2);
  oled.dot(64 + 25 - state, MIN_HEIGHT + 1);
  oled.dot(64 + 24 - state, MIN_HEIGHT + 2);  
  oled.dot(64 + 40 - state, MIN_HEIGHT + 1);
  oled.dot(64 + 41 - state, MIN_HEIGHT + 2);
  oled.dot(64 + 55 - state, MIN_HEIGHT + 1);
  oled.dot(64 + 55 - state, MIN_HEIGHT + 2);
  oled.dot(128 + 10 - state, MIN_HEIGHT + 1);
  oled.dot(128 + 12 - state, MIN_HEIGHT + 2);
  oled.dot(128 + 25 - state, MIN_HEIGHT + 1);
  oled.dot(128 + 24 - state, MIN_HEIGHT + 2);  
  oled.dot(128 + 40 - state, MIN_HEIGHT + 1);
  oled.dot(128 + 41 - state, MIN_HEIGHT + 2);
  oled.dot(128 + 55 - state, MIN_HEIGHT + 1);
  oled.dot(128 + 55 - state, MIN_HEIGHT + 2);
  state = state > 63 ? 0 : state + 1;
}

void drawScoreboard()
{
  oled.setCursor(2, 0);
  oled.print("Score: ");
  oled.print(gameScore >> 4);
}

void setup() {
  Serial.begin(115200);
  SPI.begin();
  pinMode(BTN_PIN, INPUT_PULLUP);

  oled.init();
  oled.clear();   // очистить дисплей (или буфер)
  oled.update();  // обновить. Только для режима с буфером! OLED_BUFFER

  bird.setPosXY(4 << 8, 45 << 8);
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
      }
    }
  }

  if (gameTimer.check()) {
    gameTimer.reset();

    gameScore += 1;
    moveBird(inputFlags);

    inputFlags = 0;
  }

  static long displayTimer = millis();
  if (millis() - displayTimer >= DISPLAY_REFRESH_PERIOD) {
    displayTimer = millis();

    oled.clear();
    oled.fastLineH(MAX_HEIGHT - 3, 0, 127);
    oled.fastLineH(MIN_HEIGHT, 0, 127);
    drawScoreboard();
    drawEnvironment();
    oled.drawBitmap(bird.getPosX() >> 8, bird.getPosY() >> 8, (uint8_t *)bird.getBitmap(), bird.getSizeX(), bird.getSizeY(), BITMAP_NORMAL, BUF_ADD);

    oled.update();
  }
}
