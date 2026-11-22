#include <Adafruit_NeoPixel.h>

#define DATA_PIN 6
#define BUTTON_PIN 2
#define NUM_LEDS 64

Adafruit_NeoPixel leds(NUM_LEDS, DATA_PIN, NEO_GRB + NEO_KHZ800);

// ========================================
// SOUND & HINTERGRUNDMELODIE
// ========================================
const int buzzer = 8;

const int melody[] = {
  247, 262, 247, 262, 196, 220,
  247, 262, 247, 262, 196, 220
};

const int noteDurations[] = {

  // Zum umstellen der Soundgeschwingigkeit: fals der sound langsammer sein soll denn schnellen Teil aukommentiren und beim Langsammen die doppel Striche endfernen 
  // oder für schnelleren sound das gleiche nur, dass getauscht ist welches man auskommentiert und wo man die Schrägstiche endfernt
  
  //Langsam
 // 250, 250, 250, 500, 1000, 1000,
 // 250, 250, 250, 250, 1000, 1000

  // Scheller
   125, 125, 125, 250, 500, 500,
   125, 125, 125, 125, 500, 500,
};

const int TOTAL_NOTES = 12;
int currentNote = 0;
unsigned long noteStartTime = 0;
bool notePlaying = false;
bool notePaused = false; // NEU: Verhindert Dauerfeuer von noTone()

// ========================================
// SPIEL-EINSTELLUNGEN
// ========================================
const int PLAYER_X = 1;
int playerLED = 9;

int obstacleX = 7;
int obstacleHeight = 1;

const int CHANCE_ZWEI_LEDS = 20; 
const unsigned long WARTEZEIT_GROSSE_HINDERNISSE = 10000; 

bool jumping = false;
int jumpStep = 0;

unsigned long lastJumpMove = 0;
unsigned long lastObstacleMove = 0;
unsigned long lastSpeedIncrease = 0;
unsigned long gameStartTime = 0;

const int jumpSpeed = 180;

const int START_SPEED = 350;         
int currentObstacleSpeed = START_SPEED; 
const int GESCHWINDIGKEIT_STEIGERUNG = 15; 
const int MAX_SPEED_LIMIT = 50; 


// MUSIK-FUNKTION 

void updateMusic(unsigned long now)
{
  if (!notePlaying)
  {
    tone(buzzer, melody[currentNote]);
    noteStartTime = now;
    notePlaying = true;
    notePaused = false;
  }
  else
  {
    unsigned long elapsed = now - noteStartTime;
    unsigned long duration = noteDurations[currentNote];

    // Nur EINMAL stoppen, wenn die Pause beginnt
    if (elapsed >= duration - 15 && elapsed < duration && !notePaused)
    {
      noTone(buzzer);
      digitalWrite(buzzer, HIGH);
      notePaused = true;
    }
    else if (elapsed >= duration)
    {
      currentNote++;
      if (currentNote >= TOTAL_NOTES)
      {
        currentNote = 0;
      }
      notePlaying = false;
    }
  }
}


// BODEN & HINDERNIS ZEICHNEN

void drawGround()
{
  for (int i = 0; i < 8; i++)
  {
    leds.setPixelColor(i, leds.Color(0, 80, 0));
  }
}

void drawObstacle()
{
  if (obstacleX >= 0 && obstacleX <= 7)
  {
    leds.setPixelColor(obstacleX + 8, leds.Color(255, 0, 0));

    if (obstacleHeight == 2)
    {
      leds.setPixelColor(obstacleX + 16, leds.Color(255, 0, 0));
    }
  }
}

void drawGame()
{
  // WICHTIG: Ton für den Bruchteil einer Millisekunde pausieren,
  // damit die Interrupts das NeoPixel-Signal nicht zerhacken!
  if (notePlaying && !notePaused) {
    noTone(buzzer);
  }

  leds.clear();
  drawGround();
  leds.setPixelColor(playerLED, leds.Color(0, 0, 255));
  drawObstacle();
  leds.show();

  // Ton direkt danach wieder fortsetzen
  if (notePlaying && !notePaused) {
    tone(buzzer, melody[currentNote]);
  }
}


// GAME OVER

void gameOver()
{
  noTone(buzzer);
  digitalWrite(buzzer, HIGH);

  for (int r = 0; r < 3; r++)
  {
    for (int i = 0; i < NUM_LEDS; i++)
    {
      leds.setPixelColor(i, leds.Color(255, 0, 0));
    }
    leds.show();
    delay(200);

    leds.clear();
    leds.show();
    delay(200);
  }

  playerLED = 9;
  obstacleX = 7;
  obstacleHeight = 1;
  currentObstacleSpeed = START_SPEED; 

  jumping = false;
  jumpStep = 0;
  
  currentNote = 0;
  notePlaying = false;
  notePaused = false;
  
  gameStartTime = millis(); 
  drawGame();
}

// ========================================
// SETUP
// ========================================
void setup()
{
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, HIGH);

  randomSeed(analogRead(0)); 

  leds.begin();
  leds.setBrightness(40);

  leds.clear();
  leds.show();

  gameStartTime = millis(); 
  drawGame();
}

// ========================================
// LOOP
// ========================================
void loop()
{
  unsigned long now = millis();
  bool screenChanged = false;

  updateMusic(now);

  // Taster
  if (digitalRead(BUTTON_PIN) == LOW)
  {
    if (!jumping)
    {
      jumping = true;
      jumpStep = 0;
      lastJumpMove = now;
    }
  }

  // Sprung
  if (jumping)
  {
    if (now - lastJumpMove >= jumpSpeed)
    {
      lastJumpMove = now;
      jumpStep++;

      if (jumpStep == 1)      playerLED = 17;
      else if (jumpStep == 2) playerLED = 25;
      else if (jumpStep == 3) playerLED = 25;
      else if (jumpStep == 4) playerLED = 17;
      else if (jumpStep == 5)
      {
        playerLED = 9;
        jumping = false;
        jumpStep = 0;
      }
      screenChanged = true;
    }
  }

  // Hindernis bewegen
  if (now - lastObstacleMove >= currentObstacleSpeed)
  {
    lastObstacleMove = now;
    obstacleX--;

    if (obstacleX < 0)
    {
      obstacleX = 7;
      
      if (now - gameStartTime >= WARTEZEIT_GROSSE_HINDERNISSE) 
      {
        if (random(0, 100) < CHANCE_ZWEI_LEDS) 
        {
          obstacleHeight = 2;
        } 
        else 
        {
          obstacleHeight = 1;
        }
      } 
      else 
      {
        obstacleHeight = 1; 
      }
    }
    screenChanged = true;
  }

  // Geschwindigkeit erhöhen 
  if ((now - lastSpeedIncrease >= 1000) && (obstacleX < PLAYER_X)) 
  {
    lastSpeedIncrease = now;
    
    if (currentObstacleSpeed > MAX_SPEED_LIMIT) 
    {
      currentObstacleSpeed -= GESCHWINDIGKEIT_STEIGERUNG;
      
      if (currentObstacleSpeed < MAX_SPEED_LIMIT) 
      {
        currentObstacleSpeed = MAX_SPEED_LIMIT;
      }
    }
  }

  // Kollisionsprüfung
  if (obstacleX == PLAYER_X)
  {
    if (playerLED == 9)
    {
      gameOver();
      return;
    }

    if (playerLED == 17 && obstacleHeight == 2)
    {
      gameOver();
      return;
    }
  }

  if (screenChanged)
  {
    drawGame();
  }
}