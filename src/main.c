#include <stm32f091xc.h>
#include "clock_.h"
#include "fifo.h"

/**
    @brief Declaration of all global variables
*/
#define BAUDRATE 115200
#define LOG( msg... ) printf( msg );

volatile Fifo_t usart_rx_fifo;
const uint8_t USART2_RX_PIN = 3;
const uint8_t USART2_TX_PIN = 2;

#define TARGET_LEN 9
char match_buffer[TARGET_LEN] = {0};  // Empfangspuffer
uint8_t match_index = 0;

/** =================================================
 * 
 *      Initializing of rudimentary functionalities
 * 
 ==================================================== */ 

/**
 * @brief overwrite the _write function t oredirect the output to USART
 * @return size of message as integer
 */
// For supporting printf function we override the _write function to redirect the output to UART
int _write(int handle, char* data, int size) {
    // 'handle' is typically ignored in this context, as we're redirecting all output to USART2
    // 'data' is a pointer to the buffer containing the data to be sent
    // 'size' is the number of bytes to send

    int count = size;  // Make a copy of the size to use in the loop

    // Loop through each byte in the data buffer
    while (count--) {
        // Wait until the transmit data register (TDR) is empty,
        // indicating that USART2 is ready to send a new byte
        while (!(USART2->ISR & USART_ISR_TXE)) {
            // Wait here (busy wait) until TXE (Transmit Data Register Empty) flag is set
        }

        // Load the next byte of data into the transmit data register (TDR)
        // This sends the byte over UART
        USART2->TDR = *data++;

        // The pointer 'data' is incremented to point to the next byte to send
    }

    // Return the total number of bytes that were written
    return size;
}

/**
    @brief Initialize UART
*/
void UART_INIT()
{
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;    // Enable GPIOA clock
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN; // Enable USART2 clock

    // ---------------- UART TX Pin Configuration (PA2) ----------------
    GPIOA->MODER |= 0b10 << (USART2_TX_PIN * 2);    // Set PA2 to Alternate Function mode
    GPIOA->AFR[0] |= 0b0001 << (4 * USART2_TX_PIN); // Set AF for PA2 (USART2_TX)
    GPIOA->MODER |= 0b10 << (USART2_RX_PIN * 2);    // Set PA3 to Alternate Function mode
    GPIOA->AFR[0] |= 0b0001 << (4 * USART2_RX_PIN); // Set AF for PA3 (USART2_RX)

    USART2->BRR = (APB_FREQ / BAUDRATE); // Set baud rate (requires APB_FREQ to be defined)
    USART2->CR1 |= 0b1 << 2;             // Enable receiver (RE bit)
    USART2->CR1 |= 0b1 << 3;             // Enable transmitter (TE bit)
    USART2->CR1 |= 0b1 << 0;             // Enable USART (UE bit)
    USART2->CR1 |= 0b1 << 5;             // Enable RXNE interrupt (RXNEIE bit)
}

/** =========================================
 *              
 *              Game functionalities
 * 
 ============================================*/

/**
 * @brief Generates field constellation of ships
 * @return battlefield as int array
 
int generate_field(){
    //int field[100] = 0;
    int ShipSize = 5;

    for(int i = 2; i<ShipSize+1; i++){
        
    }

    return 0;
}
*/

/**
 * @brief Calculates checksum of battlefield
 * @return checksum
int checksum(int *field){
    int size;
    size = sizeof(&field);

    return 0;
}
*/ 


/**
   * @brief Algorithm for shooting 
   * @return coords[array[10x10]], target coords x and y

int int_shoot(int coords){
    // coords is a array including the 10x10 field

    // x,y coordinates of target
    //uint8_t x = 0;
    uint8_t y = 0;

    // return modified array with x and y of new target
    return y;
}
*/

/*
    @brief main function
    @return 0
*/
int main(void)
{
    SystemClock_Config();
    //UART_INIT();

    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;    // Enable GPIOA clock
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN; // Enable USART2 clock

    // ---------------- UART TX Pin Configuration (PA2) ----------------
    GPIOA->MODER |= 0b10 << (USART2_TX_PIN * 2);    // Set PA2 to Alternate Function mode
    GPIOA->AFR[0] |= 0b0001 << (4 * USART2_TX_PIN); // Set AF for PA2 (USART2_TX)
    GPIOA->MODER |= 0b10 << (USART2_RX_PIN * 2);    // Set PA3 to Alternate Function mode
    GPIOA->AFR[0] |= 0b0001 << (4 * USART2_RX_PIN); // Set AF for PA3 (USART2_RX)

    USART2->BRR = (APB_FREQ / BAUDRATE); // Set baud rate (requires APB_FREQ to be defined)
    USART2->CR1 |= 0b1 << 2;             // Enable receiver (RE bit)
    USART2->CR1 |= 0b1 << 3;             // Enable transmitter (TE bit)
    USART2->CR1 |= 0b1 << 0;             // Enable USART (UE bit)
    USART2->CR1 |= 0b1 << 5;             // Enable RXNE interrupt (RXNEIE bit)

    NVIC_SetPriorityGrouping(0);                               // Use 4 bits for priority, 0 bits for subpriority
    uint32_t uart_pri_encoding = NVIC_EncodePriority(0, 1, 0); // Encode priority: group 1, subpriority 0
    NVIC_SetPriority(USART2_IRQn, uart_pri_encoding);          // Set USART2 interrupt priority
    NVIC_EnableIRQ(USART2_IRQn);                               // Enable USART2 interrupt

    fifo_init((Fifo_t *)&usart_rx_fifo);                       // Init the FIFO

    uint32_t bytes_recv = 0;

    // Case switch
    int state = 0;

    int ret; 
    // main loop
    for(;;)
    {    
        uint8_t byte;
        ret = fifo_get((Fifo_t *)&usart_rx_fifo, &byte) == 0;
        if (ret)
        {  
            //LOG("[Test] "); 
            
            // Charakter an richtiger Stelle speichern
            match_buffer[match_index] = byte;
            match_index++;

                // Wenn Buffer voll -> prüfen
                if (match_index == TARGET_LEN - 1)
                {
                    match_buffer[match_index] = '\0'; // Nullterminieren
                    //LOG("%s\n", match_buffer);
                    
                    if (strcmp(match_buffer, "HD_START") == 0)
                    {
                        state = 1;
                        match_index = 0;
                        //LOG("DH_START_KRAPFEN\n");
                        //break;
                    }
                    else
                    {
                        // Nachricht passt nicht -> Buffer verschieben um 1 (FIFO-like Verhalten)
                        for (uint8_t i = 0; i < TARGET_LEN - 2; i++)
                        {
                            match_buffer[i] = match_buffer[i + 1];
                        }
                        match_index--; // wieder Platz am Ende
                    }
                    //LOG("STATE %d \n", state);
                }
            
           /* switch (state){
            case 1:
                LOG("DH_START_KRAPFEN\n");
                state++;
                break;

            case 2: 
                LOG("DH_CS_5262123504\n");
                state++;
                break; 
            }*/
            bytes_recv++;
        }

        switch (state){
            case 1:
                LOG("DH_START_KRAPFEN\n");
                state++;
                break;

            case 2: 
                LOG("DH_CS_5262123504\n");
                state = 0;
                break;
                
        }

        //LOG("DH_START_KRAPFEN\n");
        
    }
    return 0;
}


void USART2_IRQHandler(void)
{
  static int ret; // You can do some error checking
  if (USART2->ISR & USART_ISR_RXNE)
  {                                              // Check if RXNE flag is set (data received)
    uint8_t c = USART2->RDR;                     // Read received byte from RDR (this automatically clears the RXNE flag)
    ret = fifo_put((Fifo_t *)&usart_rx_fifo, c); // Put incoming Data into the FIFO Buffer for later handling
  }
}