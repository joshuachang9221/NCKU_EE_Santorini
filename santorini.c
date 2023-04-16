#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<assert.h>
#include<time.h>

#define GRID_SIZE 5

typedef enum chess_e {
    BLACK = 1, WHITE = 2,
} Chess;

typedef enum god_e {
    ZEUS, DEMETER, TRITON,
} God;

typedef struct coordinate_s {
    int r, c;
} Coordinate;

const Coordinate delta[9] = {
    {-1, -1}, {-1, 0}, {-1, 1},
    {0, -1}, {0, 0}, {0, 1},
    {1, -1}, {1, 0}, {1, 1},
};

Chess myChess;
God myGod;
God opponentGod;
bool isPlaceWorkerRound;

char *chessColorFileName;
char *chessStructureFileName;
char *stepLogFileName;

int chess[GRID_SIZE][GRID_SIZE];
int structure[GRID_SIZE][GRID_SIZE];

Coordinate chessPositions[2];

void readArgs(char **);
void setGod(God *, const char *);
void initChess();
void initStructure();
void findChessPosition();
void saveChess();
void saveStructure();

Coordinate generateRandomCoordinate();
bool isOutOfRange(Coordinate);

bool canPlaceWorkerAt(Coordinate);
void placeWorkersRandomly(int);
bool canMoveWorker(Coordinate from, Coordinate to);
bool canWorkerEverMove(Coordinate);
void moveWorker(Coordinate from, Coordinate to);
bool canBuildAt(Coordinate);
void buildStructureAt(Coordinate);

Coordinate addCoordinate(Coordinate, Coordinate);

int main(int argc, char **argv) {

    srand(time(NULL));

    readArgs(argv);
    initChess();
    initStructure();

    if (isPlaceWorkerRound) {
        placeWorkersRandomly(2);
        saveChess();
        saveStructure();
        return 0;
    }

    findChessPosition();

    /* move randomly */ 
    int randIndex = rand()%2;
    Coordinate originPos = chessPositions[randIndex];
    if (!canWorkerEverMove(originPos)) {
        originPos = chessPositions[!randIndex];
    }
    Coordinate deltaPos = delta[rand()%9];
    Coordinate targetPos = addCoordinate(originPos, deltaPos);
    while (!canMoveWorker(originPos, targetPos)) {
        deltaPos = delta[rand()%9];
        targetPos = addCoordinate(originPos, deltaPos);
    }
    moveWorker(originPos, targetPos);

    /* build randomly */ 
    deltaPos = delta[rand()%9];
    Coordinate buildPos = addCoordinate(targetPos, deltaPos);
    while (!canBuildAt(buildPos)) {
        deltaPos = delta[rand()%9];
        buildPos = addCoordinate(targetPos, deltaPos);
    }
    buildStructureAt(buildPos);

    saveChess();
    saveStructure();

    return 0;
}


/*
0./[學號].out 1[Color] 2[Gods] 3[OpponentGods] 4[PlaceWorkers] 
5[chessColor.txt] 6[chessStructure.txt] 7[stepLog.txt]
*/
void readArgs(char **argv) {
    myChess = atoi(argv[1]);
    setGod(&myGod, argv[2]);
    setGod(&opponentGod, argv[3]);
    isPlaceWorkerRound = (argv[4][0] == 'Y' ? true : false);
    chessColorFileName = argv[5];
    chessStructureFileName = argv[6];
    stepLogFileName = argv[7];
}

void setGod(God *god, const char *str) {
    switch (str[0]) {
        case 'Z': *god = ZEUS; break;
        case 'D': *god = DEMETER; break;
        case 'T': *god = TRITON; break;
    }
}

void initChess() {
    FILE *file = fopen(chessColorFileName, "r");
    assert(file != NULL);

    int i;
    for (i = 0;i < GRID_SIZE;i++) {
        fscanf(file, "%1d,%1d,%1d,%1d,%1d", &chess[i][0], &chess[i][1], &chess[i][2], &chess[i][3], &chess[i][4]);
    }

    fclose(file);
}

void initStructure() {
    FILE *file = fopen(chessStructureFileName, "r");
    assert(file != NULL);

    int i;
    for (i = 0;i < GRID_SIZE;i++) {
        fscanf(file, "%1d,%1d,%1d,%1d,%1d", &structure[i][0], &structure[i][1], &structure[i][2], &structure[i][3], &structure[i][4]);
    }

    fclose(file);
}

void findChessPosition() {
    int idx = 0;
    int i, j;
    for (i = 0;i < GRID_SIZE;i++) {
        for (j = 0;j < GRID_SIZE;j++) {
            if (chess[i][j] == myChess) {
                chessPositions[idx].r = i;
                chessPositions[idx].c = j;
                idx++;
            }
        }
    }
}

void saveChess() {
    FILE *file = fopen(chessColorFileName, "w");
    assert(file != NULL);

    int i, j;
    for (i = 0;i < GRID_SIZE;i++) {
        for (j = 0;j < GRID_SIZE;j++) {
            fprintf(file, "%d%c", chess[i][j], (j == GRID_SIZE-1 ? '\n' : ','));
        }
    }

    fclose(file);
}

void saveStructure() {
    FILE *file = fopen(chessStructureFileName, "w");
    assert(file != NULL);

    int i, j;
    for (i = 0;i < GRID_SIZE;i++) {
        for (j = 0;j < GRID_SIZE;j++) {
            fprintf(file, "%d%c", structure[i][j], (j == GRID_SIZE-1 ? '\n' : ','));
        }
    }

    fclose(file);
}

Coordinate generateRandomCoordinate() {
    Coordinate coord;
    coord.r = rand()%GRID_SIZE;
    coord.c = rand()%GRID_SIZE;
    return coord;
}

bool isOutOfRange(Coordinate pos) {
    return !(0 <= pos.r && pos.r < GRID_SIZE && 0 <= pos.c && pos.c < GRID_SIZE);
}

bool canPlaceWorkerAt(Coordinate pos) {
    if (isOutOfRange(pos)) {
        return false;
    }
    if (chess[pos.r][pos.c] != 0) {
        return false;
    }
    return true;
}

void placeWorkersRandomly(int num) {
    while (num--) {
        Coordinate coord = generateRandomCoordinate();
        while (!canPlaceWorkerAt(coord)) {
            coord = generateRandomCoordinate();
        }
        chess[coord.r][coord.c] = myChess;
    }
}

bool canMoveWorker(Coordinate from, Coordinate to) {
    if (isOutOfRange(to)) {
        return false;
    }
    if (chess[to.r][to.c] != 0) {
        return false;
    }
    if (structure[to.r][to.c] == 4) {
        return false;
    }
    if (structure[to.r][to.c] - structure[from.r][from.c] > 1) {
        return false;
    }
    return true;
}

bool canWorkerEverMove(Coordinate pos) {
    int i;
    for (i = 0;i < 9;i++) {
        if (canMoveWorker(pos, addCoordinate(pos, delta[i]))) {
            return true;
        }
    }
    return false;
}

void moveWorker(Coordinate from, Coordinate to) {
    chess[from.r][from.c] = 0;
    chess[to.r][to.c] = myChess;
    /* printf("Move %d from (%d,%d) to (%d,%d)\n", myChess, from.r, from.c, to.r, to.c); */
}

bool canBuildAt(Coordinate pos) {
    if (isOutOfRange(pos)) {
        return false;
    }
    if (chess[pos.r][pos.c] != 0) {
        return false;
    }
    if (structure[pos.r][pos.c] == 4) {
        return false;
    }
    return true;
}

void buildStructureAt(Coordinate pos) {
    structure[pos.r][pos.c]++;
    /* printf("Build at (%d,%d)\n", pos.r, pos.c); */
}

Coordinate addCoordinate(Coordinate a, Coordinate b) {
    Coordinate c;
    c.r = a.r + b.r;
    c.c = a.c + b.c;
    return c;
}
