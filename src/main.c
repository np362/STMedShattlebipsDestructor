#include <stm32f091xc.h>
#include <string.h>
#include <stdio.h>

/** =====================================
 * 
 *          FIFO Configuration
 * 
 ========================================*/
#define FIFO_SIZE 64
#define FIFO_ERROR -1

typedef struct {
    uint8_t buffer[FIFO_SIZE];
    uint16_t head;
    uint16_t tail;
} Fifo_t;

void fifo_init(Fifo_t* fifo);
int fifo_put(Fifo_t* fifo, uint8_t data);
int fifo_get(Fifo_t* fifo, uint8_t* data);

void fifo_init(Fifo_t* fifo) {
    fifo->head = 0;                              // Initialize head pointer to 0
    fifo->tail = 0;                              // Initialize tail pointer to 0
}

uint8_t fifo_is_empty(Fifo_t* fifo) {
    return (fifo->head == fifo->tail);          // FIFO is empty if head and tail are equal
}

uint8_t fifo_is_full(Fifo_t* fifo) {
    return ((fifo->head + 1) % FIFO_SIZE) == fifo->tail; // FIFO is full if incrementing head would equal tail
}

int fifo_put(Fifo_t* fifo, uint8_t data) {
    if (fifo_is_full(fifo)) {                   // Check if FIFO is full before inserting
        return -1;                               // Insertion failed (buffer full)
    }

    fifo->buffer[fifo->head] = data;            // Store data at current head position
    fifo->head = (fifo->head + 1) % FIFO_SIZE;  // Move head forward and wrap around if needed
    return 0;                                   // Insertion successful
}

int fifo_get(Fifo_t* fifo, uint8_t* data) {
    if (fifo_is_empty(fifo)) {                  // Check if FIFO is empty before reading
        return -1;                               // Read failed (buffer empty)
    }

    *data = fifo->buffer[fifo->tail];           // Retrieve data at current tail position
    fifo->tail = (fifo->tail + 1) % FIFO_SIZE;  // Move tail forward and wrap around if needed
    return 0;                          // Read successful Return 0
}

// FIFO variables
volatile Fifo_t usart_rx_fifo;
#define TARGET_LEN 20
char match_buffer[TARGET_LEN] = {0};
uint8_t match_index = 0;

/**======================================
 * 
 *          Clock Configuration
 * 
========================================= */
#ifndef EPL_CLOCK_H
#define EPL_CLOCK_H

#define APB_FREQ 48000000
#define AHB_FREQ 48000000

void SystemClock_Config(void);

#endif // EPL_CLOCK_H

/**
 * @brief  System Clock Configuration
 *         The system Clock is configured as follow :
 *            System Clock source            = PLL (HSI48)
 *            SYSCLK(Hz)                     = 48000000
 *            HCLK(Hz)                       = 48000000
 *            AHB Prescaler                  = 1
 *            APB1 Prescaler                 = 1
 *            HSI Frequency(Hz)              = 48000000
 *            Flash Latency(WS)              = 1
 * @param  None
 * @retval None
 */
void SystemClock_Config(void)
{
  // Reset the Flash 'Access Control Register', and
  // then set 1 wait-state and enable the prefetch buffer.
  // (The device header files only show 1 bit for the F0
  //  line, but the reference manual shows 3...)
  FLASH->ACR &= ~(FLASH_ACR_LATENCY_Msk | FLASH_ACR_PRFTBE_Msk);
  FLASH->ACR |= (FLASH_ACR_LATENCY |
                 FLASH_ACR_PRFTBE);

  // activate the internal 48 MHz clock
  RCC->CR2 |= RCC_CR2_HSI48ON;

  // wait for clock to become stable before continuing
  while (!(RCC->CR2 & RCC_CR2_HSI48RDY))
    ;

  // configure the clock switch
  RCC->CFGR = RCC->CFGR & ~RCC_CFGR_HPRE_Msk;
  RCC->CFGR = RCC->CFGR & ~RCC_CFGR_PPRE_Msk;
  RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW_Msk) | (0b11 << RCC_CFGR_SW_Pos);

  // wait for clock switch to become stable
  while ((RCC->CFGR & RCC_CFGR_SWS) != (0b11 << RCC_CFGR_SWS_Pos))
    ;
}


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

// Define melodies for the end of the tournament
const uint16_t lvlup_notes[] = {NOTE_E, NOTE_G, 2*NOTE_E, 2*NOTE_C, 2*NOTE_D, 2*NOTE_G};
const int lvlup_length[] = {1000000, 1000000, 1000000, 800000, 800000, 1000000};
const uint16_t gameover_notes[] = {2*NOTE_G, 2*NOTE_D, 2*NOTE_D, 2*NOTE_D, 2*NOTE_C, 2*NOTE_B, 2*NOTE_G, 2*NOTE_E, 2*NOTE_E, NOTE_C};
const int gameover_length[] = {
  400000, // G5 Achtel
  800000, // D6 Viertel
  400000, // D6 Achtel
  600000, // D6 Triole
  600000, // C6 Triole
  600000, // B5 Triole
  400000, // G5 Viertel
  800000, // E5 Achtel
  400000, // E5 Achtel
  800000  // C6 punktierte Viertel
};


// Field constant
#define FIELD_SIZE 10
// Different ship types
#define SHIP_TYPES 4

const uint8_t ship_size[SHIP_TYPES] = {5, 4, 3, 2}; // Different ship types
const uint8_t ship_count[SHIP_TYPES] = {1, 2, 3, 4}; // Amount of ship type 

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

    TIM3->PSC = 47; // 1 MHz Timer-Takt (48 MHz / (47+1))
    TIM3->ARR = arr_from_freq(NOTE_A);
    TIM3->CCR3 = TIM3->ARR / 2;

    TIM3->CCMR2 |= (0b110 << 4); // PWM Mode 1
    TIM3->CCER |= TIM_CCER_CC3E;
    TIM3->CR1 &= ~TIM_CR1_CEN;
}

/**
 * @brief Initialize timer for field generator
 */
void TIMER_INIT(){
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2->PSC = 47; // 1 MHz Timer-Takt (48 MHz / (47+1)) -> 1 count = 1 µs
    TIM2->ARR = 179999999; // for 3 minutes of counting
    TIM2->CR1 |= TIM_CR1_CEN;
}

/**
 * @brief Delays the program execution for a specified amount of time.
 * @param time The amount of time to delay in number of cycles.
 * @return 0 when the delay is completed.
 */
int delay(uint32_t time){
    for(uint32_t i = 0; i < time; i++ ){
        asm("nop"); // No operation, used for delaying
    }
    return 0;
}

/** ================================================
 * 
 *              Sound specifications
 * 
 ===================================================*/

/**
 * @brief plays a victory tune (Mario level up)
 */
void play_victory_tune()
{
    TIM3->CR1 |= TIM_CR1_CEN;
    for(int idx = 0; idx < 6; idx++)
    {
        TIM3->ARR = arr_from_freq(lvlup_notes[idx]);
        TIM3->CCR3 = TIM3->ARR / 2;
        delay(lvlup_length[idx]);
    }
    
}

/**
 * @brief plays a defeat tune (Mario death sound)
 */
void play_defeat_tune()
{
    TIM3->CR1 |= TIM_CR1_CEN;
    for(int idx = 0; idx < 9; idx++)
    {
        TIM3->ARR = arr_from_freq(gameover_notes[idx]);
        TIM3->CCR3 = TIM3->ARR / 2;
        delay(300000); // constant playing length
        TIM3->CR1 &= ~(TIM_CR1_CEN); // deactivate timer for staccato effect
        delay(gameover_length[idx]); // variable pause length
        TIM3->CR1 |= TIM_CR1_CEN;
    }
}

/** =========================================
 *              
 *              Game functionalities
 * 
 ============================================*/

// Initialize own field and enemy_field as global variables
int field[FIELD_SIZE*FIELD_SIZE] = {0};
int enemy_field[FIELD_SIZE*FIELD_SIZE] = {0};

/**
 * @brief Generates field constellation of ships
 * @return battlefield as int array
*/
void init_fields() {

    // Initialize reset own field and enemy field
    for(int i = 0; i < FIELD_SIZE*FIELD_SIZE; i++) {
        field[i] = 0;
        enemy_field[i] = 0;
    }
    // generate new field
    generate_field();
}

 /**
  * @brief get "random" number from counter
  * @return Timer 2 Counter value 
  */
uint32_t get_random(){
    return TIM2->CNT;
}

/**
 * @brief Check if ship position can be set correctly 
 * @param Coords x and y, length of ship, horizontal (True/False)
 * @return True or False
 */
int valid_ship(int x, int y, int len, int horizontal) {
    // Iterate through length of ship
    for (int i = 0; i < len; i++) {
        int xi, yi;
        // check if horizontal or vertical
        if (horizontal) {
            xi = x + i;
            yi = y;
        } else {
            xi = x;
            yi = y + i;
        }
        // validate coords are within the Field
        if (xi < 0 || xi >= FIELD_SIZE || yi < 0 || yi >= FIELD_SIZE)
            return 0;

        // Check all neighbouring Fields (x-1,y-1 | x, y-1 | x+1, y-1 ; etc.)
        // When all are 0 the condition of placement is met
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                int nx = xi + dx;
                int ny = yi + dy;
                if (nx >= 0 && nx < FIELD_SIZE && ny >= 0 && ny < FIELD_SIZE) {
                    // If != 0 there is already a ship placed
                    if (field[ny * FIELD_SIZE + nx] != 0)
                        return 0;
                }
            }
        }
    }
    // all neighbouring fields are 0
    return 1;
}

/**
 * @brief generate field
 */
int generate_field() {
    int tries = 0;
    // max 10 tries to generate field
    while (tries++ < 10) { 
        // resetting field if failed attempt
        for (int i = 0; i < FIELD_SIZE * FIELD_SIZE; i++)
            field[i] = 0; 

        int placed_cells = 0;
        // Iterate through all ship_types (2, 3, 4, 5 -> sum 4)
        for (int t = 0; t < SHIP_TYPES; t++) {
            // Iterate through amount of each ship type
            for (int count = 0; count < ship_count[t]; count++) {
                // define length of each ship (2, 3, 4, 5)
                int len = ship_size[t]; 
                int placed = 0;
                int attempts = 0;
                // max 1000 tries to place ship on field
                while (!placed && attempts++ < 1000) {
                    // get a "random" number from timer 2 counter
                    uint32_t number_cnt = get_random();
                    int horizontal = number_cnt % 2; // 0 is vertical, 1 is horizontal
                    int x, y;
                    // check if horizontal or not -> define direction of ship, either x or y
                    if(horizontal){
                        x = number_cnt % (FIELD_SIZE - len + 1);
                        y = (number_cnt >> 4) % FIELD_SIZE;
                    } else {
                        x = number_cnt % FIELD_SIZE;
                        y = (number_cnt >> 4) % (FIELD_SIZE - len + 1);
                    }
                    // check if ship_placement is valid
                    if (valid_ship(x, y, len, horizontal)) {
                        // if ship placement is valid the ship can be placed
                        for (int i = 0; i < len; i++) {
                            int xi, yi;
                            if (horizontal) {
                                xi = x + i;
                                yi = y;
                            } else {
                                xi = x;
                                yi = y + i;
                            }
                            field[yi * FIELD_SIZE + xi] = len;
                        }
                        placed = 1;
                        placed_cells += len;
                    }
                }
                if (!placed) break; // cancel if ship can't be placed
            }
        }
        if (placed_cells == 30) return 1; // true if all 30 fields are set
    }
    return 0;
}


/**
 * @brief Calculates checksum of battlefield
 */
void checksum_string(char *output) {
    for (int row = 0; row < FIELD_SIZE; row++) {
        int count = 0;
        for (int col = 0; col < FIELD_SIZE; col++) {
            if (field[row * FIELD_SIZE + col] != 0) {
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
void shoot(int *x, int *y){
    for (int i = 0; i < FIELD_SIZE*FIELD_SIZE; i++){
        if((enemy_field[i] != 1) && (enemy_field[i] != 2)) // 1 is water, 2 is hit, 0 is unknown
        {    
            // if the field is not empty, return the coordinates
            *x = i % FIELD_SIZE; // x coordinate
            *y = i / FIELD_SIZE; // y coordinate
            break;
        }
    }
}

/**
 * @brief Better algorithm for shooting
 * @return target coords x and y
 */
void shoot2(int *x, int *y)
{
    // check if last shot was a hit
    if(enemy_field[*x + *y*FIELD_SIZE] == 2){
        if((*y+1 < FIELD_SIZE && enemy_field[*x + (*y+1)*FIELD_SIZE] == 0) || (*x>0 && enemy_field[(*x-1) + *y*FIELD_SIZE] != 2 && *y+1 < FIELD_SIZE)){
            *y = *y + 1;
            return;
        }
    }
    for (int i = 0; i < FIELD_SIZE*FIELD_SIZE; i++){
        if((enemy_field[i] != 1) && (enemy_field[i] != 2)) // 1 is water, 2 is hit, 0 is unknown
        {    
            // if the field is not empty, return the coordinates
            *x = i % FIELD_SIZE; // x coordinate
            *y = i / FIELD_SIZE; // y coordinate
            return;
        }
    }
}

// void shoot3(int *x, int *y)
// {
//     static int state = 0;
//     static int last_hit_x = -1, last_hit_y = -1;
//     static int direction = 0;
//     static int chain_length = 0;

//     if(enemy_field[*y*FIELD_SIZE + *x] == 2)
//     {
//         last_hit_x = *x;
//         last_hit_y = *y;
//         chain_length++;
//         if (direction == 0) {
//             if (*x + 1 < FIELD_SIZE && enemy_field[*y * FIELD_SIZE + (*x + 1)] == 0){
//                 direction = 1; 
//                 state = 1;
//             }
//              else if (*y + 1 < FIELD_SIZE && enemy_field[(*y + 1) * FIELD_SIZE + *x] == 0){
//                 direction = 2; 
//                 state = 2;
//             }
//              else{
//                 direction = 0;
//                 state = 0;
//                 chain_length = 0;
//              }
//         }
//     }
//     if(*x == 9 && *y == 9){state = 5; direction = 5;}

//     switch(state)
//     {
//         case 0:
//             for (int col = 0; col < FIELD_SIZE; col++) {
//                     for (int row = 0; row < FIELD_SIZE; row++) {
//                         if ((col % 2 == 0 && row % 2 == 0) || (col % 2 == 1 && row % 2 == 1)) { // odd row = odd col number and vice versa
//                             if(*x == col && *y == row){continue;}
//                             if (enemy_field[row * FIELD_SIZE + col] == 0) {
//                                 *x = col;
//                                 *y = row;
//                                 state = 0;
//                                 return;
//                             }
//                         }
//                     }
//                 }
//             break;
//         case 1: // Todo: Decide whether function calling or big if statements in state machine would be better
//             if((*x+1) < FIELD_SIZE && enemy_field[(*x+1) + *y*FIELD_SIZE] == 0 && enemy_field[*x + *y*FIELD_SIZE == 2]){
//                 *y = last_hit_y;
//                 *x = last_hit_x+1;
//                 return;
//             } else if(enemy_field[*x + *y*FIELD_SIZE] == 1 || enemy_field[(*x+1) + *y*FIELD_SIZE] != 0){// || (*x+1) >= FIELD_SIZE){
//                 if((last_hit_y+1) < FIELD_SIZE && enemy_field[last_hit_x + (last_hit_y+1)*FIELD_SIZE] == 0 && chain_length <= 1){
//                     chain_length = 0;
//                     *y = last_hit_y+1;
//                     *x = last_hit_x;
//                     state = 2;
//                     direction = 2;
//                     return;
//                 } 
//                 else if(FIELD_SIZE - *x - chain_length - 1 > 0 && enemy_field[(last_hit_x-chain_length) + last_hit_y*FIELD_SIZE] != 0){
//                     *y = last_hit_y;
//                     *x = last_hit_x - chain_length;
//                     chain_length = 0;
//                     state = 3;
//                     direction = 3;
//                     return;
//                 }
//             }
//             state = 5;
//             direction = 5;
//             break;

//         case 2:

//             *y = last_hit_y + 1;
//             *x = last_hit_x;
//             state = 0;
//             return;

//         case 3:
//             *x = last_hit_x - (chain_length + 1);
//             *y = last_hit_y;
//             chain_length = 0;
//             last_hit_x = *x;
//             state = 0;
//             return;

//         case 4:
//             *y = last_hit_y - (chain_length + 1);
//             *x = last_hit_x;
//             chain_length = 0;
//             last_hit_y = *y;            
//             state = 0;
//             return;

//         case 5:
//             for (int col = 0; col < FIELD_SIZE; col++) {
//                 for (int row = 0; row < FIELD_SIZE; row++) {
//                     if ((col % 2 == 0 && row % 2 == 1) || (col % 2 == 1 && row % 2 == 0)) {
//                         if (enemy_field[row * FIELD_SIZE + col] == 0) {
//                             *x = col;
//                             *y = row;
//                             //state = 0;
//                             return;
//                         }
//                     }
//                 }
//             }
//             break;
//     }
// }


/**
 * @brief updates the enemy field
 */
void update_enemy_field(int x, int y, char *checksum){
    // checking for horizontal ships
    if (x > 0 && enemy_field[(x-1) + y*FIELD_SIZE] == 2) {
        // set the neighbouring fields to 1 (water)
        if (y+1 < FIELD_SIZE) enemy_field[x + (y+1)*FIELD_SIZE] = 1;
        if (x-2 >= 0) {
            if (enemy_field[x-2 + y*FIELD_SIZE] != 2) enemy_field[x-2 + y*FIELD_SIZE] = 1;
            if (y > 0) enemy_field[x-2 + (y-1)*FIELD_SIZE] = 1;
            if (y < FIELD_SIZE-1) enemy_field[x-2 + (y+1)*FIELD_SIZE] = 1;
        }
        if (y > 0 && x-1 >= 0) enemy_field[x-1 + (y-1)*FIELD_SIZE] = 1;
        if (y < FIELD_SIZE-1 && x-1 >= 0) enemy_field[x-1 + (y+1)*FIELD_SIZE] = 1;
    }
    // checking for vertical ships
    else if (y > 0 && enemy_field[x + (y-1)*FIELD_SIZE] == 2) {
        // set the neighbouring fields to 1 (water)
        if (x+1 < FIELD_SIZE) enemy_field[x+1 + y*FIELD_SIZE] = 1;
        if (y-2 >= 0) {
            if (enemy_field[x + (y-2)*FIELD_SIZE] != 2) enemy_field[x + (y-2)*FIELD_SIZE] = 1;
            if (x > 0) enemy_field[x-1 + (y-2)*FIELD_SIZE] = 1;
            if (x < FIELD_SIZE-1) enemy_field[x+1 + (y-2)*FIELD_SIZE] = 1;
        }
        if (x > 0) enemy_field[x-1 + (y-1)*FIELD_SIZE] = 1;
        if (x < FIELD_SIZE-1) enemy_field[x+1 + (y-1)*FIELD_SIZE] = 1;
    }

    int sum = 0;
    for(int row = 0; row < 11; row++){
        // convert ASCII value to int
        sum = checksum[row] - '0';
        int count = 0;
        for(int col = 0; col < FIELD_SIZE; col++){
            if(enemy_field[col + row*FIELD_SIZE] == 2){
                count++;
                // check if marked hits equal checksum of enemy
                if(count == sum){
                    for(int check = 0; check < FIELD_SIZE; check++){
                        if(enemy_field[check + row*FIELD_SIZE] == 0){
                            enemy_field[check + row*FIELD_SIZE] = 1;
                        }
                    }
                }
            }
        }
    }
}

/**
 * @brief updates the enemy field by comparing the checksum
 */
void update_field_checksum(char *buffer){
    for(int cif = 0; cif < 11; cif++){
        if((buffer[cif]-'0') == 0){
            for(int col = 0; col < FIELD_SIZE; col++){
                enemy_field[col + (cif)*FIELD_SIZE] = 1;
            }
        }
    }
}

/**
 * @brief Checks if the shot was a hit or miss
 * @return True or False
 */
int check_hit(uint8_t x, uint8_t y) {
    return (field[y * FIELD_SIZE + x] != 0);
}

/**
 * @brief sends own battlefield to schiff.py at the end of the game
 */
void send_battlefield()
{
    for(int i = 0; i < FIELD_SIZE; i++)
    {
        LOG("DH_SF%dD", i);
        for(int j = 0; j < FIELD_SIZE; j++)
        {
            LOG("%d", field[i * FIELD_SIZE + j]);
        }
        LOG("\n");
    }
}


/**
*   @brief main function
*   @return 0
*/
int main(void)
{
    // Initializes clock, UART, PWM and timer 
    SystemClock_Config();
    UART_INIT();
    PWM_INIT();
    TIMER_INIT();

    NVIC_SetPriorityGrouping(0);                               // Use 4 bits for priority, 0 bits for subpriority
    uint32_t uart_pri_encoding = NVIC_EncodePriority(0, 1, 0); // Encode priority: group 1, subpriority 0
    NVIC_SetPriority(USART2_IRQn, uart_pri_encoding);          // Set USART2 interrupt priority
    NVIC_EnableIRQ(USART2_IRQn);                               // Enable USART2 interrupt

    fifo_init((Fifo_t *)&usart_rx_fifo);                       // Init the FIFO

    uint32_t bytes_recv = 0;

    // Case switch
    int state = 0;

    // Initialize checksum_str variable
    char checksum_str[11];
    char checksum_enemy[11];

    // coordinates for the battlefields
    int x_received = 0;
    int y_received = 0;
    int x = 0;
    int y = 0;

    // Win and Loose counter
    uint8_t win_count = 0;
    uint8_t loose_count = 0;

    // variable for checking if game has already ended
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
            // Receiving/sending start message, resetting all values
            case 1:
                LOG("DH_START_Krapfen\n");
                TIM2->CNT = 0;
                TIM2->CR1 |= TIM_CR1_CEN;
                init_fields();
                checksum_string(checksum_str);
                hit_counter = 0;
                gothit_counter = 0;
                gameover = 0;
                x = 0;
                y = 0;
                state = 0;
                break;
            // Sending checksum and saving checksum of enemy
            case 2: 
                LOG("DH_CS_%s\n", checksum_str);
                for(int cif = 6; cif < 17; cif++){
                    checksum_enemy[cif-6] = match_buffer[cif];
                }
                update_field_checksum(checksum_enemy);
                state = 0;
                break;

            // Shooting phase | Check if Hit or Miss and send coordinates of target
            case 3:
                if(check_hit(y_received, x_received))
                {
                    gothit_counter++;
                    if(gothit_counter == 30)
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
                
                //shoot3(&x, &y);
                shoot2(&x, &y);
                //shoot(&x, &y);
                
                LOG("DH_BOOM_%d_%d\n", y, x);
                
                state = 0;
                break;

            // Receiving Hit or Miss and updating enemy field
            case 4:
                // Check if Hit or Miss has been send correctly
                if (match_buffer[8] == 'H')
                {
                    // Update enemy field to mark hit
                    enemy_field[y * FIELD_SIZE + x] = 2; // 2 for hit
                    update_enemy_field(x, y, checksum_enemy);
                    hit_counter++;
                }
                else
                {
                    // Update enemy field to mark miss
                    enemy_field[y * FIELD_SIZE + x] = 1; // 1 for water
                }
                state = 0;
                break;

            // Sending final field and win message
            case 5:
                // Send final field
                send_battlefield();
                gameover = 1; // setting gameover which means that the game is done
                if((win_count + loose_count) == 100)
                //if((win_count + loose_count) == 10) // only to test melody
                {
                    if(win_count > loose_count)
                    {
                       play_victory_tune();
                       TIM3->CCR3 = 0;
                    } else if(win_count < loose_count)
                    {
                        play_defeat_tune();
                        TIM3->CCR3 = 0;
                    }
                    win_count = 0;
                    loose_count = 0;
                }
                TIM3->CR1 &= ~TIM_CR1_CEN;
                TIM2->CR1 &= ~TIM_CR1_CEN;
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