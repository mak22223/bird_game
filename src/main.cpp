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

/* Hardware restrictions */
#define DISPLAY_HEIGHT 64U
#define DISPLAY_WIDTH 128U

/* Input button flags */
#define JUMP_BUTTON_FLAG 0x01

/* Game time periods */
#define GAME_TICK_PERIOD 10
#define DISPLAY_REFRESH_RATE 30
#define USER_INPUT_CHECK_PERIOD 10

/* Game rule constants */
#define MAX_BIRD_HEIGHT (16U)
#define MIN_BIRD_HEIGHT (61U)
#define MAX_OBSTACLE_COUNT (4U)
#define MAX_OBSTACLE_HEIGHT (20U)
#define MIN_OBSTACLE_HEIGHT (55U)
#define OBSTACLE_WIDTH (12U)
#define OBSTACLE_WINDOWN_HEIGHT (25U)
#define OBSTACLE_DISTANCE (50L)
#define JUMP_ACCEL (1U << 8)
#define GRAVITY_ACCEL (1U << 4)

GyverOLED<SSD1306_128x64, OLED_BUFFER, OLED_SPI, OLED_CS, OLED_DC, OLED_RST> oled;

Timer gameTimer(GAME_TICK_PERIOD);
Timer userInputTimer(USER_INPUT_CHECK_PERIOD);
Timer displayTimer(1000 / DISPLAY_REFRESH_RATE);
Timer debugTimer(1000);

Bird bird;

/* Game variables */
long gameScore = 0;
int gameSpeed = 100;
long globalPos = 250;
uint8_t inputFlags = 0;

struct Obstacle
{
  long x;
  uint8_t y;
};
uint8_t obstacleCount = 0;
uint8_t lastObstacle = 0;
Obstacle obstacles[MAX_OBSTACLE_COUNT];


void moveBird(uint8_t input)
{
  unsigned int posY = bird.getPosY();

  if (inputFlags & JUMP_BUTTON_FLAG) {
    bird.setVelY(-(JUMP_ACCEL));
  }

  posY = posY + bird.getVelY();
  bird.setVelY(bird.getVelY() + GRAVITY_ACCEL);

  if (posY < (MAX_BIRD_HEIGHT << 8)) {
    posY = (MAX_BIRD_HEIGHT << 8);
    bird.setVelY(0);
  }
  if (posY > ((MIN_BIRD_HEIGHT - bird.getSizeY()) << 8)) {
    posY = ((MIN_BIRD_HEIGHT - bird.getSizeY()) << 8);
    bird.setVelY(0);
  }
  bird.setPosY(posY);
}

void processObstacles()
{
  /* Generating obstacles */
  if (obstacleCount < MAX_OBSTACLE_COUNT) {
    if (obstacleCount == 0 || (globalPos - obstacles[lastObstacle].x >= (OBSTACLE_DISTANCE << 8))) {
      uint8_t y = random(MAX_OBSTACLE_HEIGHT, MIN_OBSTACLE_HEIGHT - OBSTACLE_WINDOWN_HEIGHT);

      ++obstacleCount;
      lastObstacle = (lastObstacle == MAX_OBSTACLE_COUNT - 1) ? 0 : lastObstacle + 1;
      obstacles[lastObstacle].x = globalPos & 0xFFFFFF00;
      obstacles[lastObstacle].y = y;
    }
  }

  /* Removing osbstacles */
  for (uint8_t i = 0; i < obstacleCount; ++i) {
    uint8_t index = i > lastObstacle ? MAX_OBSTACLE_COUNT - (i - lastObstacle) : lastObstacle - i;

    if ((globalPos - obstacles[index].x) > ((long)(DISPLAY_WIDTH + OBSTACLE_WIDTH) << 8)) {
      --obstacleCount;
    }
  }
}

bool checkCollision()
{
  return false;
}

void drawEnvironment()
{
  uint8_t state = (globalPos >> 8) & 0x3F;
  oled.drawBitmap(100 - state, MIN_BIRD_HEIGHT - 6, tree_1, 5, 6);
  oled.drawBitmap(100 - 64 - state, MIN_BIRD_HEIGHT - 6, tree_1, 5, 6);
  oled.drawBitmap(100 + 64 - state, MIN_BIRD_HEIGHT - 6, tree_1, 5, 6);
  oled.dot(10 - state, MIN_BIRD_HEIGHT + 1);
  oled.dot(12 - state, MIN_BIRD_HEIGHT + 2);
  oled.dot(25 - state, MIN_BIRD_HEIGHT + 1);
  oled.dot(24 - state, MIN_BIRD_HEIGHT + 2);  
  oled.dot(40 - state, MIN_BIRD_HEIGHT + 1);
  oled.dot(41 - state, MIN_BIRD_HEIGHT + 2);
  oled.dot(55 - state, MIN_BIRD_HEIGHT + 1);
  oled.dot(55 - state, MIN_BIRD_HEIGHT + 2);
  oled.dot(64 + 10 - state, MIN_BIRD_HEIGHT + 1);
  oled.dot(64 + 12 - state, MIN_BIRD_HEIGHT + 2);
  oled.dot(64 + 25 - state, MIN_BIRD_HEIGHT + 1);
  oled.dot(64 + 24 - state, MIN_BIRD_HEIGHT + 2);  
  oled.dot(64 + 40 - state, MIN_BIRD_HEIGHT + 1);
  oled.dot(64 + 41 - state, MIN_BIRD_HEIGHT + 2);
  oled.dot(64 + 55 - state, MIN_BIRD_HEIGHT + 1);
  oled.dot(64 + 55 - state, MIN_BIRD_HEIGHT + 2);
  oled.dot(128 + 10 - state, MIN_BIRD_HEIGHT + 1);
  oled.dot(128 + 12 - state, MIN_BIRD_HEIGHT + 2);
  oled.dot(128 + 25 - state, MIN_BIRD_HEIGHT + 1);
  oled.dot(128 + 24 - state, MIN_BIRD_HEIGHT + 2);  
  oled.dot(128 + 40 - state, MIN_BIRD_HEIGHT + 1);
  oled.dot(128 + 41 - state, MIN_BIRD_HEIGHT + 2);
  oled.dot(128 + 55 - state, MIN_BIRD_HEIGHT + 1);
  oled.dot(128 + 55 - state, MIN_BIRD_HEIGHT + 2);
}

void drawScoreboard()
{
  oled.setCursorXY(2, 5);
  oled.print("Score:");
  oled.print(gameScore >> 4);
  oled.setCursorXY(70, 5);
  oled.print("Best:");
  oled.print(obstacleCount);
  oled.fastLineH(MAX_BIRD_HEIGHT - 1, 0, 127);
}

void drawObstacles()
{
  for (uint8_t i = 0; i < obstacleCount; ++i) {
    uint8_t index = i > lastObstacle ? MAX_OBSTACLE_COUNT - (i - lastObstacle) : lastObstacle - i;

    int posX = DISPLAY_WIDTH - (uint8_t)((globalPos - obstacles[index].x) >> 8);
    oled.rect(posX, obstacles[index].y, posX + OBSTACLE_WIDTH, MAX_BIRD_HEIGHT, 1);
    oled.rect(posX, obstacles[index].y + OBSTACLE_WINDOWN_HEIGHT, posX + OBSTACLE_WIDTH, MIN_BIRD_HEIGHT, 1);
  }
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

    moveBird(inputFlags);
    processObstacles();
    checkCollision();

    gameScore += 1;
    globalPos += gameSpeed;
    inputFlags = 0;
  }

  if (displayTimer.check()) {
    displayTimer.reset();

    oled.clear();
    oled.fastLineH(MAX_BIRD_HEIGHT, 0, 127);
    oled.fastLineH(MIN_BIRD_HEIGHT, 0, 127);
    drawScoreboard();
    drawEnvironment();
    drawObstacles();
    oled.drawBitmap(bird.getPosX() >> 8, bird.getPosY() >> 8, (uint8_t *)bird.getBitmap(), bird.getSizeX(), bird.getSizeY(), BITMAP_NORMAL, BUF_ADD);

    oled.update();
  }

  if (debugTimer.check()) {
    debugTimer.reset();

  }
}
