#include <Adafruit_NeoPixel.h>

#define DATA_PIN 6
#define BUTTON_PIN 2
#define NUM_LEDS 64

Adafruit_NeoPixel leds(NUM_LEDS, DATA_PIN, NEO_GRB + NEO_KHZ800);

// Spieler
const int PLAYER_X = 1;
int playerLED = 9;

// Hindernis
int obstacleX = 7;
int obstacleHeight = 1; // Speichert die aktuelle Höhe des Hindernisses (1 oder 2)

// Sprung
bool jumping = false;
int jumpStep = 0;

unsigned long lastJumpMove = 0;
unsigned long lastObstacleMove = 0;
unsigned long lastSpeedIncrease = 0;
unsigned long gameStartTime = 0; // NEU: Speichert, wann die Runde begonnen hat

const int jumpSpeed = 180;

// --- EINSTELLUNGEN FÜR DIE GESCHWINDIGKEIT ---
const int START_SPEED = 350;         
int currentObstacleSpeed = START_SPEED; 

// Um wie viele Millisekunden das Intervall JEDE SEKUNDE SCHNELLER wird
const int GESCHWINDIGKEIT_STEIGERUNG = 15; 

// Untergrenze für die Geschwindigkeit
const int MAX_SPEED_LIMIT = 50; 

//  EINSTELLUNGEN FÜR WAHRSCHEINLICHKEIT & ZEIT
// Wert in Prozent (0 = nie 2 LEDs hoch, 100 = immer 2 LEDs hoch, 30 = 30% Chance)
const int CHANCE_ZWEI_LEDS = 20; 

//  Zeit in Millisekunden, ab wann große Hindernisse überhaupt auftauchen dürfen (z. B. 7000 = 7 Sekunden)
const unsigned long WARTEZEIT_GROSSE_HINDERNISSE = 10000; 


// ========================================
// BODEN
// ========================================

void drawGround()
{
  for (int i = 0; i < 8; i++)
  {
    leds.setPixelColor(
      i,
      leds.Color(0, 80, 0)
    );
  }
}


// ========================================
// HINDERNIS ZEICHNEN (Jetzt mit dynamischer Höhe)
// ========================================

void drawObstacle()
{
  if (obstacleX >= 0 && obstacleX <= 7)
  {
    // Untere LED des Hindernisses (Y-Reihe 1 = Index + 8)
    leds.setPixelColor(
      obstacleX + 8,
      leds.Color(255, 0, 0)
    );

    // Wenn das Hindernis 2 LEDs hoch ist, zeichne die obere LED (Y-Reihe 2 = Index + 16)
    if (obstacleHeight == 2)
    {
      leds.setPixelColor(
        obstacleX + 16,
        leds.Color(255, 0, 0)
      );
    }
  }
}


// ========================================
// SPIEL ZEICHNEN
// ========================================

void drawGame()
{
  leds.clear();

  drawGround();

  // Spieler
  leds.setPixelColor(
    playerLED,
    leds.Color(0, 0, 255)
  );

  // Hindernis
  drawObstacle();

  leds.show();
}


// ========================================
// GAME OVER
// ========================================

void gameOver()
{
  for (int r = 0; r < 3; r++)
  {
    for (int i = 0; i < NUM_LEDS; i++)
    {
      leds.setPixelColor(
        i,
        leds.Color(255, 0, 0)
      );
    }

    leds.show();
    delay(200);

    leds.clear();
    leds.show();
    delay(200);
  }

  // --- ZURÜCKSETZEN BEIM TOD ---
  playerLED = 9;
  obstacleX = 7;
  obstacleHeight = 1; // Startet nach dem Tod wieder mit einem kleinen Hindernis
  currentObstacleSpeed = START_SPEED; 

  jumping = false;
  jumpStep = 0;
  
  // NEU: Startzeit für die neue Runde zurücksetzen
  gameStartTime = millis(); 

  drawGame();
}


// ========================================
// SETUP
// ========================================

void setup()
{
  pinMode(
    BUTTON_PIN,
    INPUT_PULLUP
  );

  // Zufallsgenerator mit unverbundenem Pin initialisieren für echte Zufallswerte
  randomSeed(analogRead(0)); 

  leds.begin();
  leds.setBrightness(40);

  leds.clear();
  leds.show();

  // Startzeit für die allererste Runde setzen
  gameStartTime = millis(); 

  drawGame();
}


// ========================================
// LOOP
// ========================================

void loop()
{
  unsigned long now = millis();


  // ======================================
  // TASTER
  // ======================================

  if (digitalRead(BUTTON_PIN) == LOW)
  {
    if (!jumping)
    {
      jumping = true;
      jumpStep = 0;

      lastJumpMove = now;
    }
  }


  // ======================================
  // SPRUNG
  // ======================================

  if (jumping)
  {
    if (now - lastJumpMove >= jumpSpeed)
    {
      lastJumpMove = now;

      jumpStep++;

      if (jumpStep == 1)
      {
        playerLED = 17;
      }

      else if (jumpStep == 2)
      {
        playerLED = 25;
      }

      else if (jumpStep == 3)
      {
        // Oben bleiben
        playerLED = 25;
      }

      else if (jumpStep == 4)
      {
        playerLED = 17;
      }

      else if (jumpStep == 5)
      {
        playerLED = 9;

        jumping = false;
        jumpStep = 0;
      }
    }
  }


  // ======================================
  // HINDERNIS BEWEGEN
  // ======================================

  if (now - lastObstacleMove >= currentObstacleSpeed)
  {
    lastObstacleMove = now;

    obstacleX--;

    // Hindernis links raus -> Neues Hindernis vorbereiten
    if (obstacleX < 0)
    {
      obstacleX = 7;
      
      // NEU: Prüfen, ob die Schonfrist für große Hindernisse abgelaufen ist
      if (now - gameStartTime >= WARTEZEIT_GROSSE_HINDERNISSE) 
      {
        // Wenn die Zeit um ist, entscheidet die Prozent-Wahrscheinlichkeit
        if (random(0, 100) < CHANCE_ZWEI_LEDS) 
        {
          obstacleHeight = 2; // Hohes Hindernis
        } 
        else 
        {
          obstacleHeight = 1; // Normales Hindernis
        }
      } 
      else 
      {
        // Solange die Zeit noch NICHT um ist, immer 1 LED hoch
        obstacleHeight = 1; 
      }
    }
  }


  // ======================================
  // GESCHWINDIGKEIT ERHÖHEN (Jetzt nur hinter dem Spieler)
  // ======================================

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


  // ======================================
  // ERWEITERTE KOLLISIONSPRÜFUNG
  // ======================================

  if (obstacleX == PLAYER_X)
  {
    // Fall 1: Spieler ist am Boden (playerLED = 9) -> Kollidiert IMMER mit dem Hindernis
    if (!jumping && playerLED == 9)
    {
      gameOver();
      return;
    }

    // Fall 2: Spieler ist in der Luft auf Stufe 1 (playerLED = 17)
    // Kollidiert nur, wenn das Hindernis 2 LEDs hoch ist!
    if (playerLED == 17 && obstacleHeight == 2)
    {
      gameOver();
      return;
    }

    // Hinweis: Wenn der Spieler ganz oben ist (playerLED = 25), 
    // fliegt er sicher über beide Hindernis-Arten hinweg.
  }


  // ======================================
  // ANZEIGE
  // ======================================

  drawGame();
}