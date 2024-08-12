//
//  snake.c
//  Snake Course Project — CS 355, Section 01
//
//  Authored by Rumman Shahzad and Shah Qureshi.
//

// Headers, global variables, and function prototypes.
// Primary Author: Shah Qureshi

// Header files
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>

// Global variables
int dirX, dirY, initLength, snakeLength, trophyX, trophyY, trophyVal, trophyTime;
bool hardMode; // In hard difficulty, the snake will move faster and a longer snake length (65% of the perimeter) is required to win.

// Function prototypes
void drawBorder(void);
void initTrophy(int[][2]);
void randomStartDir(int[]);
void handleInput(int);
void moveSnake(int[][2]);
bool borderDeath(int[][2]);
bool collisionDeath(int[][2], int);
bool victory(int);
void gameOver(int, int[][2], bool);

// Main function, entry point/driver of snake game. To play a hard version, pass "-hard" as second argument.
// Primary Author: Rumman Shahzad
int main(int argc, const char * argv[]) {
    
    // MARK: Game Setup
    
    // Set hard difficulty if specified
    if (argc >= 2 && strcmp(argv[1], "-hard") == 0) {
        hardMode = true;
        printf("NOTE: You have selected hard difficulty, which means that the snake will move slightly faster, and a longer length is required to win the game. Good luck!\n");
        sleep(5);
    }
    
    // Initialize curses library
    initscr();
    curs_set(0);
    timeout(0); // Non-blocking program, won't halt for user input (explained below)
    noecho();
    keypad(stdscr, TRUE);
    
    // Create color pairs
    start_color();
    init_pair(1, COLOR_CYAN, COLOR_BLACK); // border
    init_pair(2, COLOR_GREEN, COLOR_BLACK); // snake, victory screen
    init_pair(3, COLOR_YELLOW, COLOR_BLACK); // trophies
    init_pair(4, COLOR_MAGENTA, COLOR_BLACK); // border (hard mode)
    init_pair(5, COLOR_RED, COLOR_BLACK); // snake (hard mode), death, game over screen
    
    // Control game pace and trophy expiration
    int sleepTime = (hardMode) ? 85000 : 100000;
    int expiryTimer = 0;
    
    // Create the snake (memory allocated determined based on playing space)
    int snakeBody[2*(COLS + LINES)][2];
    snakeBody[0][0] = COLS / 2;
    snakeBody[0][1] = LINES / 2;
    initLength = 1;
    snakeLength = 5;
    
    // Game starts with snake moving in random direction
    int randValues[2];
    randomStartDir(randValues);
    dirX = randValues[0];
    dirY = randValues[1];
    
    // Setup snake pit and place first trophy
    drawBorder();
    initTrophy(snakeBody);
    
    // Load screen
    refresh();
    
    // MARK: Gameplay
    
    // Loop to control game until a collision/victory occurs
    while (borderDeath(snakeBody) && collisionDeath(snakeBody, snakeLength) && !victory(snakeLength)) {
        
        // Process arrow key input if one was pressed
        int keyPressed = getch();
        if (keyPressed != ERR) {
            handleInput(keyPressed); // Only handle the input if a key was pressed
        }
        
        // Move the snake, changing direction if needed
        moveSnake(snakeBody);
        
        // Snake consumes a trophy
        if(snakeBody[0][1] == trophyX && snakeBody[0][0] == trophyY) {
            
            // Lengthen snake and increase its speed
            snakeLength += trophyVal;
            sleepTime -= trophyVal * ((hardMode) ? 475 : 500);
            
            // Generate new trophy and reset trophy timer
            initTrophy(snakeBody);
            expiryTimer = 0;
        }
        
        // Trophy expired; erase old one and generate a new one
        if (expiryTimer >= (trophyTime*1000000)) {
            
            mvprintw(trophyX, trophyY, " ");
            initTrophy(snakeBody);
            expiryTimer = 0; // Reset trophy timer
        }
        
        refresh(); // Update screen with changes
        expiryTimer += sleepTime; // Timer running for trophy expiration
        usleep(sleepTime);
    }
    
    // When the loop exits (game halts) determine outcome
    gameOver(snakeLength, snakeBody, victory(snakeLength));
    
    // End curses and terminate program
    endwin();
    return 0;
}

// Draws the border at the start of the game.
// Primary Author: Shah Qureshi
void drawBorder(void) {
    
    attron(COLOR_PAIR((hardMode) ? 4 : 1)); // Border color, depending on difficulty
    
    // Draw top and bottom borders
    for (int i = 0; i < COLS; i++) {
        mvprintw(0, i, "#");
        mvprintw(LINES-1, i, "#");
    }
    
    // Draw left and right borders
    for (int i = 1; i < LINES-1; i++) {
        mvprintw(i, 0, "#");
        mvprintw(i, COLS-1, "#");
    }
    
    attroff(COLOR_PAIR((hardMode) ? 4 : 1));
}

// Create a trophy at a random position on the screen that is not inside the snake
// Primary Author: Rumman Shahzad
void initTrophy(int snakeBody[][2]) {
    
    // Initialize X and Y coordinates of trophy to a random spot within the border
    trophyX = rand() % (LINES-3) + 2;
    trophyY = rand() % (COLS-3) + 2;
    
    // Confirm the trophy is not inside the snake
    int i = 0;
    while (i < snakeLength) {
        if (snakeBody[i][0] == trophyY && snakeBody[i][1] == trophyX) {
            
            // Conflict found; generate a new location
            trophyX = rand() % (LINES-3) + 2;
            trophyY = rand() % (COLS-3) + 2;
            
            // Check again from the beginning
            i = 0;
        }
        i++;
    }
    
    // Set trophy attributes
    trophyVal = (rand() % 9) + 1; // Random trophy value
    trophyTime = (rand() % 9) + 1; // Random trophy expiration time
    
    // Parse int randVal into string valStr
    char valStr[10];
    sprintf(valStr, "%d", trophyVal);
    
    // Display trophy at coordinates
    attron(COLOR_PAIR(3)); // Yellow trophy color
    mvprintw(trophyX, trophyY, "%s", valStr);
    attroff(COLOR_PAIR(3));
}

// Set random starting direction of snake by X and Y axes. -1 or 1 indicates the snake moves left/up or right/down, respectively (depending on which axis is triggered). If one direction value is -1 or 1, the other's value MUST be 0.
// Primary Author: Rumman Shahzad
void randomStartDir(int randValues[]) {
    
    // Initialize local variables
    srand((unsigned int) time(0));
    int randDirX, randDirY;
    
    int min = -1; // starting direction left or down
    int max = 1; // starting direction right or up
    
    // Direction of 0 indicates that said axis isn't used (to avoid diagonal movement)
    
    randDirX = min + rand() % (max - min + 1); // Set a random X direction
    
    if (randDirX != 0) { // If X direction set successfully, no need to set Y
        randDirY = 0;
    } else {
        
        // X was not set; we need to set a Y direction
        do {
            randDirY = min + rand() % (max - min + 1);
        } while (randDirY == 0);
    }
    
    // Store direction values in the array
    randValues[0] = randDirX;
    randValues[1] = randDirY;
}

// Changes the snake's direction based on arrow keys
// Primary Author: Shah Qureshi
void handleInput(int keyPressed) {
    
    switch (keyPressed) {
        case KEY_UP:
            dirY = -1;
            dirX = 0;
            break;
        case KEY_DOWN:
            dirY = 1;
            dirX = 0;
            break;
        case KEY_LEFT:
            dirY = 0;
            dirX = -1;
            break;
        case KEY_RIGHT:
            dirY = 0;
            dirX = 1;
            break;
    }
}

// Controls movement of the snake (given a certain direction)
// Primary Author: Rumman Shahzad
void moveSnake(int snakeBody[][2]) {
    
    attron(COLOR_PAIR((hardMode) ? 5 : 2)); // Snake color, depending on difficulty
    
    // Clear the snake's old trailing characters
    for (int i = 0; i < initLength; i++) {
        mvprintw(snakeBody[i][1], snakeBody[i][0], " ");
    }
    
    // Move each segment of the snake body in the appropriate direction
    for (int i = initLength; i > 0; i--) {
        snakeBody[i][0] = snakeBody[i-1][0];
        snakeBody[i][1] = snakeBody[i-1][1];
    }
    
    // Increment snake's head coordinates (based on direction)
    snakeBody[0][0] += dirX;
    snakeBody[0][1] += dirY;
    
    // Redraw the snake in its new position
    mvprintw(snakeBody[0][1], snakeBody[0][0], "O");
    for (int i = 1; i < initLength; i++)
        mvprintw(snakeBody[i][1], snakeBody[i][0], "o");
    
    // Allows snake length to grow during gameplay (upon trophy consumption)
    if (initLength < snakeLength)
        initLength++;
    
    attroff(COLOR_PAIR((hardMode) ? 5 : 2));
}

// Snake dies if it runs into the border
// Primary Author: Shah Qureshi
bool borderDeath(int snakeBody[][2]) {
    
    // If the snake ran into side walls or top/bottom (specified by conditions below), return false. Else, the snake is still alive (return true).
    
    return (snakeBody[0][0] >= COLS-1 || snakeBody[0][0] <= 0) ? false : (snakeBody[0][1] >= LINES-1 || snakeBody[0][1] <= 0) ? false : true;
}

// Snake dies if it collides with itself or direction is reversed
// Primary Author: Shah Qureshi
bool collisionDeath(int snakeBody[][2], int snakeLength) {
    
    // Check if snake collided with itself at any point
    for (int i = 1; i < snakeLength; i++) {
        if (snakeBody[i][0] == snakeBody[0][0] && snakeBody[i][1] == snakeBody[0][1]) {
            return false;
        }
    }
    
    // Snake is still alive
    return true;
}

// Game ends with a victory if snake length becomes size of half the snake pit perimeter. (HARD DIFFICULTY: Snake length becomes 65% of perimeter.)
// Primary Author: Shah Qureshi
bool victory(int snakeLength) {
    return (hardMode) ? (snakeLength >= (int) (0.65 * (2*(COLS-1) + 2*(LINES-1)))) : (snakeLength >= (COLS-1) + (LINES-1));
}

// When the game halts, determines if the player won or lost, displaying the corresponding message
// Primary Author: Rumman Shahzad
void gameOver(int snakeLength, int snakeBody[][2], bool isVictory) {
    
    // Store points in a string (cumulative value of trophies consumed)
    char points[COLS+LINES];
    sprintf(points, "%d", snakeLength-5);
    
    // Set color for ending screen
    attron(COLOR_PAIR((isVictory) ? 2 : 5)); // Green or red
    
    if (isVictory) {
        
        // Display victory screen
        sleep(1);
        clear();
        mvprintw(((LINES - 1) / 2) - 1, ((COLS-1) / 2) -3, "You Won!"); // Printed in center
        
    } else { // Loss
        
        // Show impact point of head
        mvprintw(snakeBody[0][1], snakeBody[0][0], "X"); // Based on boolean logic, this will be printed in red color
        refresh();
        sleep(1);
        
        // Display losing screen
        clear();
        mvprintw(((LINES - 1) / 2) - 1, ((COLS-1) / 2) -3, "Game Over!"); // Printed in center
    }
    
    // Print out the points in the center (underneath win/loss message)
    mvprintw(((LINES - 1) / 2), ((COLS-1) / 2) -3, "Points: ");
    mvprintw(((LINES - 1) / 2), ((COLS-1) / 2) + 5, "%s", points);
    attroff(COLOR_PAIR((isVictory) ? 2 : 5));
    
    refresh();
    sleep(3);
}
