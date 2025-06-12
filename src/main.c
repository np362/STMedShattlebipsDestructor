#include <stm32f091xc.h>
#include "clock_.h"
#include "fifo.h"

/** =================================================
 * 
 *      Declaration of global variables
 * 
 ====================================================*/
 // Baudrate and LOG function
#define BAUDRATE 115200
#define LOG( msg... ) printf( msg );

// PINs for Receiver and Transmitter
const uint8_t USART2_RX_PIN = 3;
const uint8_t USART2_TX_PIN = 2;

// PIN for PWM
#define PIN_SPEAKER 8
// Define all Notes
# define NOTE_E 659 // E5
# define NOTE_B 987 // B5
# define NOTE_C 1047 // C6
# define NOTE_D 1175 // D6
# define NOTE_F 1397 // F6
# define NOTE_A 880 // A5
# define NOTE_G 784 // G5

const uint16_t notes[] = {NOTE_C, NOTE_D, NOTE_E, NOTE_F, NOTE_G, NOTE_A, NOTE_B};

// FIFO variables
volatile Fifo_t usart_rx_fifo;
#define TARGET_LEN 20
char match_buffer[TARGET_LEN] = {0};
uint8_t match_index = 0;

// Field constants
#define FIELD_SIZE 100

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

/**
 * @brief calculates arr value for different frequencies
 */
uint32_t arr_from_freq(uint16_t freq) {
    return APB_FREQ / (freq * (TIM3->PSC + 1)) - 1;
}

/**
 * @brief Initialize PWM for speaker
 */
void PWM_INIT()
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

    // PC8 as AF
    GPIOC->MODER |= (0b10 << (PIN_SPEAKER * 2));
    GPIOC->AFR[1] &= ~(0b0000 << (PIN_SPEAKER * 4));
    GPIOC->AFR[1] |= (0b0000 << (PIN_SPEAKER * 4)); // AF0 für TIM3

    TIM3->PSC = 47; // ergibt 1 MHz Timer-Takt (48 MHz / (47+1))
    TIM3->ARR = arr_from_freq(NOTE_A);
    TIM3->CCR3 = TIM3->ARR / 2;

    TIM3->CCMR2 |= (0b110 << 4); // PWM Mode 1
    TIM3->CCER |= TIM_CCER_CC3E;
    TIM3->CR1 &= ~TIM_CR1_CEN;
}

/**
 * @brief plays a victory tune
 */
void play_victory_tune()
{
    TIM3->CR1 |= TIM_CR1_CEN;
    for(int idx = 0; idx < 7; idx++)
    {
        //TIM3->CNT = 0;
        TIM3->ARR = arr_from_freq(notes[idx]);
        TIM3->CCR3 = TIM3->ARR / 2;
        for(uint32_t i = 0; i < 1000000; i++ )
        {
            asm("nop"); // No operation, used for delaying
        }

        //while(TIM3->CNT < 1000000);
    }
    
}

/** =========================================
 *              
 *              Game functionalities
 * 
 ============================================*/

/**
 * @brief Generates field constellation of ships
 * @return battlefield as int array
*/
// This function generates a constant field for the game
int field[FIELD_SIZE] = {
        0, 0, 0, 5, 5, 5, 5, 5, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        2, 0, 0, 0, 3, 3, 3, 0, 0, 0,
        2, 0, 0, 0, 0, 0, 0, 0, 4, 0,
        0, 0, 3, 3, 3, 0, 0, 0, 4, 0,
        3, 0, 0, 0, 0, 0, 0, 0, 4, 0,
        3, 0, 2, 2, 0, 0, 0, 0, 4, 0,
        3, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 4, 4, 4, 4, 0, 2, 0, 2,
        0, 0, 0, 0, 0, 0, 0, 2, 0, 2
};
int field_copy[FIELD_SIZE];
int enemy_field[FIELD_SIZE] = {0};

void init_fields() {
    // Initialize the copy of the field with the same values as the original field and reset enemy field
    for(int i = 0; i < FIELD_SIZE; i++) {
        field_copy[i] = field[i];
        enemy_field[i] = 0;
    }
}

 // constant field
 int *generate_field(){
    return field;
 }

/*
 // random field
int *generate_field(){
    //int field[100] = 0;
    int ShipSize = 6; // 5 (biggest shipsize) + 1 (amount of biggest ship)
    int FieldSize = 100;
    int x = 0;
    int y = 0;

    for(int length = 2; length<ShipSize; length++)
    {
        int amount = ShipSize-length;
            if(amount > 0) // && (!valid_field(x,y,length)) // && ((field[x-1 + y*10] == length) ||  (field[x+1 + y*10] == length) || (field[x + (y-1)*10] == length) || (field[x + (y+1)*10] == length))
            {
                /**
                field[x + y*10] = length;
                
                amount--;
            }

    }

    return 0;
}
*/

int valid_field(int coord_x, int coord_y, int length)
{
    if(((field[coord_x-1 + coord_y*10] == length) ||  (field[coord_x+1 + coord_y*10] == length) || (field[coord_x + (coord_y-1)*10] == length) || (field[coord_x + (coord_y+1)*10] == length)))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}



/**
 * @brief Calculates checksum of battlefield
 * @return checksum
 */

// checksum_string function to convert the checksum into a string
void checksum_string(/*int *field,*/ char *output) {
    for (int row = 0; row < 10; row++) {
        int count = 0;
        for (int col = 0; col < 10; col++) {
            if (field[row * 10 + col] != 0) {
                count++;
            }
        }
        // write the count to the output string
        output[row] = '0' + count;
    }
    output[10] = '\0';
}

/**
   * @brief Algorithm for shooting 
   * @return target coords x and y
*/

void shoot(int *enemy_field, int *x, int *y){
    for (int i = 0; i < FIELD_SIZE; i++){
        if((enemy_field[i] != 1) && (enemy_field[i] != 2) && (enemy_field[i] != 3)){ // 1 is water, 2 is hit, 3 is pending shot
            // if the field is not empty, return the coordinates
            *x = i % 10; // x coordinate
            *y = i / 10; // y coordinate
            break;
        }
    }
}

/**
 * @brief Checks if the shot was a hit or miss
 * @return "H" for hit or "M" for miss
 */
int check_hit(uint8_t x, uint8_t y) {
    if (field[y * 10 + x] != 0) {
        field_copy[y * 10 + x] = 0; // Mark as hit
        return 1;
    } else {
        return 0;
    }
}

void send_battlefield()
{
    for(int i = 0; i < 10; i++)
    {
        LOG("DH_SF%dD", i);
        for(int j = 0; j < 10; j++)
        {
            LOG("%d", field[i * 10 + j]);
        }
        LOG("\n");
    }
}


/*
    @brief main function
    @return 0
*/
int main(void)
{
    SystemClock_Config();
    UART_INIT();
    PWM_INIT();

    NVIC_SetPriorityGrouping(0);                               // Use 4 bits for priority, 0 bits for subpriority
    uint32_t uart_pri_encoding = NVIC_EncodePriority(0, 1, 0); // Encode priority: group 1, subpriority 0
    NVIC_SetPriority(USART2_IRQn, uart_pri_encoding);          // Set USART2 interrupt priority
    NVIC_EnableIRQ(USART2_IRQn);                               // Enable USART2 interrupt

    fifo_init((Fifo_t *)&usart_rx_fifo);                       // Init the FIFO

    uint32_t bytes_recv = 0;

    // Case switch
    int state = 0;

    // Initialize field and checksum
    //init_field_copy();
    char checksum_str[11];
    // Has to be put inside the case state as soon as field generates randomly
    checksum_string(/*(int *)generate_field(),*/ checksum_str);
    
    // coordinates for the battlefield
    int x_received = 0;
    int y_received = 0;

    // Win and Loose counter
    uint8_t win_count = 0;
    uint8_t loose_count = 0;

    int gameover = 0;

    // counts hits
    uint8_t hit_counter;
    uint8_t gothit_counter;


    int ret; 
    // main loop
    for(;;)
    {    
        // configure PWM
        TIM3->ARR = arr_from_freq(NOTE_A);
        TIM3->CCR3 = TIM3->ARR / 2;
        

        uint8_t byte;
        ret = fifo_get((Fifo_t *)&usart_rx_fifo, &byte) == 0;
        if (ret)
        //if(fifo_get((Fifo_t *)&usart_rx_fifo, &byte) == 0)
        {              
            // Save byte in match_buffer
            match_buffer[match_index] = byte;
            match_index++;

            if(byte == '\n')
            {
                match_buffer[match_index] = '\0';

                if (strncmp(match_buffer, "HD_START", 8) == 0)
                {
                    state = 1;
                } else if (strncmp(match_buffer, "HD_CS_", 5) == 0)
                {
                    state = 2;
                } else if (strncmp(match_buffer, "HD_BOOM_", 7) == 0)
                {
                    if(match_buffer[8] == 'M' || match_buffer[8] == 'H')
                    {
                        state = 4; // Hit or Miss
                    }
                    else
                    { 
                        // Extract coordinates from match_buffer
                        sscanf(match_buffer, "HD_BOOM_%d_%d", &x_received, &y_received);
                        state = 3; // Shoot
                    }
                } else if (strncmp(match_buffer, "HD_SF", 5) == 0)
                {
                    if((match_buffer[5] == '9') && (gameover == 0))
                    {
                        // Win or Loss
                        state = 5; 
                        win_count++;
                        TIM3->CR1 |= TIM_CR1_CEN;
                    } else
                    {
                        state = 0;
                    }
                }
                else
                {
                    LOG("Unknown command: %s\n", match_buffer);
                    state = 0; // Reset state for unknown commands
                }
                match_index = 0;
            }
            bytes_recv++;
            
        }
        // switch case for the state model
        switch (state){
            // Receiving/sending start message
            case 1:
                LOG("DH_START_Krapfen\n");
                init_fields();
                hit_counter = 0;
                gothit_counter = 0;
                gameover = 0;
                state = 0;
                break;
            // Sending checksum
            case 2: 
                LOG("DH_CS_%s\n", checksum_str);
                state = 0;
                break;
            // Shooting phase | Check if Hit or Miss and send coordinates of target
            case 3:
                //TIM3->CR1 |= TIM_CR1_CEN;
                if(check_hit(x_received, y_received))
                {
                    gothit_counter++;
                    if(gothit_counter == 29)
                    {
                        loose_count++;
                        state = 5;
                        break;
                    }
                    LOG("DH_BOOM_H\n");
                } else
                {
                    LOG("DH_BOOM_M\n");
                }
                int x = 0;
                int y = 0;
                shoot(enemy_field, &x, &y);
                enemy_field[y * 10 + x] = 3; // pending shot
                
                LOG("DH_BOOM_%d_%d\n", x, y);
                
                state = 0;
                break;
            // Receiving Hit or Miss and updating enemy field
            case 4:
                // Check if Hit or Miss has been send correctly
                if (match_buffer[8] == 'H')
                {
                    // Update enemy field to mark hit
                    enemy_field[y * 10 + x] = 2; // 2 for hit
                    hit_counter++;
                }
                else
                {
                    // Update enemy field to mark miss
                    enemy_field[y * 10 + x] = 1; // 1 for water
                }
                state = 0;
                break;
            // Sending final field and win message
            case 5:
            // Send final field
                send_battlefield();
                gameover = 1;
                if((win_count + loose_count) == 98) // 100 - 2 (iteration starts at 0)
                {
                    if(win_count < loose_count)
                    {
                       play_victory_tune();
                       TIM3->CCR3 = 0;
                    }
                    win_count = 0;
                    loose_count = 0;
                }
                TIM3->CR1 &= ~TIM_CR1_CEN;
                state = 0;
                continue;
        }
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