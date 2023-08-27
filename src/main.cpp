#include <GyverOLED.h>
#include <charMap.h>
#include <icons_7x7.h>
#include <icons_8x8.h>
#include <EEPROM.h>
#include "timer.hpp"
#include "bird_character.hpp"

/* TODO:
 * - notify on new best score
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
#define BEST_SCORE_ADDRESS 1U

/* Input button flags */
#define JUMP_BUTTON_FLAG 0x01

/* Game time periods */
#define GAME_TICK_PERIOD 10
#define DISPLAY_REFRESH_RATE 30
#define USER_INPUT_CHECK_PERIOD 10

/* Game rule constants */
#define MAX_FIELD_HEIGHT (16U)
#define MIN_FIELD_HEIGHT (61U)
#define STARTING_POS_X (4U)
#define STARTING_POS_Y (29U)
#define MAX_OBSTACLE_COUNT (4U)
#define MAX_OBSTACLE_HEIGHT (20U)
#define MIN_OBSTACLE_HEIGHT (55U)
#define OBSTACLE_WIDTH (12U)
#define OBSTACLE_WINDOWN_HEIGHT (25U)
#define OBSTACLE_DISTANCE (50L)
#define DEFAULT_GAME_SPEED (128U)
#define JUMP_ACCEL (1U << 8)
#define GRAVITY_ACCEL (1U << 4)

GyverOLED<SSD1306_128x64, OLED_BUFFER, OLED_SPI, OLED_CS, OLED_DC, OLED_RST> oled;

Timer gameTimer(GAME_TICK_PERIOD);
Timer userInputTimer(USER_INPUT_CHECK_PERIOD);
Timer displayTimer(1000 / DISPLAY_REFRESH_RATE);
Timer debugTimer(1000);

Bird bird;

enum GameState {
  MENU,
  GAME,
  GAME_OVER
};

/* Game variables */
long gameScore = 0;
GameState gameState = MENU;
int gameSpeed = DEFAULT_GAME_SPEED;
long globalPos = 256;
uint8_t inputFlags = 0;
int bestScore = 0;

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

  if (posY < (MAX_FIELD_HEIGHT << 8)) {
    posY = (MAX_FIELD_HEIGHT << 8);
    bird.setVelY(0);
  }
  if (posY > ((MIN_FIELD_HEIGHT - bird.getSizeY()) << 8)) {
    posY = ((MIN_FIELD_HEIGHT - bird.getSizeY()) << 8);
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
  bool collision = false;
  
  for (uint8_t i = 0; i < obstacleCount; ++i) {
    uint8_t index = i > lastObstacle ? MAX_OBSTACLE_COUNT - (i - lastObstacle) : lastObstacle - i;

    unsigned int obstPosX = ((long)DISPLAY_WIDTH << 8) - (globalPos - obstacles[index].x);
    if ((obstPosX > (int)(STARTING_POS_X + BirdSizeX) << 8)) {
      continue;
    }

    struct Point2D
    {
      int x;
      unsigned int y;
    };
    
    Point2D charPoints[6] = {{ bird.getPosX()                       , ((unsigned)bird.getPosY() >> 8)             },
                             { bird.getPosX()                       , ((unsigned)bird.getPosY() >> 8) + BirdSizeY },
                             { bird.getPosX() + (BirdSizeX << 8)    , ((unsigned)bird.getPosY() >> 8)             },
                             { bird.getPosX() + (BirdSizeX << 8)    , ((unsigned)bird.getPosY() >> 8) + BirdSizeY },
                             { bird.getPosX() + (BirdSizeX / 2 << 8), ((unsigned)bird.getPosY() >> 8)             },
                             { bird.getPosX() + (BirdSizeX / 2 << 8), ((unsigned)bird.getPosY() >> 8) + BirdSizeY }};

    if ((charPoints[0].x > (int)obstPosX && charPoints[0].x < (int)(obstPosX + (OBSTACLE_WIDTH << 8)) && charPoints[0].y < obstacles[index].y) ||
        (charPoints[2].x > (int)obstPosX && charPoints[2].x < (int)(obstPosX + (OBSTACLE_WIDTH << 8)) && charPoints[2].y < obstacles[index].y) ||
        (charPoints[4].x > (int)obstPosX && charPoints[4].x < (int)(obstPosX + (OBSTACLE_WIDTH << 8)) && charPoints[4].y < obstacles[index].y) ||
        (charPoints[1].x > (int)obstPosX && charPoints[1].x < (int)(obstPosX + (OBSTACLE_WIDTH << 8)) && charPoints[1].y > obstacles[index].y + OBSTACLE_WINDOWN_HEIGHT) ||
        (charPoints[3].x > (int)obstPosX && charPoints[3].x < (int)(obstPosX + (OBSTACLE_WIDTH << 8)) && charPoints[3].y > obstacles[index].y + OBSTACLE_WINDOWN_HEIGHT) ||
        (charPoints[5].x > (int)obstPosX && charPoints[5].x < (int)(obstPosX + (OBSTACLE_WIDTH << 8)) && charPoints[5].y > obstacles[index].y + OBSTACLE_WINDOWN_HEIGHT))
    {
      collision = true;
      break;
    }
  }

  return collision;
}

void drawEnvironment()
{
  uint8_t state = (globalPos >> 8) & 0x3F;
  oled.drawBitmap(100 - state, MIN_FIELD_HEIGHT - 6, tree_1, 5, 6);
  oled.drawBitmap(100 - 64 - state, MIN_FIELD_HEIGHT - 6, tree_1, 5, 6);
  oled.drawBitmap(100 + 64 - state, MIN_FIELD_HEIGHT - 6, tree_1, 5, 6);
  oled.dot(10 - state, MIN_FIELD_HEIGHT + 1);
  oled.dot(12 - state, MIN_FIELD_HEIGHT + 2);
  oled.dot(25 - state, MIN_FIELD_HEIGHT + 1);
  oled.dot(24 - state, MIN_FIELD_HEIGHT + 2);  
  oled.dot(40 - state, MIN_FIELD_HEIGHT + 1);
  oled.dot(41 - state, MIN_FIELD_HEIGHT + 2);
  oled.dot(55 - state, MIN_FIELD_HEIGHT + 1);
  oled.dot(55 - state, MIN_FIELD_HEIGHT + 2);
  oled.dot(64 + 10 - state, MIN_FIELD_HEIGHT + 1);
  oled.dot(64 + 12 - state, MIN_FIELD_HEIGHT + 2);
  oled.dot(64 + 25 - state, MIN_FIELD_HEIGHT + 1);
  oled.dot(64 + 24 - state, MIN_FIELD_HEIGHT + 2);  
  oled.dot(64 + 40 - state, MIN_FIELD_HEIGHT + 1);
  oled.dot(64 + 41 - state, MIN_FIELD_HEIGHT + 2);
  oled.dot(64 + 55 - state, MIN_FIELD_HEIGHT + 1);
  oled.dot(64 + 55 - state, MIN_FIELD_HEIGHT + 2);
  oled.dot(128 + 10 - state, MIN_FIELD_HEIGHT + 1);
  oled.dot(128 + 12 - state, MIN_FIELD_HEIGHT + 2);
  oled.dot(128 + 25 - state, MIN_FIELD_HEIGHT + 1);
  oled.dot(128 + 24 - state, MIN_FIELD_HEIGHT + 2);  
  oled.dot(128 + 40 - state, MIN_FIELD_HEIGHT + 1);
  oled.dot(128 + 41 - state, MIN_FIELD_HEIGHT + 2);
  oled.dot(128 + 55 - state, MIN_FIELD_HEIGHT + 1);
  oled.dot(128 + 55 - state, MIN_FIELD_HEIGHT + 2);
}

void drawScoreboard()
{
  switch (gameState)
  {
  case MENU:
    oled.setCursorXY(2, 5);
    oled.print("Jumpy Bird");
    oled.setCursorXY(70, 5);
    oled.print("Best:");
    oled.print(bestScore);
    break;

  case GAME:
    oled.setCursorXY(2, 5);
    oled.print("Score:");
    oled.print(gameScore >> 5);
    oled.setCursorXY(70, 5);
    oled.print("Best:");
    oled.print(bestScore);
    break;

  case GAME_OVER:
    oled.setCursorXY(35, 5);
    oled.print("Game Over!");
    break;
  }

  oled.fastLineH(MAX_FIELD_HEIGHT - 1, 0, 127);
}

void drawObstacles()
{
  for (uint8_t i = 0; i < obstacleCount; ++i) {
    uint8_t index = i > lastObstacle ? MAX_OBSTACLE_COUNT - (i - lastObstacle) : lastObstacle - i;

    int posX = DISPLAY_WIDTH - (uint8_t)((globalPos - obstacles[index].x) >> 8);
    oled.rect(posX, obstacles[index].y, posX + OBSTACLE_WIDTH, MAX_FIELD_HEIGHT, 1);
    oled.rect(posX, obstacles[index].y + OBSTACLE_WINDOWN_HEIGHT, posX + OBSTACLE_WIDTH, MIN_FIELD_HEIGHT, 1);
  }
}

void setup() {
  Serial.begin(115200);
  SPI.begin();
  pinMode(BTN_PIN, INPUT_PULLUP);
  if (EEPROM.read(0) != 'a') {
    EEPROM.update(0, 'a');
    EEPROM.put(BEST_SCORE_ADDRESS, (int)0);
  }
  EEPROM.get(BEST_SCORE_ADDRESS, bestScore);

  oled.init();
  oled.clear();   // очистить дисплей (или буфер)
  oled.update();  // обновить. Только для режима с буфером! OLED_BUFFER

  bird.setPosXY(STARTING_POS_X << 8, STARTING_POS_Y << 8);
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

    switch (gameState)
    {
    case MENU:
      if (inputFlags & JUMP_BUTTON_FLAG) {
        gameState = GAME;
      }
      break;
    
    case GAME:
      moveBird(inputFlags);
      processObstacles();
      if (checkCollision()) {
        gameState = GAME_OVER;
        if ((gameScore >> 5) > bestScore) {
          bestScore = gameScore >> 5;
          EEPROM.put(BEST_SCORE_ADDRESS, bestScore);
        }
      }

      gameScore += 1;
      globalPos += gameSpeed;
      break;

    case GAME_OVER:
      if (inputFlags & JUMP_BUTTON_FLAG) {
        globalPos = 256;
        gameScore = 0;
        gameSpeed = DEFAULT_GAME_SPEED;
        obstacleCount = 0;
        bird.setPosXY(STARTING_POS_X, STARTING_POS_Y);
        bird.setVelXY(0, 0);
        gameState = GAME;
      }
      break;
    }
    inputFlags = 0;
  }

  if (displayTimer.check()) {
    displayTimer.reset();

    oled.clear();

    switch (gameState)
    {
    case MENU:
      static Timer animTimer(1000);
      static bool animState = false;

      if (animTimer.check()) {
        animTimer.reset();
        animState = !animState;
      }

      if (animState) {
        oled.drawBitmap(10, STARTING_POS_Y - 1, (uint8_t *)bird.getBitmap(), bird.getSizeX(), bird.getSizeY(), BITMAP_NORMAL, BUF_ADD);
      } else {
        oled.drawBitmap(10, STARTING_POS_Y + 1, (uint8_t *)bird.getBitmap(), bird.getSizeX(), bird.getSizeY(), BITMAP_NORMAL, BUF_ADD);
      }
      oled.setCursorXY(45, 34);
      oled.print("Press JUMP!");
      break;
    
    case GAME:
      oled.drawBitmap(bird.getPosX() >> 8, bird.getPosY() >> 8, (uint8_t *)bird.getBitmap(), bird.getSizeX(), bird.getSizeY(), BITMAP_NORMAL, BUF_ADD);

      break;

    case GAME_OVER:
      oled.drawBitmap(bird.getPosX() >> 8, bird.getPosY() >> 8, (uint8_t *)bird.getBitmap(), bird.getSizeX(), bird.getSizeY(), BITMAP_NORMAL, BUF_ADD);
      oled.rect(8, 31, 110, 43, 0);
      oled.setCursorXY(12, 34);
      oled.print("Your score: ");
      oled.print(gameScore >> 5);
      break;
    }

    drawScoreboard();
    drawEnvironment();
    drawObstacles();
    oled.fastLineH(MAX_FIELD_HEIGHT, 0, 127);
    oled.fastLineH(MIN_FIELD_HEIGHT, 0, 127);
    
    oled.update();
  }

  if (debugTimer.check()) {
    // debugTimer.reset();

  }
}
