#include "../src/srgp.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define WIDTH 500
#define HEIGHT 400

#define BLUE 2
#define GRAY 3

#define X_COUNT 12

int x_axis[X_COUNT];
int y_axis[X_COUNT];

void clearScreen()
{
    SRGP_setColor(SRGP_BLACK);
    SRGP_fillRectangleCoord(0, 0, WIDTH, HEIGHT);
    SRGP_setColor(SRGP_WHITE);
}

void updateScreen()
{
    static boolean initialized = FALSE;
    clearScreen();
    int x_step = (WIDTH - 50)/X_COUNT;
    SRGP_setColor(SRGP_WHITE);
    SRGP_lineCoord(50/2, 50/2, 50/2, HEIGHT - 50);
    SRGP_lineCoord(50/2, 50/2, WIDTH - 50, 50/2);
    for(int i = 0; i < X_COUNT; i++) {
        x_axis[i] = (50/2) + (i*x_step);
    }
    SRGP_setColor(GRAY);
    SRGP_setLineWidth(1);

    for(int i = 1; i < X_COUNT; i++) {
        SRGP_lineCoord(x_axis[i], 50/2, x_axis[i], HEIGHT - 50);
    }

    SRGP_setLineWidth(2);
    if(initialized == FALSE) {
        for(int i = 0; i < X_COUNT; i++) {
            y_axis[i] = (50/2) + (rand()%(HEIGHT - 50));
        }        
    } else {
        for(int i = 1; i < X_COUNT; i++) {
            y_axis[i - 1] = y_axis[i];
            if(i == X_COUNT - 1) {
                y_axis[i] = (50/2) + (rand()%(HEIGHT - 50));
            }
        }
    }

    initialized = TRUE;
    
    SRGP_setColor(BLUE);
    SRGP_polyLineCoord(X_COUNT, x_axis, y_axis);
    SRGP_setLineWidth(1);
}

int
main()
{
    srand(time(NULL));
    SRGP_begin("new_test", WIDTH, HEIGHT, 0, TRUE);
    SRGP_setLineWidth(2);
    SRGP_loadCommonColor(BLUE, "lightblue");
    SRGP_loadCommonColor(GRAY, "gray");

    while(1) {
        updateScreen();
        SRGP_refresh();
        sleep(1);
    }

    
    SRGP_setKeyboardProcessingMode (RAW);
    SRGP_setInputMode (KEYBOARD, EVENT);
    SRGP_waitEvent(-1);

    return 0;
}
