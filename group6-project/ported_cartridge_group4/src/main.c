#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "wrapper.h"
#include "display.h"
#include "game.h"

#define MTIME_LOW (*((volatile uint32_t *)0x40000008))
#define TIMER_INTERVAL 10000

volatile int global = 42;
volatile uint32_t controller_status = 0;

typedef uint32_t *TContext;
typedef void (*TEntry)(void *);
TContext InitContext(uint32_t *stacktop, TEntry entry, void *param);
void SwitchContext(TContext *oldcontext, TContext newcontext);
uint32_t ThreadStack[128];
TContext Mainthread;
TContext Otherthread;

// CMD
volatile uint32_t *INT_PEND_REG = (volatile uint32_t *)(0x40000004);
volatile uint32_t *INT_ENABLE_REG = (volatile uint32_t *)(0x40000000);

volatile int VID_INTRR_CNT = 0; // Count of video interrupts
volatile int CMD_INTRR_CNT = 0; // Count of CMD interrupts
volatile int bg = 1;

void interrupt_handler(void)
{
    // command button interrupt counter increment
    if (((*INT_PEND_REG) & 0x4) >> 2)
    {
        CMD_INTRR_CNT++;
        (*INT_PEND_REG) |= 0x4;
    }
    // video interrupt counter increment
    if (((*INT_PEND_REG) & 0x2) > 0)
    {
        VID_INTRR_CNT++;
        (*INT_PEND_REG) |= 0x2;
    }
}

void toggleBGColor()
{
    if (bg == 1)
    {
        setBackgroundColor(0, 0, 0x00000000);
        bg = 2;
    }
    else if (bg == 2)
    {
        setBackgroundColor(0, 0, 0xFF66CCFF);
        bg = 1;
    }
}

int main()
{
    int last_global = 42;
    // switchToTextMode();
    // printText("GAME START!!!");
    initializeSpriteControllers();
    switchToGraphicsMode();
    setColor(0, 0, 0x80FF0000);
    setBackgroundColor(0, 0, 0xFF66CCFF);

    drawRectangleWithBackgroundSpriteControl(0, generateBackgroundConfig(0, 0, 0, 0));

    int foodX = 100;
    int foodY = 100;
    int centerXPos = foodX + 4;
    int centerYPos = foodY + 4;
    int snakeSpeed = 1; // modify this to increase or decrease initial speed

    generateFoodBlock();
    drawRectangleWithSmallSprite(0, generateSmallSpriteConfig(foodX, foodY, 8, 8, 0));

    int control_idx = 1;
    int currXPos = 0;
    int currYPos = 0;
    int growSnakeParam = 5;
    int snakeStatus = 1;
    uint32_t current_status = 0;
    uint32_t last_status = 0;
    const int snakeLength = 6;
    int CMD_INTRR = 0;
    int CURRENT_CMD_INTRR;
    int pause = 0;
    int m_count = MTIME_LOW;

    Otherthread = InitContext(ThreadStack + 128, endGame, NULL);

    while (snakeStatus == 1)
    {
        global = getTicksVal();
        interrupt_handler();
        CURRENT_CMD_INTRR = CMD_INTRR_CNT;
        if (CURRENT_CMD_INTRR != CMD_INTRR)
        {
            if (pause == 0)
            {              // if pause is untrue, let's pause
                pause = 1; // let's disable pause
            }
            else if (pause == 1)
            { // if pause is enabled, let's resume
                pause = 0;
            }
            CMD_INTRR = CURRENT_CMD_INTRR;
        }
        if (global != last_global && pause == 0)
        {
            controller_status = getStatus();
            if (controller_status == 0x0)
            {
                current_status = last_status;
            }
            else
            {
                current_status = controller_status;
            }
            if (current_status & 0x1)
            {
                if (currXPos >= snakeSpeed)
                {
                    currXPos -= snakeSpeed;
                }
                else
                {
                    break;
                }
            }
            if (current_status & 0x2)
            {
                if (currYPos >= snakeSpeed)
                {
                    currYPos -= snakeSpeed;
                }
                else
                {
                    break;
                }
            }
            if (current_status & 0x4)
            {
                if (currYPos <= DISPLAY_HEIGHT - 1 - snakeSpeed)
                {
                    currYPos += snakeSpeed;
                }
                else
                {
                    break;
                }
            }
            if (current_status & 0x8)
            {
                if (currXPos <= DISPLAY_WIDTH - 1 - snakeSpeed)
                {
                    currXPos += snakeSpeed;
                }
                else
                {
                    break;
                }
            }
            if (getFoodStatus(currXPos, currYPos, centerXPos, centerYPos, growSnakeParam))
            {
                snakeSpeed += 1;     // increase difficulty
                growSnakeParam += 3; // modify this to make the snake grow longer or shorter when consuming food
                foodX = genRandom(DISPLAY_WIDTH);
                foodY = genRandom(DISPLAY_HEIGHT);
                moveSmallSprite(0, foodX, foodY);
                centerXPos = foodX + (snakeLength / 2);
                centerYPos = foodY + (snakeLength / 2);
            }

            snakeStatus = getSnakeStatus(currXPos, currYPos, growSnakeParam);
            if (getSmallSpriteControl(control_idx) == 0x0)
            {
                drawRectangleWithSmallSprite(control_idx, generateSmallSpriteConfig(currXPos, currYPos, snakeLength, snakeLength, 0));
            }
            else
            {
                moveSmallSprite(control_idx, currXPos, currYPos);
            }
            control_idx++;
            if (control_idx == growSnakeParam)
            {
                control_idx = 1;
            }
            last_global = global;
            last_status = current_status;
        }

        if (MTIME_LOW >= m_count + TIMER_INTERVAL)
        {
            toggleBGColor();
            m_count = MTIME_LOW;
        }
    }
    SwitchContext(&Mainthread, Otherthread);
    return 0;
}
