#include<SoftwareSerial.h>

// led matrix pins

const int DIN_PIN = 10;   // Data In
const int CLK_PIN = 12;   // Clock
const int LOAD_PIN = 11;  // Load (CS)

// button pins

const int BTN1 = 2;
const int BTN2 = 3;
const int BTN3 = 4;
const int BTN4 = 5;

int btn1lastState = 1;
int btn2lastState = 1;
int btn3lastState = 1;
int btn4lastState = 1;

// game time

unsigned long time = 0;
int millisPerUpdate = 1500;


byte gameState[8] = {
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
};

byte fallingPiece[8] = {
  0b00100000,
  0b00110000,
  0b00100000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000,
};


void getInput(){
    if(digitalRead(BTN1) == LOW){
    if(btn1lastState){
      moveRight();
    }
    btn1lastState = 0;
  } else {
    btn1lastState = 1;
  }
  if(digitalRead(BTN2) == LOW){
    if(btn2lastState){
      rotatePiece();
    }
    btn2lastState = 0;
  } else {
    btn2lastState = 1;
  }
  if(digitalRead(BTN3) == LOW){
    if(btn3lastState){
      fallOneRow();
    }
    btn3lastState = 0;
  } else {
    btn3lastState = 1;
  }
  if(digitalRead(BTN4) == LOW){
    if(btn4lastState){
      moveLeft();
    }
    btn4lastState = 0;
  } else {
    btn4lastState = 1;
  }
}

void sendCommand(byte address, byte data) {
  digitalWrite(LOAD_PIN, LOW);
  shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, address);
  shiftOut(DIN_PIN, CLK_PIN, MSBFIRST, data);
  digitalWrite(LOAD_PIN, HIGH);
}


void setup() {
  pinMode(BTN1, INPUT_PULLUP);
  pinMode(BTN2, INPUT_PULLUP);
  pinMode(BTN3, INPUT_PULLUP);
  pinMode(BTN4, INPUT_PULLUP);

  pinMode(DIN_PIN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);
  pinMode(LOAD_PIN, OUTPUT);

  sendCommand(0x09, 0x00);  // Decode Mode: No decoding
  sendCommand(0x0A, 0x00);  // Intensity: Set brightness (0x00 to 0x0F)
  sendCommand(0x0B, 0x07);  // Scan Limit: Display digits 0 to 7
  sendCommand(0x0C, 0x01);  // Shutdown: Normal operation
  sendCommand(0x0F, 0x00);  // Display Test: Disable test mode

  Serial.begin(9600);
  for (int row = 1; row <= 8; ++row) {
    sendCommand(row, 0);
  }
}

void removeRows(){
  for(int i=0;i<8;i++){
    if(gameState[i]==255){
      for(int j=i;j>0;j--){
        gameState[j] = gameState[j-1];
      }
      gameState[0] = 0;
    }
  }
}

void mergePiece(){
  for(int i=0;i<8;i++){
    gameState[i] |= fallingPiece[i];
  }
  removeRows();
}

void newPiece(){
  fallingPiece[0] = 0b00110000;
  fallingPiece[1] = 0b00110000;
  fallingPiece[2] = 0b00000000;
  fallingPiece[3] = 0b00000000;
  fallingPiece[4] = 0b00000000;
  fallingPiece[5] = 0b00000000;
  fallingPiece[6] = 0b00000000;
  fallingPiece[7] = 0b00000000;
}

void fallOneRow(){
  for(int i=0;i<7;i++){
    // check collision
    if(fallingPiece[7] || (fallingPiece[i-1] & gameState[i])){
      mergePiece();
      newPiece();
      return;
    }
  }
  for(int i=7;i>0;i--){
    fallingPiece[i] = fallingPiece[i-1];
  }
  fallingPiece[0]=0;
}

void moveRight(){
  //check if moving the piece is possible
  for(int i=0;i<8;i++){
    //out of board
    if(fallingPiece[i]%2)
      return;
    //collision
    if(((fallingPiece[i] >> 1) | gameState[i]) != (fallingPiece[i] >> 1) + gameState[i])
      return;
  }
  //move the piece
  for(int i=0;i<8;i++){
    fallingPiece[i] = fallingPiece[i] >> 1;
  }
}

void moveLeft(){
  //check if moving the piece is possible
  for(int i=0;i<8;i++){
    //out of board
    if(fallingPiece[i]>127)
      return;
    //collision
    if(((fallingPiece[i] << 1) | gameState[i]) != (fallingPiece[i] << 1) + gameState[i])
      return;
  }
  //move the piece
  for(int i=0;i<8;i++){
    fallingPiece[i] = fallingPiece[i] << 1;
  }
}

void rotatePiece(){
  
}

void nextGameState(){
  fallOneRow();
}

void updateGame(){
  if(millis() - time > millisPerUpdate){
    time = millis();
    nextGameState();
  }
  for (int row = 1; row <= 8; ++row) {
      sendCommand(row, gameState[row-1] | fallingPiece[row-1]);
  }
}

void loop() {
  getInput();  
  updateGame();
}