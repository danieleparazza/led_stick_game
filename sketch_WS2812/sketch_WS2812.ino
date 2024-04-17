#include <FastLED.h>

// Constants for pins and game parameters
#define LED_USER             13
#define LED_PIN              2
#define NUM_LEDS             (60 * 5)
#define UI_COIN_PIN          9
#define UI_UP_PIN            10
#define UI_DN_PIN            11
#define UI_ATTACK_PIN        12
#define COIN_LED_PIN         3
#define UI_VDIR_STOP         0
#define UI_VDIR_UP           1
#define UI_VDIR_DN           2
#define UI_BTN_RELEASED      0
#define UI_BTN_PRESSED       1
#define NUM_ENEMIES          10
#define GAME_START           0
#define GAME_PLAY            1
#define GAME_OVER            2
#define GAME_WIN             3
#define GAME_END             4
#define GAME_CHEAT           5
#define PLAYER_ATTACK_RANGE  3
#define PLAYER_ATTACK_MAX_DURATION_MS 2500
#define PLAYER_ATTACK_INTERVAL_MS     2000
#define MAX_SEQUENCE_LENGTH 20
#define NUM_CHEAT_CODES 0
#define LED_DIRECTION_FW 0b01
#define LED_DIRECTION_BW 0b10
#define LED_EFFECT_NONE   0b0
#define LED_EFFECT_FLASH  0b1
#define DEBUG_CHEAT false

// Define structures for game objects
typedef struct {
  int position;
  int originalPosition;
  int movementRange;
  int8_t movementDirection = 1;
  unsigned int enemyMovementTimer;
  unsigned int attackTimer;
  unsigned int attackTiming;
} Enemy;

typedef struct {
  uint8_t gamePhase;
  int playerPosition;
  uint8_t uiVerticalDirection;
  uint8_t uiAttack;
  uint8_t uiCoinInserted;
  uint8_t currentAttackRange;
  CRGB leds[NUM_LEDS];
  CRGB led_off;
  CRGB led_enemy = CRGB(255, 0, 0);;
  CRGB led_player;
  CRGB led_attack = CRGB(255, 0, 255);
  CRGB led_trophy = CRGB(0, 255, 0);
  unsigned int playerBreatheTimer;
  unsigned int uiAttackTimer;
  bool newAttackAvailable;
  uint8_t playerBreatheDirection;
  Enemy enemies[NUM_ENEMIES];
  int currentEnemy;
  bool gameOver;
  int currentCheatCode;
} GameState;

void(* resetFunc) (void) = 0;

// Definisci i cheat codes come array di sequenze di tasti
const int cheatCodes[NUM_CHEAT_CODES][MAX_SEQUENCE_LENGTH] = {
};

const int cheatActions[NUM_CHEAT_CODES] = {
};

// Timer and LED state variables
unsigned int userLedToggleTimer;
int userLedState = HIGH;
unsigned int userLedBlinkInterval;
int inputBuffer[MAX_SEQUENCE_LENGTH];
int inputBufferIndex = 0;
int cheatResetTimer = 0;

GameState gameState;

// Function prototypes
void setUserLED(int status);
void initializeGame(GameState *pGameState);
void handleUserInput(GameState *pGameState);
void drawLeds(GameState *pGameState);
void managePlayerPosition(GameState *pGameState);
void playerBreathe(GameState *pGameState);
void playerAttack(GameState *pGameState);
void enemyMovement(GameState *pGameState);
void handleGameOver(GameState *pGameState);
void manageTimers(GameState *pGameState);
void openingSequence(GameState *pGameState);
void closingSequence(GameState *pGameState, CRGB explosionColor, int startIndex);
void setupSysTick();
void setupSysGPIO();
void manageUserLed();
void setupGame();

void setUserLED(int status) {
  digitalWrite(LED_USER, status);
}

void initializeGame(GameState *pGameState) {
  pGameState->playerPosition = 0;
  pinMode(UI_UP_PIN, INPUT);
  pinMode(UI_DN_PIN, INPUT);
  pinMode(UI_COIN_PIN, INPUT);
  pinMode(UI_ATTACK_PIN, INPUT);
  pinMode(COIN_LED_PIN, OUTPUT);
  FastLED.addLeds<WS2812, LED_PIN, GRB>(pGameState->leds, NUM_LEDS);

  for (int i = 1; i <= NUM_ENEMIES; i++) {
    pGameState->enemies[i - 1].originalPosition = pGameState->enemies[i - 1].position = (NUM_LEDS / NUM_ENEMIES) * i;
    pGameState->enemies[i - 1].movementRange = 15 * i;

    Serial.print("Created enemy ");
    Serial.print(i - 1);
    Serial.print(" at position ");
    Serial.print(pGameState->enemies[i - 1].position);
    Serial.print(" with movement range of ");
    Serial.println(pGameState->enemies[i - 1].movementRange);
  }
}

void handleUserInput(GameState *pGameState) {
  static int buttonPressMask = 0;
  int buttonPressMaskNew = 0;
  int btnUp = digitalRead(UI_UP_PIN);
  int btnDn = digitalRead(UI_DN_PIN);
  int btnAtt = digitalRead(UI_ATTACK_PIN);

  buttonPressMaskNew = ((btnUp & 1) << 0) 
                      | ((btnDn & 1) << 1) 
                      | ((btnAtt & 1) << 2) ;

  if(buttonPressMaskNew != buttonPressMask) {
    cheatResetTimer = 2000;
    buttonPressMask = buttonPressMaskNew;
    inputBuffer[inputBufferIndex] = (btnUp == HIGH) ? UI_UP_PIN :
                                    (btnDn == HIGH) ? UI_DN_PIN :
                                    (btnAtt == HIGH) ? UI_ATTACK_PIN : 0;
    if(DEBUG_CHEAT) Serial.println(inputBuffer[inputBufferIndex], HEX);
    inputBufferIndex = (inputBufferIndex + 1) % MAX_SEQUENCE_LENGTH;
  }
  
  if (btnUp == HIGH && btnDn == LOW) {
    pGameState->uiVerticalDirection = UI_VDIR_UP;
  } else if (btnUp == LOW && btnDn == HIGH) {
    pGameState->uiVerticalDirection = UI_VDIR_DN;
  } else {
    pGameState->uiVerticalDirection = UI_VDIR_STOP;
  }

  if (btnAtt == HIGH && btnUp == LOW && btnDn == LOW && pGameState->newAttackAvailable) {
    if (pGameState->uiAttackTimer == 0) {
      pGameState->uiAttackTimer = PLAYER_ATTACK_MAX_DURATION_MS;
      pGameState->newAttackAvailable = false;
    }
  } else if (btnAtt == LOW) {
    if (pGameState->uiAttackTimer > PLAYER_ATTACK_INTERVAL_MS) pGameState->uiAttackTimer = PLAYER_ATTACK_INTERVAL_MS;
    pGameState->newAttackAvailable = true;
  }
}

void drawLeds(GameState *pGameState) {
  for (int i = 0; i < NUM_LEDS; i++) {

    if (pGameState->playerPosition == i) {
      pGameState->leds[i] = pGameState->led_player;
    } else if (pGameState->enemies[pGameState->currentEnemy].position == i) {
      pGameState->leds[i] = pGameState->led_enemy;
    } else if (pGameState->currentEnemy > i) {
      pGameState->leds[i] = pGameState->led_trophy;
    } else {
      pGameState->leds[i] = pGameState->led_off;
    }
  }

  if (pGameState->currentAttackRange > 0) {
    for (int i = 0; i < pGameState->currentAttackRange; i++) {
      if (pGameState->playerPosition + i < NUM_LEDS) pGameState->leds[pGameState->playerPosition + i] = pGameState->led_attack;
      // if(pGameState->playerPosition-i >= 0) pGameState->leds[pGameState->playerPosition-i] = pGameState->led_attack;
    }
  }

  FastLED.show();
}

void managePlayerPosition(GameState *pGameState) {
  switch (pGameState->uiVerticalDirection) {

    case UI_VDIR_UP:
      if (++pGameState->playerPosition >= NUM_LEDS) pGameState->playerPosition = NUM_LEDS - 1;
      break;

    case UI_VDIR_DN:
      if (--pGameState->playerPosition < 0) pGameState->playerPosition = 0;
      break;

    default:
      break;
  }
}

void playerBreathe(GameState *pGameState) {
  if (pGameState->playerBreatheTimer) return;
  pGameState->playerBreatheTimer = 10;

  if (pGameState->led_player.r <= 80) pGameState->playerBreatheDirection = 1;
  if (pGameState->led_player.r >= 248) pGameState->playerBreatheDirection = -1;

  pGameState->led_player.r += 8 * pGameState->playerBreatheDirection;
  pGameState->led_player.g += 8 * pGameState->playerBreatheDirection;
  pGameState->led_player.b += 8 * pGameState->playerBreatheDirection;
}

void playerAttack(GameState *pGameState) {
  pGameState->currentAttackRange = (pGameState->uiAttackTimer > PLAYER_ATTACK_INTERVAL_MS) ? PLAYER_ATTACK_RANGE : 0;
  if (pGameState->currentAttackRange == 0) return;

  if (pGameState->enemies[pGameState->currentEnemy].position <= pGameState->playerPosition + pGameState->currentAttackRange) {
    Serial.println("Enemy defeated!!!");
    pGameState->currentEnemy++;
    Serial.print("New enemy: ");
    Serial.println(pGameState->currentEnemy);
  }
}

void enemyMovement(GameState *pGameState) {
  if (pGameState->enemies[pGameState->currentEnemy].enemyMovementTimer) return;
  pGameState->enemies[pGameState->currentEnemy].enemyMovementTimer = 100;

  if ((pGameState->enemies[pGameState->currentEnemy].position >= (pGameState->enemies[pGameState->currentEnemy].originalPosition + pGameState->enemies[pGameState->currentEnemy].movementRange)) || (pGameState->enemies[pGameState->currentEnemy].position >= NUM_LEDS)) {
    if (pGameState->enemies[pGameState->currentEnemy].position >= NUM_LEDS) pGameState->enemies[pGameState->currentEnemy].position = NUM_LEDS;
    pGameState->enemies[pGameState->currentEnemy].movementDirection = -1;
  } else if ((pGameState->enemies[pGameState->currentEnemy].position <= (pGameState->enemies[pGameState->currentEnemy].originalPosition - pGameState->enemies[pGameState->currentEnemy].movementRange)) || (pGameState->enemies[pGameState->currentEnemy].position <= 0)) {
    if (pGameState->enemies[pGameState->currentEnemy].position <= 0) pGameState->enemies[pGameState->currentEnemy].position = 0;
    pGameState->enemies[pGameState->currentEnemy].movementDirection = 1;
  }

  pGameState->enemies[pGameState->currentEnemy].position += 1 * pGameState->enemies[pGameState->currentEnemy].movementDirection;
}

void handleGameOver(GameState *pGameState) {
  if (pGameState->currentEnemy >= NUM_ENEMIES) {
    pGameState->gamePhase = GAME_WIN;
    Serial.println("Game Win!");
  } else if (pGameState->playerPosition >= pGameState->enemies[pGameState->currentEnemy].position) {
    Serial.print("Current enemy index ");
    Serial.println(pGameState->currentEnemy);
    Serial.print("Player position ");
    Serial.println(pGameState->playerPosition);
    Serial.print("Enemy position ");
    Serial.println(pGameState->enemies[pGameState->currentEnemy].position);

    pGameState->gamePhase = GAME_OVER;
    Serial.println("Game Over!");
  }
}

void manageTimers(GameState *pGameState) {
  if (pGameState->playerBreatheTimer) pGameState->playerBreatheTimer--;
  if (pGameState->uiAttackTimer) pGameState->uiAttackTimer--;
  if (pGameState->enemies[pGameState->currentEnemy].enemyMovementTimer) pGameState->enemies[pGameState->currentEnemy].enemyMovementTimer--;
  if (userLedToggleTimer) userLedToggleTimer--;
  if (cheatResetTimer) cheatResetTimer--;
}

void openingSequence(GameState *pGameState) {
  static uint8_t startIndex = 0;
  static const int colorsCount = 5;
  CRGB colors[colorsCount] = { CRGB::Red, CRGB::Orange, CRGB::Yellow, CRGB::Green, CRGB::Blue };

  for (int i = 0; i < NUM_LEDS; i++) {
    int colorIndex = (i * colorsCount) / NUM_LEDS;  // Spread colors evenly along the strip
    int hue = startIndex + (i * 255 / NUM_LEDS);    // Gradually change hue along the strip
    pGameState->leds[i] = colors[colorIndex];
    pGameState->leds[i].setHue(hue);
  }

  FastLED.show();
  startIndex += 2;
  if (startIndex >= 254) {
    pGameState->gamePhase = GAME_PLAY;
    Serial.println("Gameplay phase");
  }
}

void closingSequence(GameState *pGameState, CRGB explosionColor, int startIndex, int direction, int effect) {
  static const int maxBrightness = 255;
  static const int fadeDuration = 500;  // Milliseconds
  static const int maxRadius = NUM_LEDS;      // Adjust the explosion radius as needed
  static const int blinkFrequency = 5;
  static int blinkToggleCounter = 0;
  static bool blinkToggle = true;

  for (int radius = 0; radius <= maxRadius; radius++) {
    for (int i = 0; i < NUM_LEDS; i++) {
      int _distance = i - startIndex;
      if(direction == LED_DIRECTION_FW && _distance < 0) continue;
      if(direction == LED_DIRECTION_BW && _distance > 0) continue;
      int distance = abs(_distance);

      if (distance <= radius) {
        int brightness = map(distance, 0, radius, maxBrightness, 0);
        pGameState->leds[i] = explosionColor;
        pGameState->leds[i].fadeToBlackBy(brightness);
      }
    }

    if(effect == LED_EFFECT_FLASH) {
      if(blinkToggleCounter++ % blinkFrequency == 0) blinkToggle = !blinkToggle;
    } else {
      blinkToggle = true;
    }
    if(!blinkToggle && radius < maxRadius) {
      memset(pGameState->leds, 0, sizeof(pGameState->leds[0]) * NUM_LEDS);
    }
    FastLED.show();
    delay(fadeDuration / (maxRadius + 1));
  }

  // Clear the LEDs after the explosion
  FastLED.show();
  pGameState->gamePhase = GAME_END;
  Serial.println("Game ended");
}

void setupSysTick() {
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 249;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS11);
  TIMSK1 |= (1 << OCIE1A);
  interrupts();
}

// ogni 1 ms
ISR(TIMER1_COMPA_vect) {
  manageTimers(&gameState);
}

void setupSysGPIO() {
  pinMode(LED_USER, OUTPUT);
}

void manageUserLed() {
  if (userLedToggleTimer) return;
  userLedToggleTimer = userLedBlinkInterval;

  userLedState = !userLedState;
  setUserLED(userLedState);
}

void setup() {
  Serial.begin(9600);

  setupSysTick();
  setupSysGPIO();

  initializeGame(&gameState);
  Serial.println("Game started");
}

void manageCoinButton(GameState * pGameState) {
  static bool cheatCheckDebounce;
  int btnCoin = digitalRead(UI_COIN_PIN);
  pGameState->uiCoinInserted = (btnCoin == HIGH) && (pGameState->gamePhase == GAME_END);
  digitalWrite(COIN_LED_PIN, userLedBlinkInterval);
  if (pGameState->uiCoinInserted) {
    resetFunc();
  }

  if(btnCoin == LOW) {
    cheatCheckDebounce = false;

  } else if(btnCoin == HIGH && !cheatCheckDebounce) {
    cheatCheckDebounce = true;
    for (int i = 0; i < NUM_CHEAT_CODES; ++i) {
      int *cheatCode = cheatCodes[i];
      int cheatLength = 0;
      while (cheatLength < MAX_SEQUENCE_LENGTH && cheatCode[cheatLength] != -1) {
        ++cheatLength;
      }

      if(DEBUG_CHEAT) {
        Serial.println("Inserted cheat:");
        for(int c=0; c<cheatLength; c++) {
          Serial.print(inputBuffer[c], HEX);
          Serial.print("-");
        }
        Serial.println();
        Serial.println("Compared to cheat:");
        for(int c=0; c<cheatLength; c++) {
          Serial.print(cheatCode[c], HEX);
          Serial.print("-");
        }
        Serial.println();
      }
      if(memcmp(cheatCode, inputBuffer, cheatLength) == 0) {
        Serial.println("Cheat ok");
        pGameState->currentCheatCode = cheatActions[i];
        pGameState->gamePhase = GAME_CHEAT;
      } else {
        Serial.println("Cheat ko");
      }
    }

    cheatResetTimer = 0;
  }
  
  if(inputBufferIndex > 0 && cheatResetTimer == 0) {
    if(DEBUG_CHEAT) Serial.println("Cheat sequence reset");
    memset(inputBuffer, 0, sizeof(MAX_SEQUENCE_LENGTH));
    inputBufferIndex = 0;
  }
}

void loop() {
  static unsigned int mainLoopDelay = 0;
  delay(mainLoopDelay);                 // Delay the main loop by the specified delay time

  manageUserLed();                      // Manage the user LED (Toggle its state)
  manageCoinButton(&gameState);         // Manage the coin button

  switch (gameState.gamePhase) {
    case GAME_START:
      userLedBlinkInterval = 250;       // Set the user LED blink interval
      openingSequence(&gameState);      // Execute the opening sequence of the game
      break;

    case GAME_PLAY:
      mainLoopDelay = 20;               // Set the main loop delay for game playing
      userLedBlinkInterval = 1000;      // Set the user LED blink interval during gameplay
      handleUserInput(&gameState);      // Handle user input (button presses)
      managePlayerPosition(&gameState); // Manage the player's position
      playerBreathe(&gameState);        // Simulate player breathing effect
      playerAttack(&gameState);         // Handle player attacks
      enemyMovement(&gameState);        // Manage enemy movement
      handleGameOver(&gameState);       // Check for game over conditions
      drawLeds(&gameState);             // Update and draw the LEDs to display the game state
      break;

    case GAME_CHEAT:
      mainLoopDelay = 0;                // No delay in the main loop      
      switch (gameState.currentCheatCode) {
        default:
          break;
      }
      break;

    case GAME_OVER:
      mainLoopDelay = 0;                // No delay in the main loop
      closingSequence(&gameState, CRGB(255, 0, 0), gameState.playerPosition, LED_DIRECTION_FW | LED_DIRECTION_BW, LED_EFFECT_NONE);  // Execute the closing sequence for game over
      break;

    case GAME_WIN:
      mainLoopDelay = 0;                // No delay in the main loop
      closingSequence(&gameState, CRGB(0, 255, 0), NUM_LEDS - 1, LED_DIRECTION_FW | LED_DIRECTION_BW, LED_EFFECT_NONE);  // Execute the closing sequence for game win
      break;

    default:
      break;
  }
}


