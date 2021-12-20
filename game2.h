#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include <MD_MAX72xx.h>

struct Tetromino {
  enum Kind { NONE, TET_I, TET_O, TET_T, TET_J, TET_L, TET_S, TET_Z };
  Kind kind;
  uint8_t x, y, rot;
  bool falling;
  
  // every fixed tetromino possibility including rotations
  // rotations can be done logically but this way is simpler
  void bitboard(uint8_t out_board[4]) {
    switch (kind) {
    case TET_I:
      if (rot == 1 || rot == 3) out_board[1] = 0b00001111;
      else {
        out_board[0] = 0b00000100;
        out_board[1] = 0b00000100;
        out_board[2] = 0b00000100;
        out_board[3] = 0b00000100;
      }
      break;
    case TET_O:
      out_board[0] = 0b00000110;
      out_board[1] = 0b00000110;
      break;
    case TET_T:
      switch (rot) {
      case 0:
        out_board[1] = 0b00000111;
        out_board[2] = 0b00000010;
        break;
      case 1:
        out_board[0] = 0b00000010;
        out_board[1] = 0b00000110;
        out_board[2] = 0b00000010;
        break;
      case 2:
        out_board[0] = 0b00000010;
        out_board[1] = 0b00000111;
        break;
      case 3: 
        out_board[0] = 0b00000010;
        out_board[1] = 0b00000011;
        out_board[2] = 0b00000010;
        break;
      }
      break;
    case TET_J:
      if (rot == 0 || rot == 2) out_board[1] = 0b00000111; 
      else out_board[1] = 0b00000010;
      
      switch (rot) {
      case 0: out_board[2] = 0b00000001; break;
      case 2: out_board[0] = 0b00000100; break;
      case 1: 
        out_board[0] = 0b00000011;
        out_board[2] = 0b00000010;
        break;
      case 3: 
        out_board[0] = 0b00000010;
        out_board[2] = 0b00000110;
        break; 
      }
      break;
    case TET_L:
      if (rot == 0 || rot == 2) out_board[1] = 0b00000111; 
      else out_board[1] = 0b00000010;
      
      switch (rot) {
      case 0: out_board[0] = 0b00000001; break;
      case 2: out_board[2] = 0b00000100; break;
      case 1: 
        out_board[0] = 0b00000110;
        out_board[2] = 0b00000010;
        break;
      case 3: 
        out_board[0] = 0b00000010;
        out_board[2] = 0b00000011;
        break;
      }
      break;
    case TET_S:
      if (rot == 0 || rot == 2) {
        out_board[1] = 0b00000011;
        out_board[2] = 0b00000110;
      } else {
        out_board[0] = 0b00000100;
        out_board[1] = 0b00000110;
        out_board[2] = 0b00000010;
      }
      break;
    case TET_Z:
      if (rot == 0 || rot == 2) {
        out_board[1] = 0b00000110;
        out_board[2] = 0b00000011;
      } else {
        out_board[0] = 0b00000010;
        out_board[1] = 0b00000110;
        out_board[2] = 0b00000100;
      }
      break;
    }
  }
};


#define MD_72XX_HEIGHT 8          // Height and Width of the MD_72XX LED display
#define TETROMINO_MAX 64          // Maximum tetrominoes allowed on the board, 8x8 is obviously enough
uint8_t curr_mino = 0;            // Current tetromino

bool update_move = false;         // Whether current tetromino should be updated

Tetromino minoes[TETROMINO_MAX]; 
uint8_t board[MD_72XX_HEIGHT];    // Bitboard
MD_MAX72XX* Mx;
HardwareSerial* Glob_Serial;

void game_setup(MD_MAX72XX* mx, HardwareSerial* serial) {
  Mx = mx;
  Glob_Serial = serial;
  for (uint8_t i = 0; i < MD_72XX_HEIGHT; i++)
    board[i] = 0b00000000;
  
  for (uint8_t i = 0; i < TETROMINO_MAX; i++) {
    Tetromino mino = { .kind = Tetromino::NONE, .x = 0, .y = 0, .rot = 0, .falling = true };
    minoes[i] = mino;
  }
}

// draw the board
void game_draw(MD_MAX72XX* mx) {
  for (uint8_t i = 0; i < MD_72XX_HEIGHT; i++) {
    mx->setColumn(i, board[i]);
  }
}

// clear the screen
void game_clear(MD_MAX72XX* mx) {
  for (uint8_t i = 0; i < MD_72XX_HEIGHT; i++) {
    mx->setColumn(i, 0);
    board[i] = 0;
  }
}

#define MAX_DEVICES 4
#define GAME_FPS 2
#define MOVE_RIGHT_PIN 7

void game_tick(unsigned long delta) {
  while (millis() < delta + (1000 / GAME_FPS)) {
    if (digitalRead(MOVE_RIGHT_PIN)) {
      minoes[curr_mino].x++;
      update_move = true;
      delay(500 / GAME_FPS);
      break;
    }
  }
  for (uint8_t i = 0; i < TETROMINO_MAX; i++) {
    if (minoes[i].kind == Tetromino::NONE) {
      if (i == curr_mino) minoes[i].kind = Tetromino::Kind(1 + (curr_mino % 6));
      else continue;
    }
    
    // convert tetromino into bitboard
    uint8_t mino_2d[4] = {0}; // empty bitboard buffer
    minoes[i].bitboard(mino_2d);   // mino_2d as the input and output of the function
  
    // write to board
    for (uint8_t j = 0; j < 4; j++) {
      // this is the offset of the tetromino in terms of the actual game board
      uint8_t yoffs = j + minoes[i].y;

      // if the bitboard row is empty, we break out of the loop
      // due to there being no tetrominoes with empty space in between pixels
      // the I shape is a special case as it is 4 long, so we cant check for an empty row
      if (minoes[i].kind == Tetromino::TET_I && j == 3 && yoffs >= MD_72XX_HEIGHT-1) {
        minoes[i].falling = false; 
        board[yoffs] = board[yoffs] | (mino_2d[j] << minoes[i].x);
        break;
      }
      if (j > 1 && mino_2d[j] == 0) {
        // if the bottom of the tetromino is greater than the last row,
        // we make it stop falling
        if (yoffs >= MD_72XX_HEIGHT) minoes[i].falling = false;
        break;
      }
      if (minoes[i].falling && board[yoffs+1] & mino_2d[j]) {
        minoes[i].falling = false;
      }
      
      
      // add 2d bitboard tetromino grid to board
      board[yoffs] = board[yoffs] | (mino_2d[j] << minoes[i].x);

    }
    
    // if an update is detected
    if (update_move) {
      update_move = false;
      continue;
    }

    // make the tetromino fall
    if (minoes[i].falling) minoes[i].y++;
    else if (i == curr_mino) {
      curr_mino++;
      delay(2000);
    }
  }
  
  // draw screen then clear board
  for (uint8_t i = 0; i < MD_72XX_HEIGHT; i++) {
    Mx->setColumn(i, board[i]);
    board[i] = 0;
  }
  
}


#endif