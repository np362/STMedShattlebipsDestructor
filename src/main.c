#include <stm32f091xc.h>
#include <stdio.h>

/**
    @brief Declaration of all global variables
*/

/**
    @brief Initialize UART
*/
void UART_INIT()
{

}


/**
   * @brief Algorithm for shooting 
   * @return coords[array[10x10]], target coords x and y
**/
int int_shoot(int coords){
    // coords is a array including the 10x10 field

    // x,y coordinates of target
    uint8_t x = 0;
    uint8_t y = 0;

    // return modified array with x and y of new target
    return coords, x, y;
}

/*
    @brief main function
    @return 0
*/
int main(void)
{
    // main loop
    for(;;){
        // do something
        asm("nop");
    }
    return 0;
}