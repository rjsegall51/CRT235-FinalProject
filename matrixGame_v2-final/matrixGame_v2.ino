#include <Adafruit_NeoMatrix.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>

#define WIDTH 32
#define HEIGHT 8
#define MATRIX_PIN 6
#define BUTTON_PIN 2

#define MIN_X 4
#define MIN_Y 0
#define MAX_X 28
#define MAX_Y 8

#define BASE_NOSE_X 10
#define BASE_NOSE_Y 5
#define BASE_GROUND_Y 7

#define JUMP_HEIGHT 4

#define START_INTERVAL 18
#define STARTING_LEVEL 1

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(WIDTH, HEIGHT, MATRIX_PIN,
  NEO_MATRIX_TOP    + NEO_MATRIX_LEFT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_GRB            + NEO_KHZ800);

boolean gameStarted = false;
boolean isPlaying = false;
boolean levelChange = false;
boolean triggered = false;
boolean striding = false;
boolean isDead = false;
boolean gameWon = false;
boolean crazy = false;

int reading = 1;
int lastReading = 1;

boolean jumping = false;
int jumpStartY = 0;

int curNoseX = BASE_NOSE_X;
int curNoseY = BASE_NOSE_Y;
int curGroundY = BASE_GROUND_Y;

int startScrollX = 32;
int levelScrollX = 32;
int endScrollX = 32;
int gameScrollX = 32;
int winScrollX = 32;

int blockWidth = 0;
int blockHeight = 0;
uint16_t blockColor;

int level = 1;
int levelScore = 0;

// game millis timer
unsigned long startTime = millis();
unsigned long endTime = 0;
unsigned long lastTrigger = 0;
int interval = START_INTERVAL;

// millis timer for dog running
unsigned long drawStartTime = millis();
unsigned long drawEndTime = 0;

// millis timer for dog jumping
unsigned long playerStartTime = millis();
unsigned long playerEndTime = 0;


// handles input interrupt with debounce
void trigger() {
  int now = millis();
  if (now - lastTrigger > 100) {
    triggered = true;
    lastTrigger = now;
  }
}


void initVars() {
  gameStarted = false;
  isPlaying = false;
  levelChange = false;
  triggered = false;
  striding = false;
  isDead = false;
  gameWon = false;
  crazy = false;

  reading = 1;
  lastReading = 1;
  
  jumping = false;
  jumpStartY = 0;
  
  curNoseX = BASE_NOSE_X;
  curNoseY = BASE_NOSE_Y;
  curGroundY = BASE_GROUND_Y;
  
  startScrollX = 32;
  levelScrollX = 32;
  endScrollX = 32;
  gameScrollX = 32;
  winScrollX = 32;
  
  blockWidth = 0;
  blockHeight = 0;
  blockColor = matrix.Color(0,0,0);
  
  level = 1;
  levelScore = 0;
  
  // game millis timer
  startTime = millis();
  endTime = 0;
  lastTrigger = 0;
  interval = START_INTERVAL;
  
  // millis timer for dog running
  drawStartTime = millis();
  drawEndTime = 0;
  
  // millis timer for dog jumping
  playerStartTime = millis();
  playerEndTime = 0;
}

// draws the current level screen to the matrix
void levelScreen() {
  if (triggered) {
    triggered = false;
    levelChange = false;
    levelScrollX = 32;
    return;
  }

  endTime = millis();

  if (endTime - startTime > interval) {
    startTime = millis();
  
    if (level < 4) {
      matrix.setTextColor(matrix.Color(0,255,0));
    }
    else if (level < 7) {
      matrix.setTextColor(matrix.Color(255,255,0));
    }
    else if (level <= 10) {
      matrix.setTextColor(matrix.Color(255,0,0));
    }
    else {
      gameWon = true;
      levelChange = false;
      levelScrollX = 32;
      return;
    }

    if (levelScrollX < -44) {
      levelChange = false;
      levelScrollX = 32;
      return;
    }
    
    String s = String("Level ") + level;
    matrix.fillScreen(0);
    matrix.setCursor(levelScrollX, 0);
    matrix.print(s);
    matrix.show();

    levelScrollX--;
  }
}


void winAnimation() {
  if (triggered) {
    initVars();
    return;
  }

  endTime = millis();

  if (endTime - startTime > interval) {
    startTime = millis();
    matrix.fillScreen(0);

    if (winScrollX < -60) {
      gameWon = false;
      crazy = true;
      return;
    }
    matrix.setCursor(winScrollX, 0);
    matrix.setTextColor(matrix.Color(0,255,0));
    matrix.print("You win!");
    matrix.show();
    
    winScrollX--;
  }
}


void drawGame() {
  
  if (level < 4) {
    blockColor = matrix.Color(0,255,0);
  }
  else if (level < 7) {
    blockColor = matrix.Color(255,255,0);
  }
  else {
    blockColor = matrix.Color(255,0,0);
  }

  if (gameScrollX < 0 - blockWidth) {
    levelScore++;
    gameScrollX = 32;

    if (levelScore >= 5) {
      level++;
      interval = START_INTERVAL;
      levelChange = true;
      levelScore = 0;
      return;
    }
  }

  // generate new random block
  if (gameScrollX == 32) {
    blockWidth = random(5, 17);
    blockHeight = (level > 3) ? random(1, 4) : random(1, 3);
  }

  // draw block
  matrix.fillRect(gameScrollX, HEIGHT - blockHeight, blockWidth, blockHeight, blockColor);

  if (gameScrollX <= curNoseX && gameScrollX + blockWidth > curNoseX - 4) {
    curGroundY = BASE_GROUND_Y - blockHeight;
  }
  else {
    curGroundY = BASE_GROUND_Y;
  }

  gameScrollX--;
}


void drawPlayer(int noseX, int noseY, int r, int g, int b) {

  /*if (triggered) {
    isJumping = true;
    triggered = false;
  }*/

  /*drawEndTime = millis();

  if (drawEndTime - drawStartTime > interval) {
    drawStartTime = millis();
  
    //matrix.clear();*/
  
    for (int x = (noseX - 3); x < noseX; x++) {
      matrix.drawPixel(x, noseY + 1, matrix.Color(r, g, b));
    }
    matrix.drawPixel(noseX - 1, noseY, matrix.Color(r, g, b));
    matrix.drawPixel(noseX, noseY, matrix.Color(r, g, b));
    
    if (striding) {
      matrix.drawPixel(noseX - 4, noseY + 2, matrix.Color(r, g, b));
      matrix.drawPixel(noseX, noseY + 2, matrix.Color(r, g, b));
    }
    else {
      matrix.drawPixel(noseX - 3, noseY + 2, matrix.Color(r, g, b));
      matrix.drawPixel(noseX - 1, noseY + 2, matrix.Color(r, g, b));
    }

    if (noseY + 2 > curGroundY) {
      interval = START_INTERVAL;
      isDead = true;
      return;
    }
    
    striding = !striding;
}



/**
 * Calculates the next player position and calls drawPlayer with 
 * the approprite coordinates
 */
void tick() {

  interval = START_INTERVAL - level;
  
  // button pressed
  if (triggered) {
    // start new jump if not in the air
    if (curNoseY + 2 == curGroundY) {
      jumping = true;
      jumpStartY = curNoseY;
      Serial.print("jump start: ");
      Serial.println(jumpStartY);
    }
    // reset button
    triggered = false;
  }

  playerEndTime = millis();
  
  // jumping and falling are on player timer
  if (playerEndTime - playerStartTime > interval) {
    playerStartTime = millis();
    Serial.print("pre: ");
    Serial.println(curNoseY);
      
    // move dog up if needed (negative y dir)
    if (jumping) { 
      curNoseY--;
      Serial.print("post: ");
      Serial.println(curNoseY);
  
      // end jump when height is reached
      if (curNoseY == jumpStartY - JUMP_HEIGHT) {
        jumping = false;
      }
    }
    
    // else fall if not on current ground (positive y dir)
    else if (curNoseY + 2 < curGroundY) {
      curNoseY++;
    }

    matrix.clear();
      
    // render game
    drawGame();
    
    // render player at cur position
    drawPlayer(curNoseX, curNoseY, 0,0,255);

    matrix.show();
  }

}


void start() {
  if (triggered) {
    isPlaying = true;
    triggered = false;
    level = STARTING_LEVEL;
    levelChange = true;
    startScrollX = 32;
    return;
  }

  endTime = millis();

  if (endTime - startTime > interval) {
    startTime = millis();
    matrix.fillScreen(0);

    if (startScrollX < -110) {
      startScrollX = 32;
    }
    matrix.setCursor(startScrollX, 0);
    matrix.setTextColor(matrix.Color(0,0,255));
    matrix.print("Jump to start game");
    matrix.show();
    
    startScrollX--;
  }
}


void crazyMode() {
  if (triggered) {
    initVars();
    return;
  }

  endTime = millis();
  
  if (endTime - startTime > interval) {
    startTime = millis();
    matrix.fillRect(0, 0, 32, 8, matrix.Color(random(150), random(150), random(150)));
    drawPlayer(curNoseX, curNoseY, 255,255,255);
  }
}


void gameOver() {
  if (triggered) {
    initVars();
    return;
  }

  endTime = millis();

  if (endTime - startTime > interval) {
    startTime = millis();
    matrix.fillScreen(0);

    if (endScrollX < -72) {
      initVars();
      return;
    }
    matrix.setCursor(endScrollX, 0);
    matrix.setTextColor(matrix.Color(255,0,0));
    matrix.print("Game over :(");
    matrix.show();
    
    endScrollX--;
  }
}



void setup() {
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(50);
  Serial.begin(9600);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  //attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), trigger, RISING);

  initVars();
}


void loop() {
  //matrix.clear();

  reading = digitalRead(BUTTON_PIN);
  if (reading == 1 && lastReading == 0) {
    triggered = true;
  }
  lastReading = reading;

  if (isPlaying) {
    if (isDead) {
      gameOver();
    }
    else if (levelChange) {
      levelScreen();
    }
    else if (gameWon) {
      winAnimation();
    }
    else if (crazy) {
      crazyMode();
    }
    else {
      tick();
    }
  }
  else {
    start();
  }

  matrix.show();
}
