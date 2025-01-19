/**
*  
* Solution to course project # 05
* Introduction to programming course
* Faculty of Mathematics and Informatics of Sofia University
* Winter semester 2023/2024
*
* @author Nikolay Gagov
* @idnumber 1MI0600365
* @compiler VC
*
* MAIN FILE: Solution
*
*/

#include <iostream>
#include <fstream>
#include <windows.h>
#include <cmath>

// Global variables
bool gameOver = false;

// Constants for the characters used in the game
const char pacManChar = 'Y';
const char blinkyChar = 'B';
const char pinkyChar = 'P';
const char inkyChar = 'I';
const char clydeChar = 'C';

// Cage exit coordinates
const int CAGE_EXIT_ROW = 7;
const int CAGE_EXIT_COL = 9;

// Game map variables
char** gameMap = nullptr;
int mapRows = 0, mapCols = 0;
int currentScore = 0;

// Previous ghost positions and tiles
int blinkyPrevRow = 0, blinkyPrevCol = 1;
char blinkyPrevTile = ' ';

int pinkyPrevRow = 1, pinkyPrevCol = 0;
char pinkyPrevTile = ' ';

int inkyPrevRow = 0, inkyPrevCol = 1;
char inkyPrevTile = ' ';

int clydePrevRow = 0, clydePrevCol = -1;
char clydePrevTile = ' ';

// Function to load the game map from a file
void loadGameMap(const char* filePath, char**& gameMap, int& rows, int& cols) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << filePath << std::endl;
        rows = cols = 0;
        gameMap = nullptr;
        return;
    }

    file >> rows >> cols;
    file.ignore(); // Ignore the newline after the dimensions

    // Allocate memory for the game map
    gameMap = new char* [rows];
    for (int i = 0; i < rows; ++i) {
        gameMap[i] = new char[cols + 1]; // +1 for null terminator
    }

    // Read the map data
    for (int i = 0; i < rows; ++i) {
        file.getline(gameMap[i], cols + 1);
    }

    file.close();
}

// Function to free memory allocated for the game map
void freeGameMapMemory(char** gameMap, int rows) {
    for (int i = 0; i < rows; ++i) {
        delete[] gameMap[i];
    }
    delete[] gameMap;
}

// Function to check if a ghost is inside the cage
bool isInCage(int row, int col) {
    const int CAGE_TOP_ROW = 7;    
    const int CAGE_BOTTOM_ROW = 9; 
    const int CAGE_LEFT_COL = 7;   
    const int CAGE_RIGHT_COL = 11; 

    return (row >= CAGE_TOP_ROW && row <= CAGE_BOTTOM_ROW && col >= CAGE_LEFT_COL && col <= CAGE_RIGHT_COL);
}

// Function to locate a character (Pac-Man or Ghost) in the game map
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

// Function to move Pac-Man based on user input
void movePacMan(int& pacRow, int& pacCol, char direction, int& score, bool& frightenedMode) {
    int newRow = pacRow;
    int newCol = pacCol;

    // Update Pac-Man's position based on the direction input
    if (direction == 'w' || direction == 'W') { 
        newRow--;
    } else if (direction == 'a' || direction == 'A') { 
        newCol--;
    } else if (direction == 's' || direction == 'S') { 
        newRow++;
    } else if (direction == 'd' || direction == 'D') { 
        newCol++;
    }

    // Check if the new position is within bounds
    if (newRow >= 0 && newRow < mapRows && newCol >= 0 && newCol < mapCols) {
        char target = gameMap[newRow][newCol];

        // Check if Pac-Man collides with a ghost
        if ((target == 'B') || (target == 'P') || (target == 'C') || (target == 'I')) {
            gameOver = true;
        }

        // Move Pac-Man if the target position is empty or contains a collectible
        if (target == '-' || target == '@' || target == ' ') {
            gameMap[pacRow][pacCol] = ' ';
            gameMap[newRow][newCol] = pacManChar;

            pacRow = newRow;
            pacCol = newCol;

            // Increase score if Pac-Man collects an item
            if (target == '-' || target == '@') {
                score++;
            }
        }
    }
}

// Function to print the game map to the console
void printGameMap(char** gameMap, int rows, int cols) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            std::cout << gameMap[i][j];
        }
        std::cout << std::endl;
}

// Function to clear the screen (useful for updating the game state)
void clearScreen() {
    system("cls");
}

// Function to calculate the maximum score based on the number of collectibles in the map
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

// Function for moving a ghost out of the cage
void exitCageUp(int& ghostRow, int& ghostCol, bool& exitedCage, char ghostChar, char& prevTile, int& prevCol, int& prevRow) {
    int addRow = -1;

    gameMap[ghostRow][ghostCol] = prevTile;

    prevTile = gameMap[ghostRow + addRow][ghostCol];
    ghostRow += addRow;
    gameMap[ghostRow][ghostCol] = ghostChar;

    prevRow = addRow;

    // If the ghost reaches the exit coordinates, set the flag to true
    if (ghostRow == CAGE_EXIT_ROW && ghostCol == CAGE_EXIT_COL) {
        exitedCage = true;
    }
}

// Function for moving a ghost toward Pac-Man's position
void moveGhost(int& ghostRow, int& ghostCol, int targetCol, int targetRow, int& prevCol, int& prevRow, char& prevTile, char ghostChar) {
    // Directions array: Up, Left, Down, Right
    int directions[4][2] = { {-1, 0}, {0, -1}, {1, 0}, {0, 1} }; 
    int oppositeCol = ghostCol - prevCol;
    int oppositeRow = ghostRow - prevRow;

    int bestCol = 0, bestRow = 0;
    double minDistance = 1e9;
    bool validMoveFound = false;

    // Loop through the four possible directions and find the best move
    for (int i = 0; i < 4; i++) {
        int dRow = directions[i][0];
        int dCol = directions[i][1];

        int newCol = ghostCol + dCol;
        int newRow = ghostRow + dRow;

        // Avoid moving in the opposite direction
        if (dCol == -oppositeCol && dRow == -oppositeRow) continue;

        // Skip if the new position is blocked by walls or another ghost
        if (gameMap[newRow][newCol] == '#' || gameMap[newRow][newCol] == 'C' || gameMap[newRow][newCol] == 'I' || gameMap[newRow][newCol] == 'P') continue;

        validMoveFound = true;

        // Calculate the distance to Pac-Man and choose the shortest path
        double distance = sqrt((targetCol - newCol) * (targetCol - newCol) + (targetRow - newRow) * (targetRow - newRow));

        if (distance < minDistance) {
            minDistance = distance;
            bestCol = dCol;
            bestRow = dRow;
        }
    }

    // If no valid move is found, go in the opposite direction
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

    // Update ghost position
    prevCol = ghostCol;
    prevRow = ghostRow;

    ghostCol += bestCol;
    ghostRow += bestRow;

    gameMap[prevRow][prevCol] = prevTile;
    prevTile = gameMap[ghostRow][ghostCol];
    gameMap[ghostRow][ghostCol] = ghostChar;

    // Check if the ghost caught Pac-Man
    if (ghostCol == targetCol && ghostRow == targetRow) {
        gameOver = true;
    }
}

// Main game loop
void startGame(char** gameMap, int rows, int cols, int& score, char& pacOrientation,
    int& pacRow, int& pacCol, int& blinkyRow, int& blinkyCol,
    int& pinkyRow, int& pinkyCol, int& inkyRow, int& inkyCol,
    int& clydeRow, int& clydeCol) {

    // Ensure Pac-Man is found in the map
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

    // Main game loop
    while (!gameOver) {
        clearScreen();
        printGameMap(gameMap, rows, cols);

        std::cout << "Score: " << score << std::endl;
        std::cout << "\nEnter direction (w/a/s/d) or q to quit: ";
        char direction;
        std::cin >> direction;

        // Quit game if 'q' is pressed
        if (direction == 'q') {
            break;
        }

        pacOrientation = direction;
        movePacMan(pacRow, pacCol, direction, score, frightenedMode);

        // Check for game over condition (ghost caught Pac-Man)
        if (gameOver) {
            std::cout << "Game Over! A ghost caught Pac-Man!" << std::endl;
            break;
        }

        // Move each ghost
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

// Main function to load the map, start the game, and free memory
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

    // Free the allocated memory after the game ends
    freeGameMapMemory(gameMap, mapRows);
    return 0;
}
