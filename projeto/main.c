#include "timer.h"
#include "screen.h"
#include "keyboard.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdbool.h>

#define halfw 15
#define halfh 10
#define exit 100

#define wall 5
#define block 1

#define up 72
#define left 75
#define right 77
#define down 80
#define space 32
#define esc 27

int width = 25;
int height = 20;

bool paused = false;

typedef struct currentlocation {
  int X;
  int Y;
} location;

void keyboardInit();

void timerInit(int valueMilliSec);

void screenscreenGotoxy(int x, int y);

void hidecursor() { printf("\e[?25l"); }

int** map;

int kbhit() {
  struct termios oldt, newt;
  int ch;
  int oldf;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if (ch != EOF) {
    ungetc(ch, stdin);
    return 1;
  }

  return 0;
}
int getKeyDown() {
  if (kbhit()) {
    return getchar();
  } else {
    return -1;
  }
}

void drawWall(int map[height][width]) {
  int heig, wid;

  for (heig = 0; heig <= height + 1; heig++) {
    for (wid = 0; wid <= width + 1; wid++) {
      screenGotoxy(wid + 1, heig + 1);
      if (heig == height + 1 || heig == 0 || (wid == 0 || wid == width + 1))
        printf("■");
      else
        printf(" ");
    }
    printf("\n");
  }

  screenGotoxy(halfw, halfh + 12);
  printf("<Exit : 'esc' / Pause : 'p'>");
}

void drawMap(int map[height][width]) {
  int h, w;

  for (h = 0; h < height; h++) {
    for (w = 0; w < width; w++) {
      screenGotoxy(w + 2, h + 2);
      if (map[h][w] == 0) {
        printf("·");
      } else if (map[h][w] == block) {
        printf("#");
      }
    }
    printf("\n");
  }
}

int drawFrontMenu() {
  int key;
  screenGotoxy(1, 2);
  printf("=====================================================");
  screenGotoxy(1, 3);
  printf("=================== T E T R I S =====================");
  screenGotoxy(1, 4);
  printf("=====================================================\n");

  printf("Sair: 'esc' \n");
  printf("Pausar: 'p' \n");

  while (1) {
    key = getKeyDown();
    timerInit(1000000);
    if (key == 's' || key == 'S') {
      system("clear");

      break;
    }

    if (key == esc)
      break;

    screenGotoxy(7, 15);
    printf(" === press 's' to start ===");
  }

  return key;
}

void drawSubShape(int shape[4][4]) {
  int h, w;

  for (h = 3; h <= 6; h++) {
    for (w = halfw + 1; w <= halfw + 4; w++) {
      screenGotoxy(w, h);
      printf(" ");
    }
  }

  for (h = 3; h <= 6; h++) {
    for (w = halfw + 1; w <= halfw + 4; w++) {
      if (shape[h - 3][w - halfw - 1] == block) {
        screenGotoxy(w, h);
        printf("■");  
      }
    }
  }
}


void drawShape(int map[height][width], int shape[4][4],
               location curLoc) {
  int h, w;

  for (h = 0; h < 4; h++) {
    for (w = 0; w < 4; w++) {
      if (shape[h][w] == block) {
        map[curLoc.Y + h][curLoc.X + w] = block;
      }
    }
  }
}
void drawTetrisShape(int map[height][width], int blockShape[4][4], location curLoc) {
  int i, j;
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      if (blockShape[i][j] == block) {
        map[curLoc.Y + i][curLoc.X + j] = block;
      }
    }
  }
}

void startTime() {
  int i;

  for (i = 0; i < 3; i++) {
    screenGotoxy(2, 0);
    printf("Start : %d sec", 3 - i);
    timerInit(5000);
  }
  screenGotoxy(2, 0);
  printf("                       ");
}

int** createMap(int height, int width) {
    int** newMap = (int**)malloc(height * sizeof(int*));
    for (int i = 0; i < height; i++) {
        newMap[i] = (int*)malloc(width * sizeof(int));
        for (int j = 0; j < width; j++) {
            newMap[i][j] = 0;
        }
    }
    return newMap;
}

void freeMap(int** map, int height) {
    for (int i = 0; i < height; i++) {
        free(map[i]);
    }
    free(map);
}


void locationInit(location *curLoc) {
  curLoc->X = 3;
  curLoc->Y = 0;
}

void setBlock(int blockShape[4][4]) {
  int shape[7][4][4] = {
      {{0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 0, 0}},
      {{0, 0, 0, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}},
      {{0, 0, 0, 0}, {0, 1, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}},
      {{0, 0, 1, 0}, {0, 1, 1, 0}, {0, 1, 0, 0}, {0, 0, 0, 0}},
      {{0, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 1, 0}, {0, 0, 0, 0}},
      {{0, 0, 0, 0}, {0, 1, 0, 0}, {0, 1, 1, 1}, {0, 0, 0, 0}},
      {{0, 0, 0, 0}, {0, 0, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}},
  };
  
  int i, j;
  
      int shapeIndex = rand() % 7;  

      for (i = 0; i < 4; i++) {
          for (j = 0; j < 4; j++) {
              blockShape[i][j] = shape[shapeIndex][i][j];
          }
      }
  }

void moveBlock(int map[height][width], int blockShape[4][4],
               location *curLoc, int input) {
  int i, j;

  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      if (blockShape[i][j] == block) {
        map[curLoc->Y + i][curLoc->X + j] = 0;
      }
    }
  }

  switch (input) {
  case up:
    curLoc->Y -= 1;
    break;
  case down:
    curLoc->Y += 1;
    break;
  case left:
    curLoc->X -= 1;
    break;
  case right:
    curLoc->X += 1;
    break;
  }

  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      if (blockShape[i][j] == block) {
        map[curLoc->Y + i][curLoc->X + j] = block;
      }
    }
  }
}
void rotateBlock(int blockShape[4][4]) {
    int i, j;
    int tmp[4][4];

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            tmp[i][j] = blockShape[i][j];
        }
    }

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            blockShape[j][3-i] = tmp[i][j];
        }
    }
}


int checkCollision(int map[height][width], int blockShape[4][4],
                   location curLoc) {
  int i, j;

  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      if (blockShape[i][j] == block && map[curLoc.Y + i][curLoc.X + j] == block)
        return 1;
    }
  }
  return 0;
}

int gameover(int map[height][width]) {
  int i;
  for (i = 0; i < width; i++) {
    if (map[0][i] == block)
      return 1;
  }
  return 0;
}

int checkLine(int map[height][width]) {
  int i, j;
  int cont = 0;

  for (i = height - 1; i >= 0; i--) {
    int isLine = 1;
    for (j = 0; j < width; j++) {
      if (map[i][j] == 0) {
        isLine = 0;
        break;
      }
    }

    if (isLine) {
      cont++;

      for (j = 0; j < width; j++) {
        map[i][j] = 0;
      }

      for (j = i; j >= 1; j--) {
        for (int k = 0; k < width; k++) {
          map[j][k] = map[j - 1][k];
        }
      }
    }
  }

  return cont;
}

void togglePause() {
    paused = !paused;
}

void drawPauseMenu() {
    screenGotoxy(1, 20);
    printf("                       ");
    screenGotoxy(1, 21);
    printf(" ====================================================");
    screenGotoxy(1, 22);
    printf("======================= P A U S E D =====================");
    screenGotoxy(1, 23);
    printf(" ====================================================");
    screenGotoxy(1, 24);
    printf("Press 'p' to resume");
}
void clearPauseMenu() {
    screenGotoxy(1, 20);
    printf("                       ");
    screenGotoxy(1, 21);
    printf("                       ");
    screenGotoxy(1, 22);
    printf("                       ");
    screenGotoxy(1, 23);
    printf("                       ");
    screenGotoxy(1, 24);
    printf("                       ");
}


int main() {
    system("clear");
    int userChoice;
    int blockShape[4][4];
    location curLoc;
    hidecursor();
    int counter = 0;
    int counterLimit = 1000;
    int speed = 10000;

    map = createMap(height, width);

    userChoice = drawFrontMenu();

    while (1) {
        userChoice = drawFrontMenu();

        if (userChoice == 't' || userChoice == 'T') {
            break;
        }
        locationInit(&curLoc);
        setBlock(blockShape);

        startTime();

        while (userChoice != 't' && userChoice != 'T') {
            if (!paused) {
                system("clear");
                drawWall(map);
                drawMap(map);
                drawShape(map, blockShape, curLoc);

                int key = getKeyDown();
                if (key == up) {
                    rotateBlock(blockShape);
                    if (checkCollision(map, blockShape, curLoc)) {
                        rotateBlock(blockShape);
                        rotateBlock(blockShape);
                        rotateBlock(blockShape);
                    }
                }
                if (key == left || key == right) {
                    moveBlock(map, blockShape, &curLoc, key);
                    if (checkCollision(map, blockShape, curLoc)) {
                        moveBlock(map, blockShape, &curLoc, -key);
                    }
                }

                if (key == space) {
                    while (!checkCollision(map, blockShape, curLoc)) {
                        moveBlock(map, blockShape, &curLoc, down);
                    }
                    moveBlock(map, blockShape, &curLoc, -down);
                }

                if (key == esc) {
                    break;
                }

                if (key == 'p' || key == 'P') {
                    togglePause();
                    if (paused) {
                        drawPauseMenu();
                    } else {
                        clearPauseMenu();
                    }
                }

                if (counter >= counterLimit) {
                    moveBlock(map, blockShape, &curLoc, down);
                    if (checkCollision(map, blockShape, curLoc)) {
                        moveBlock(map, blockShape, &curLoc, -down);
                        checkLine(map);
                        setBlock(blockShape);
                        locationInit(&curLoc);
                        if (gameover(map)) {
                            userChoice = 't';
                            break;
                        }
                    }
                    counter = 0;
                }

                counter++;
                timerInit(1000);
            } else {
                int key = getKeyDown();
                if (key == 'p' || key == 'P') {
                    togglePause();
                    clearPauseMenu();
                }
            }
        }

        screenGotoxy(1, 20);
        printf("                       ");
        screenGotoxy(1, 21);
        printf(" ====================================================");
        screenGotoxy(1, 22);
        printf("===================== G A M E  O V E R ====================");
        screenGotoxy(1, 23);
        printf(" ====================================================");
        screenGotoxy(1, 24);
        printf("                            ");
      
        freeMap(map, height);
    }

    return 0;
}

