/*
 * HD44780 16x2 LCD Driver - Two-Way Text Bounce (Ping-Pong Marquee)
 * Board  : Nucleo-F446RE (STM32F446RE, ARM Cortex-M4)
 * Output : HD44780-based 16x2 LCD in 4-bit mode
 *          RS = PA0, E = PA1, D4..D7 = PA4..PA7, RW = GND
 * Bare-metal program (no HAL): the text shifts left with 0x18
 *          for SHIFT_COUNT counts, then reverses and shifts right with 0x1C.
 */
#include <stdint.h>

/* Register Address = Peripheral Base Address + Register Offset
 * RCC_AHB1ENR = 0x40023800 + 0x30 = 0x40023830
 * GPIOA_MODER = 0x40020000 + 0x00 = 0x40020000
 * GPIOA_ODR   = 0x40020000 + 0x14 = 0x40020014
 * volatile: forces every access to read/write the real hardware address.
 */
#define RCC_BASE     0x40023800UL
#define GPIOA_BASE   0x40020000UL

#define RCC_AHB1ENR  (*(volatile uint32_t *)(RCC_BASE + 0x30))
#define GPIOA_MODER  (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_ODR    (*(volatile uint32_t *)(GPIOA_BASE + 0x14))

#define RS 0   // PA0 (LCD connections)
#define E  1   // PA1
#define D4 4   // PA4
#define D5 5   // PA5
#define D6 6   // PA6
#define D7 7   // PA7

/* Free columns on each line: 16 - 11 characters = 5 */
#define SHIFT_COUNT 5

void LCD_Init(void);
void LCD_4bit(unsigned char x);
void LCD_Command(unsigned char cmd);
void LCD_Data(unsigned char data);
void LCD_Print(char *s);
void delay(void);

int main(void)
{
    LCD_Init();

    /* 5 leading spaces place the 11-character text at the right edge,
       so it can move 5 positions left and come back without leaving the screen */
    LCD_Print("     Hello STM32");   // Text on the 1st line
    LCD_Command(0xC0);               // Force cursor to the beginning of the 2nd line
    LCD_Print("     ECE 350 CBU");   // Text on the 2nd line

    while (1)
    {
        /* Shift left SHIFT_COUNT times */
        for (int i = 0; i < SHIFT_COUNT; i++)
        {
            LCD_Command(0x18);   // Shift the entire display to the left
            delay();
        }

        /* Reverse direction: shift right SHIFT_COUNT times */
        for (int i = 0; i < SHIFT_COUNT; i++)
        {
            LCD_Command(0x1C);   // Shift the entire display to the right
            delay();
        }
    }
}

void LCD_Init(void)
{
    /* Enable GPIOA clock (RCC_AHB1ENR bit 0 = 1) */
    RCC_AHB1ENR |= (1<<0);

    /* Configure PA0, PA1, PA4-PA7 = output (01 in each MODER field) */
    GPIOA_MODER |= (1<<(2*RS)) | (1<<(2*E))  |
                   (1<<(2*D4)) | (1<<(2*D5)) |
                   (1<<(2*D6)) | (1<<(2*D7));
    delay();

    /* Initialize LCD to 4-bit mode */
    LCD_4bit(0x03);      // 0x03 sent 3 times resets the controller to a known 8-bit state
    LCD_4bit(0x03);
    LCD_4bit(0x03);
    LCD_4bit(0x02);      // 0x02 switches to 4-bit mode: each byte is sent as two nibbles

    LCD_Command(0x28);   // 4-bit, 2-line
    LCD_Command(0x0C);   // Display ON, cursor OFF
    LCD_Command(0x06);   // Cursor moves right
    LCD_Command(0x01);   // Clear display
    delay();
}

/* Send 4-bits to LCD */
void LCD_4bit(unsigned char x)
{
    // Clear the output lines D7-D4
    GPIOA_ODR &= ~((1<<D4) | (1<<D5) | (1<<D6) | (1<<D7));

    // Extract & send the 4-bit value
    if (x & 1) GPIOA_ODR |= (1<<D4);   // PA4
    if (x & 2) GPIOA_ODR |= (1<<D5);   // PA5
    if (x & 4) GPIOA_ODR |= (1<<D6);   // PA6
    if (x & 8) GPIOA_ODR |= (1<<D7);   // PA7

    GPIOA_ODR |= (1<<E);     // E = 1
    delay();
    GPIOA_ODR &= ~(1<<E);    // E = 0 (High-to-Low latches the data)
    delay();
}

/* Send command */
void LCD_Command(unsigned char cmd)
{
    GPIOA_ODR &= ~(1<<RS);   // RS = 0 -> command register
    LCD_4bit(cmd >> 4);      // Send high nibble of the command
    LCD_4bit(cmd & 0x0F);    // Send low nibble of the command
    delay();
}

/* Send character */
void LCD_Data(unsigned char data)
{
    GPIOA_ODR |= (1<<RS);    // RS = 1 -> data register
    LCD_4bit(data >> 4);
    LCD_4bit(data & 0x0F);
    delay();
}

/* Display string */
void LCD_Print(char *s)
{
    while (*s)
        LCD_Data(*s++);
}

/* Simple delay
 * Delay Time = (Number of iterations x Cycles per iteration) / CPU Clock Frequency
 * (50,000 x 9) / 16,000,000 = about 28 ms (about 9 cycles per iteration on this board)
 */
void delay(void)
{
    for (volatile int i = 0; i < 50000; i++);
}
