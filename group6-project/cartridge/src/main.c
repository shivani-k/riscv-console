#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "api.h"
#include "display.h"
#include "game.h"

volatile int global = 42;
volatile uint32_t controller_status = 0;

volatile uint32_t *background_sprite_control = (volatile uint32_t *)(0x500F5A00);

// threads

typedef uint32_t *TContext;
typedef void (*TEntry)(void*);

TContext InitContext(uint32_t *stacktop, TEntry entry,void *param);
void SwitchContext(TContext *oldcontext, TContext newcontext);


uint32_t ThreadStack[128];
TContext Mainthread;
TContext Otherthread;

int main() {
    int last_global = 42;
    // switchToTextMode();
    // printText("GAME START!!!");
    switchToGraphicsMode();
    // Set color to sprite palette
    setColor(0, 0, 0x8000A65F);
    setColor(0, 1, 0x80FFFFFF);
    setColor(0, 2, 0xFFC19A6B);
    setBackgroundColor(0, 0, 0x80C19A6B);
    drawRectangleWithBackgroundSpriteControl(0, generateBackgroundConfig(0,0,0,0));

    int pellet_x = 100;
    int pellet_y = 100;
    int center_x = pellet_x + 4;
    int center_y = pellet_y + 4;
    int step_size = 3;
    
    drawPellet();
    drawRectangleWithSmallSprite(0, generateSmallSpriteConfig(pellet_x,pellet_y,8,8,0));

    int control_idx = 1;
    int cur_x = 0;
    int cur_y = 0;
    int budget = 5;
    int alive = 1;
    uint32_t current_status = 0;
    uint32_t last_status = 0;
    const int snake_width = 6;
    int cmd_interrupt = 0;
    int current_cmd_interrupt;

    // threads
    Otherthread = InitContext(ThreadStack + 128, gameOver, NULL);

    while (alive == 1) {
        global = getTicks();
        current_cmd_interrupt = getCMDInterruptCount();
        if (current_cmd_interrupt != cmd_interrupt){
            pellet_x = genRandom(DISPLAY_WIDTH);
            pellet_y = genRandom(DISPLAY_HEIGHT);
            moveSmallSprite(0, pellet_x, pellet_y);
            center_x = pellet_x + (snake_width/2);
            center_y = pellet_y + (snake_width/2);
            cmd_interrupt = current_cmd_interrupt;
        }
        if(global != last_global){
            controller_status = getStatus();
            if (controller_status == 0x0){
                current_status = last_status;
            }
            else {
                current_status = controller_status;
            }
            if(current_status & 0x1){
                if(cur_x >= step_size){
                    cur_x -= step_size;
                }
                else{
                    break;
                }
            }
            if(current_status & 0x2){
                if( cur_y >= step_size){
                    cur_y -= step_size;
                }
                else{
                    break;
                }
            }
            if(current_status & 0x4){
                if( cur_y <= DISPLAY_HEIGHT - 1 - step_size){
                    cur_y += step_size;
                }
                else{
                    break;
                }
            }
            if(current_status & 0x8){
                if(cur_x <= DISPLAY_WIDTH - 1 - step_size){
                    cur_x += step_size;
                }
                else{
                    break;
                }
            }
            if (checkGetPellet(cur_x, cur_y, center_x, center_y, budget)){
                budget += 3;
                pellet_x = genRandom(DISPLAY_WIDTH);
                pellet_y = genRandom(DISPLAY_HEIGHT);
                moveSmallSprite(0, pellet_x, pellet_y);
                center_x = pellet_x + (snake_width/2);
                center_y = pellet_y + (snake_width/2);
            }

            alive = checkAlive(cur_x, cur_y, budget);
            if (getSmallSpriteControl(control_idx) == 0x0){
                drawRectangleWithSmallSprite(control_idx, generateSmallSpriteConfig(cur_x,cur_y,snake_width,snake_width,0));
            }
            else{
                moveSmallSprite(control_idx, cur_x, cur_y);
            }
            control_idx++;
            if (control_idx == budget){
                control_idx = 1;
            }
            last_global = global;
            last_status = current_status;
        }
    }
    // Thread
    SwitchContext(&Mainthread, Otherthread);
    return 0;
}
