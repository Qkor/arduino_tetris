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

byte gameState[8];

// pieces
byte pieceType = 0;
byte currentRotation = 0;
byte posX=0;
byte posY=0;
byte fallingPiece[10];

byte pieces[6][4][3] = {
  //piece 0
  {
    {
      0b11000000,
      0b01100000,
      0b00000000,
    },
    {
      0b01000000,
      0b11000000,
      0b10000000,
    },
    {
      0b11000000,
      0b01100000,
      0b00000000,
    },
    {
      0b01000000,
      0b11000000,
      0b10000000,
    },
  },
  // piece 1
  {
    {
      0b01100000,
      0b11000000,
      0b00000000,
    },
    {
      0b10000000,
      0b11000000,
      0b01000000,
    },  
    {
      0b01100000,
      0b11000000,
      0b00000000,
    },
    {
      0b10000000,
      0b11000000,
      0b01000000,
    },
  },
  // piece 2
  {
    {
      0b01000000,
      0b11100000,
      0b00000000,
    },
    {
      0b01000000,
      0b01100000,
      0b01000000,
    },
    {
      0b00000000,
      0b11100000,
      0b01000000,
    },
    {
      0b01000000,
      0b11000000,
      0b01000000,
    }
  },
  // piece 3
  {
    {
      0b10000000,
      0b11100000,
      0b00000000,
    },
    {
      0b01100000,
      0b01000000,
      0b01000000,
    },
    {
      0b00000000,
      0b11100000,
      0b00100000,
    },
    {
      0b01000000,
      0b01000000,
      0b11000000,
    },
  },
  // piece 4
  {
    {
      0b00100000,
      0b11100000,
      0b00000000,
    },
    {
      0b01000000,
      0b01000000,
      0b01100000,
    },
    {
      0b00000000,
      0b11100000,
      0b10000000,
    },
    {
      0b11000000,
      0b01000000,
      0b01000000,
    },
  },
  // piece 5
  {
    {
      0b11000000,
      0b11000000,
      0b00000000,
    },
    {
      0b11000000,
      0b11000000,
      0b00000000,
    },
    {
      0b11000000,
      0b11000000,
      0b00000000,
    },
    {
      0b11000000,
      0b11000000,
      0b00000000,
    },
  },
};

void rotatePiece(){
  byte nextRotation;
  if(pieceType > 4)
    return;
  if(currentRotation==3) nextRotation = 0;
  else nextRotation = currentRotation+1;

  byte fallingPieceNextState[10] = {0,0,0,0,0,0,0,0,0,0};
  for(int i=0;i<3;i++){
    fallingPieceNextState[posY+i] = pieces[pieceType][nextRotation][i] >> posX;
    // check if rotated piece gets out of the board
    if((fallingPieceNextState[posY+i] << posX) != pieces[pieceType][nextRotation][i])
      return;
  }
  // check collisions
  for(int i=2;i<10;i++){
    if(fallingPieceNextState[i] & gameState[i-2])
      return;
  }
  // update falling piece
  currentRotation = nextRotation;
  for(int i=0;i<10;i++){
    fallingPiece[i] = fallingPieceNextState[i];
  }
}

void startGame(){
  for(int i=0;i<8;i++)
    gameState[i] = 0;
  newPiece();
}

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

void sendCommandToLedMatrix(byte address, byte data) {
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

  // LED board setup
  sendCommandToLedMatrix(0x09, 0x00); 
  sendCommandToLedMatrix(0x0A, 0x00);  
  sendCommandToLedMatrix(0x0B, 0x07);  
  sendCommandToLedMatrix(0x0C, 0x01);  
  sendCommandToLedMatrix(0x0F, 0x00);  

  for (int row = 1; row <= 8; ++row) {
    sendCommandToLedMatrix(row, 0);
  }

  startGame();
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
    gameState[i] |= fallingPiece[i+2];
  }
  removeRows();
}

void newPiece(){
  pieceType = rand()%6;
  posX=3;
  posY=0;
  currentRotation = 0;
  for(int i=0;i<3;i++){
    fallingPiece[i] = pieces[pieceType][currentRotation][i] >> posX;
  }
  for(int i=3;i<=10;i++){
    fallingPiece[i] = 0;
  }
}

void fallOneRow(){
  // check if piece is at the bottom row
  if(fallingPiece[9]){
    mergePiece();
    newPiece();
    return;
  }
  for(int i=2;i<9;i++){
    // check collision
    if(fallingPiece[i] & gameState[i-1]){
      mergePiece();
      newPiece();
      return;
    }
  }
  //move the piece
  posY++;
  for(int i=9;i>0;i--){
    fallingPiece[i] = fallingPiece[i-1];
  }
  fallingPiece[0]=0;
}

void moveRight(){
  //check if moving the piece is possible
  for(int i=0;i<10;i++){
    //out of board
    if(fallingPiece[i]%2)
      return;
    //collision
    if(i>=2)
      if(((fallingPiece[i] >> 1) | gameState[i-2]) != (fallingPiece[i] >> 1) + gameState[i-2])
        return;
  }
  //move the piece
  posX++;
  for(int i=0;i<=9;i++){
    fallingPiece[i] = fallingPiece[i] >> 1;
  }
}

void moveLeft(){
  //check if moving the piece is possible
  for(int i=0;i<10;i++){
    //out of board
    if(fallingPiece[i]>127)
      return;
    //collision
    if(i>=2)
      if(((fallingPiece[i] << 1) | gameState[i-2]) != (fallingPiece[i] << 1) + gameState[i-2])
        return;
  }
  //move the piece
  posX--;
  for(int i=0;i<=9;i++){
    fallingPiece[i] = fallingPiece[i] << 1;
  }
}

void updateGame(){
  // restart game on loss 
  if(gameState[0]){
    startGame();
  }
  if(millis() - time > millisPerUpdate){
    time = millis();
    fallOneRow();
  }
  for (int row = 1; row <= 8; ++row) {
      sendCommandToLedMatrix(row, gameState[row-1] | fallingPiece[row+1]);
  }
}

void loop() {
  getInput();
  updateGame();
}