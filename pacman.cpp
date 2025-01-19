#include <iostream>
#include <fstream>
#include <windows.h>
#include <cmath>

bool gameOver = false;

const char pacManChar = 'Y';
const char blinkyChar = 'B';
const char pinkyChar = 'P';
const char inkyChar = 'I';
const char clydeChar = 'C';

// Cage exit coordinates
const int CAGE_EXIT_ROW = 7;
const int CAGE_EXIT_COL = 9;

char** gameMap = nullptr;
int mapRows = 0, mapCols = 0;
int currentScore = 0;

int blinkyPrevRow = 0, blinkyPrevCol = 1;
char blinkyPrevTile = ' ';

int pinkyPrevRow = 1, pinkyPrevCol = 0;
char pinkyPrevTile = ' ';

int inkyPrevRow = 0, inkyPrevCol = 1;
char inkyPrevTile = ' ';

int clydePrevRow = 0, clydePrevCol = -1;
char clydePrevTile = ' ';

void loadGameMap(const char* filePath, char**& gameMap, int& rows, int& cols) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << filePath << std::endl;
        rows = cols = 0;
        gameMap = nullptr;
        return;
    }

    file >> rows >> cols;
    file.ignore();

    gameMap = new char* [rows];
    for (int i = 0; i < rows; ++i) {
        gameMap[i] = new char[cols + 1]; // +1 for null terminator
    }

    for (int i = 0; i < rows; ++i) {
        file.getline(gameMap[i], cols + 1);
    }

    file.close();
}

void freeGameMapMemory(char** gameMap, int rows) {
    for (int i = 0; i < rows; ++i) {
        delete[] gameMap[i];
    }
    delete[] gameMap;
}

bool isInCage(int row, int col) {
    const int CAGE_TOP_ROW = 7;    
    const int CAGE_BOTTOM_ROW = 9; 
    const int CAGE_LEFT_COL = 7;   
    const int CAGE_RIGHT_COL = 11; 

    return (row >= CAGE_TOP_ROW && row <= CAGE_BOTTOM_ROW && col >= CAGE_LEFT_COL && col <= CAGE_RIGHT_COL);
}

void locateCharacter(int& row, int& col, char ch) {
    for (int i = 0; i < mapRows; ++i) {
        for (int j = 0; j < mapCols; ++j) {
            if (gameMap[i][j] == ch) {
                row = i;
                col = j;
                return;
            }
        }
    }
    row = -1;
    col = -1;
}

void movePacMan(int& pacRow, int& pacCol, char direction, int& score, bool& frightenedMode) {
    int newRow = pacRow;
    int newCol = pacCol;

    if (direction == 'w' || direction == 'W') { 
        newRow--;
    } else if (direction == 'a' || direction == 'A') { 
        newCol--;
    } else if (direction == 's' || direction == 'S') { 
        newRow++;
    } else if (direction == 'd' || direction == 'D') { 
        newCol++;
    }

    if (newRow >= 0 && newRow < mapRows && newCol >= 0 && newCol < mapCols) {
        char target = gameMap[newRow][newCol];
        if ((target == 'B') || (target == 'P') || (target == 'C') || (target == 'I')) {
            gameOver = true;
        }
        if (target == '-' || target == '@' || target == ' ') {
            gameMap[pacRow][pacCol] = ' ';
            gameMap[newRow][newCol] = pacManChar;

            pacRow = newRow;
            pacCol = newCol;

            if (target == '-' || target == '@') {
                score++;
            }
        }
    }
}

void printGameMap(char** gameMap, int rows, int cols) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            std::cout << gameMap[i][j];
        }
        std::cout << std::endl;
    }
}

void clearScreen() {
    system("cls");
}

int getMaxScore(char** gameMap, int rows, int cols) {
    int counter = 0;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (gameMap[i][j] == '-' || gameMap[i][j] == '@') {
                counter++;
            }
        }
    }
    return counter;
}

void exitCageUp(int& ghostRow, int& ghostCol, bool& exitedCage, char ghostChar, char& prevTile, int& prevCol, int& prevRow) {
    int addRow = -1;

    gameMap[ghostRow][ghostCol] = prevTile;

    prevTile = gameMap[ghostRow + addRow][ghostCol];
    ghostRow += addRow;
    gameMap[ghostRow][ghostCol] = ghostChar;

    prevRow = addRow;

    if (ghostRow == CAGE_EXIT_ROW && ghostCol == CAGE_EXIT_COL) {
        exitedCage = true;
    }
}

void moveGhost(int& ghostRow, int& ghostCol, int targetCol, int targetRow, int& prevCol, int& prevRow, char& prevTile, char ghostChar) {
    int directions[4][2] = { {-1, 0}, {0, -1}, {1, 0}, {0, 1} }; 
    int oppositeCol = ghostCol - prevCol;
    int oppositeRow = ghostRow - prevRow;

    int bestCol = 0, bestRow = 0;
    double minDistance = 1e9;
    bool validMoveFound = false;

    for (int i = 0; i < 4; i++) {
        int dRow = directions[i][0];
        int dCol = directions[i][1];

        int newCol = ghostCol + dCol;
        int newRow = ghostRow + dRow;

        if (dCol == -oppositeCol && dRow == -oppositeRow) continue;

        if (gameMap[newRow][newCol] == '#' || gameMap[newRow][newCol] == 'C' || gameMap[newRow][newCol] == 'I' || gameMap[newRow][newCol] == 'P') continue;

        validMoveFound = true;

        double distance = sqrt((targetCol - newCol) * (targetCol - newCol) + (targetRow - newRow) * (targetRow - newRow));

        if (distance < minDistance) {
            minDistance = distance;
            bestCol = dCol;
            bestRow = dRow;
        }
    }

    if (!validMoveFound) {
        int newCol = ghostCol + oppositeCol;
        int newRow = ghostRow + oppositeRow;
        if (newCol >= 0 && newCol < mapCols && newRow >= 0 && newRow < mapRows &&
            gameMap[newRow][newCol] != '#' && gameMap[newRow][newCol] != 'C' &&
            gameMap[newRow][newCol] != 'I' && gameMap[newRow][newCol] != 'P' && gameMap[newRow][newCol] != 'B') {
            bestCol = oppositeCol;
            bestRow = oppositeRow;
        }
    }

    prevCol = ghostCol;
    prevRow = ghostRow;

    ghostCol += bestCol;
    ghostRow += bestRow;

    gameMap[prevRow][prevCol] = prevTile;
    prevTile = gameMap[ghostRow][ghostCol];
    gameMap[ghostRow][ghostCol] = ghostChar;

    if (ghostCol == targetCol && ghostRow == targetRow) {
        gameOver = true;
    }
}

void startGame(char** gameMap, int rows, int cols, int& score, char& pacOrientation,
    int& pacRow, int& pacCol, int& blinkyRow, int& blinkyCol,
    int& pinkyRow, int& pinkyCol, int& inkyRow, int& inkyCol,
    int& clydeRow, int& clydeCol) {

    if (pacRow == -1 || pacCol == -1) {
        std::cerr << "Error: Pac-Man not found in the map." << std::endl;
        freeGameMapMemory(gameMap, rows);
        exit(1);
    }

    bool frightenedMode = false;
    bool blinkyExitedCage = false;
    bool pinkyExitedCage = false;
    bool inkyExitedCage = false;
    bool clydeExitedCage = false;

    while (!gameOver) {
        clearScreen();
        printGameMap(gameMap, rows, cols);

        std::cout << "Score: " << score << std::endl;
        std::cout << "\nEnter direction (w/a/s/d) or q to quit: ";
        char direction;
        std::cin >> direction;

        if (direction == 'q') {
            break;
        }

        pacOrientation = direction;
        movePacMan(pacRow, pacCol, direction, score, frightenedMode);

        if (gameOver) {
            std::cout << "Game Over! A ghost caught Pac-Man!" << std::endl;
            break;
        }

        if (!blinkyExitedCage) {
            exitCageUp(blinkyRow, blinkyCol, blinkyExitedCage, blinkyChar, blinkyPrevTile, blinkyPrevCol, blinkyPrevRow);
        } else {
            moveGhost(blinkyRow, blinkyCol, pacCol, pacRow, blinkyPrevCol, blinkyPrevRow, blinkyPrevTile, blinkyChar);
        }

        if (score >= 20) {
            if (!pinkyExitedCage) {
                exitCageUp(pinkyRow, pinkyCol, pinkyExitedCage, pinkyChar, pinkyPrevTile, pinkyPrevCol, pinkyPrevRow);
            } else {
                moveGhost(pinkyRow, pinkyCol, pacCol, pacRow, pinkyPrevCol, pinkyPrevRow, pinkyPrevTile, pinkyChar);
            }
        }

        if (score >= 40) {
            if (!inkyExitedCage) {
                exitCageUp(inkyRow, inkyCol, inkyExitedCage, inkyChar, inkyPrevTile, inkyPrevCol, inkyPrevRow);
            } else {
                moveGhost(inkyRow, inkyCol, pacCol, pacRow, inkyPrevCol, inkyPrevRow, inkyPrevTile, inkyChar);
            }
        }

        if (score >= 60) {
            if (!clydeExitedCage) {
                exitCageUp(clydeRow, clydeCol, clydeExitedCage, clydeChar, clydePrevTile, clydePrevCol, clydePrevRow);
            } else {
                moveGhost(clydeRow, clydeCol, pacCol, pacRow, clydePrevCol, clydePrevRow, clydePrevTile, clydeChar);
            }
        }
    }
}

int main() {
    loadGameMap("map.txt", gameMap, mapRows, mapCols);
    if (!gameMap) {
        return 1;
    }

    int score = 0;
    char pacOrientation = ' ';
    int pacRow, pacCol;
    locateCharacter(pacRow, pacCol, pacManChar);

    int blinkyRow, blinkyCol;
    locateCharacter(blinkyRow, blinkyCol, blinkyChar);

    int pinkyRow, pinkyCol;
    locateCharacter(pinkyRow, pinkyCol, pinkyChar);

    int inkyRow, inkyCol;
    locateCharacter(inkyRow, inkyCol, inkyChar);

    int clydeRow, clydeCol;
    locateCharacter(clydeRow, clydeCol, clydeChar);

    startGame(gameMap, mapRows, mapCols, score, pacOrientation, pacRow, pacCol, blinkyRow, blinkyCol, pinkyRow, pinkyCol, inkyRow, inkyCol, clydeRow, clydeCol);

    freeGameMapMemory(gameMap, mapRows);
    return 0;
}
