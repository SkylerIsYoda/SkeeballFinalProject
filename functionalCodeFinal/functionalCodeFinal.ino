
//seven segment display library
#include <SevSeg.h>

//create object for the 4-digit display
SevSeg sevseg;

//initialize scoreboard
int totalPoints = 0;

//initialize ball count
int ballCount = 0;

//initialize ball counter pins for segments
int a = 38; 
int b = 39;
int c = 40;
int d = 41;
int e = 42;
int f = 43;
int g = 44;

//photoresistor array for pins and scoring
int photoPins[] = { 0, 1, 2, 3, 4, 5 };
int points[] = { 1000, 1000, 500, 250, 100, 0 };  

//array to hold "isCrossed" state for each photoresistor
bool isCrossed[6] = { false, false, false, false, false, false };

//reset button vars
int resetPin = 13;
bool prevReading = false;

//on/off button vars
int powerPin = 28;
bool prevPower = false;
bool currentPower = false;
int powerDebounce = 0;

//time between cross and when to read
unsigned long prevTime[6] = { 0, 0, 0, 0, 0, 0 };
unsigned long interval = 50; 

//state var
int state = 0;
bool gameStarted = false;

void setup() {
  Serial.begin(9600);

  //segment pin initialization
  pinMode(a, OUTPUT);
  pinMode(b, OUTPUT);
  pinMode(c, OUTPUT);
  pinMode(d, OUTPUT);
  pinMode(e, OUTPUT);
  pinMode(f, OUTPUT);
  pinMode(g, OUTPUT);

  //initialize score display
  byte numDigits = 4;
  byte digitPins[] = { 2, 3, 4, 5 };
  byte segmentPins[] = { 6, 7, 8, 9, 10, 11, 12 };
  sevseg.begin(COMMON_CATHODE, numDigits, digitPins, segmentPins);
  sevseg.setBrightness(90);

  //initialize reset button
  pinMode(resetPin, INPUT);
  pinMode(powerPin, INPUT);
}

void loop() {
  handlePowerButton();
  stateMachine();
}

void stateMachine() {
  Serial.println(state);

  switch (state) {
    case 0:
      turnOffAllSegments();
      break;
    case 1:
      handleGameState();
      break;
  }
}

void handlePowerButton() {
  currentPower = digitalRead(powerPin);

  if (currentPower && !prevPower && millis() - powerDebounce > 200) {
    if (state == 0) {
      totalPoints = 0;
      ballCount = 0;
      sevseg.setNumber(0, 0);
      sevseg.refreshDisplay();
      state = 1;
      gameStarted = true;
    } else {
      state = 0;
      turnOffAllSegments();
      gameStarted = false;
    }
    powerDebounce = millis();
  }
  prevPower = currentPower;
}

void handleGameState() {

  if (gameStarted) {
    totalPoints = 0;
    ballCount = 0;
    sevseg.setNumber(0, 0);
    sevseg.refreshDisplay();
    gameStarted = false;
  }

  //handle points for all photo pins
  for (int i = 0; i < 6; i++) {
    handlePoints(photoPins[i], points[i], i);
  }

  //handle reset button
  handleResetButton();

  //display the score on the 4-digit display
  sevseg.setNumber(totalPoints, 0);
  sevseg.refreshDisplay();

  //display the ball count on the 1-digit display
  displayNumber(ballCount);
}


void turnOffAllSegments() {

  //turn off the 1-digit display
  digitalWrite(a, LOW);
  digitalWrite(b, LOW);
  digitalWrite(c, LOW);
  digitalWrite(d, LOW);
  digitalWrite(e, LOW);
  digitalWrite(f, LOW);
  digitalWrite(g, LOW);

  //clear 4-digit display
  sevseg.blank();
}

void displayNumber(int num) {
  //reset ball count at 9
  if (num > 9) {
    num = 0;
  }

  //segment patterns for 0-9
  byte segmentPatterns[10] = {
    0b00111111,  // 0
    0b00000110,  // 1
    0b01011011,  // 2
    0b01001111,  // 3
    0b01100110,  // 4
    0b01101101,  // 5
    0b01111101,  // 6
    0b00000111,  // 7
    0b01111111,  // 8
    0b01101111   // 9
  };

  byte pattern = segmentPatterns[num];

  //set each segment according to the pattern
  if (pattern & 0b00000001) {
    digitalWrite(a, HIGH);
  } else {
    digitalWrite(a, LOW);
  }

  if (pattern & 0b00000010) {
    digitalWrite(b, HIGH);
  } else {
    digitalWrite(b, LOW);
  }

  if (pattern & 0b00000100) {
    digitalWrite(c, HIGH);
  } else {
    digitalWrite(c, LOW);
  }

  if (pattern & 0b00001000) {
    digitalWrite(d, HIGH);
  } else {
    digitalWrite(d, LOW);
  }

  if (pattern & 0b00010000) {
    digitalWrite(e, HIGH);
  } else {
    digitalWrite(e, LOW);
  }

  if (pattern & 0b00100000) {
    digitalWrite(f, HIGH);
  } else {
    digitalWrite(f, LOW);
  }

  if (pattern & 0b01000000) {
    digitalWrite(g, HIGH);
  } else {
    digitalWrite(g, LOW);
  }
}

void handlePoints(int photoPin, int points, int index) {
  // read time
  unsigned long currentTime = millis();
  int photoRead;

  if (state == 0) return;

  if (currentTime - prevTime[index] >= interval) {
    // reset timer for this photo pin
    prevTime[index] = currentTime;

    // read sensor value
    photoRead = analogRead(photoPin);

    // determine if sensor is crossed
    if (photoRead <= 50 && !isCrossed[index]) {
      // add points and ball to displays
      isCrossed[index] = true;
      totalPoints += points;
      ballCount++;

      // reset ball count to 0 after reaching 9
      if (ballCount > 9) {
        ballCount = 0;
        totalPoints = 0;
      }
    }

    // check if the sensor is no longer triggered
    if (photoRead > 50) {
      isCrossed[index] = false;
    }
  }
}

void handleResetButton() {
  bool currentReading = digitalRead(resetPin);

  //check if the reset button is pressed
  if (currentReading && !prevReading) {
    //reset score and ball count
    totalPoints = 0;
    ballCount = 0;
    Serial.println("Score Reset");

    turnOffAllSegments();
  }

  //reset button state
  prevReading = currentReading;
}


/*
Within this code, I had aid from my good friend ChatGPT. I had help with the following code:

I asked for help with the one-digit seven segment display, as I could not use the library for both at the same time.
I added in the ball reset to the function and used what I was given since I had not had to use the display before.

Prompt: How would I write numbers 0-9 on a 7 segment, 1-digit display and cycle when a sensor is crossed?
Answer: 

void displayNumber(int num) {

  //segment patterns for 0-9
  byte segmentPatterns[10] = {
    0b00111111,  // 0
    0b00000110,  // 1
    0b01011011,  // 2
    0b01001111,  // 3
    0b01100110,  // 4
    0b01101101,  // 5
    0b01111101,  // 6
    0b00000111,  // 7
    0b01111111,  // 8
    0b01101111   // 9
  };

  byte pattern = segmentPatterns[num];

  //set each segment according to the pattern
  if (pattern & 0b00000001) {
    digitalWrite(a, HIGH);
  } else {
    digitalWrite(a, LOW);
  }

  if (pattern & 0b00000010) {
    digitalWrite(b, HIGH);
  } else {
    digitalWrite(b, LOW);
  }

  if (pattern & 0b00000100) {
    digitalWrite(c, HIGH);
  } else {
    digitalWrite(c, LOW);
  }

  if (pattern & 0b00001000) {
    digitalWrite(d, HIGH);
  } else {
    digitalWrite(d, LOW);
  }

  if (pattern & 0b00010000) {
    digitalWrite(e, HIGH);
  } else {
    digitalWrite(e, LOW);
  }

  if (pattern & 0b00100000) {
    digitalWrite(f, HIGH);
  } else {
    digitalWrite(f, LOW);
  }

  if (pattern & 0b01000000) {
    digitalWrite(g, HIGH);
  } else {
    digitalWrite(g, LOW);
  }
}

I also had help initializing the 4-digit display for score
Prompt: How do I initialize a 7-segment, 4-digit display?
Answer (something along the lines):
  //initialize score display
  byte numDigits = 4;
  byte digitPins[] = { 2, 3, 4, 5 };
  byte segmentPins[] = { 6, 7, 8, 9, 10, 11, 12 };
  sevseg.begin(COMMON_CATHODE, numDigits, digitPins, segmentPins);
  sevseg.setBrightness(90);

I used the library online to understand the built in functions within the library

There is still some debugging to do but this is the code so far! I still have to add one for 4-digit display and one more button
*/
