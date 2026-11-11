# STM32 HD44780 LCD Driver

A bare-metal C driver for an **HD44780-based 16x2 character LCD** on the **NUCLEO-F446RE** (STM32F446RE, ARM Cortex-M4), running in **4-bit mode**. The demo prints two lines of text and bounces them left and right across the screen like a ping-pong marquee.

Written with **direct register access only**: no HAL, no CMSIS headers. GPIO and clock registers are reached through their memory-mapped addresses.

<!-- Demo GIF goes here: ![Demo](media/demo.gif) -->

## Features

- **4-bit interface:** only 6 GPIO pins drive the display (RS, E and D4–D7) instead of 10 in 8-bit mode.
- **Small driver API:** `LCD_Init`, `LCD_Command`, `LCD_Data` and `LCD_Print` cover initialization, commands and text output.
- **Two-line text:** `0xC0` moves the cursor to the start of line 2.
- **Bounce animation:** the whole display shifts left (`0x18`) 5 times, then right (`0x1C`) 5 times, forever. Each 11-character string is padded so it never leaves the 16-column screen.

## Hardware

| Component | Details |
|---|---|
| Board | NUCLEO-F446RE (STM32F446RE, 16 MHz HSI) |
| Display | 16x2 character LCD, HD44780-compatible controller |
| Contrast | Potentiometer on the LCD's V0 pin |

### Wiring

| LCD pin | MCU pin | Nucleo header |
|---|---|---|
| RS | PA0 | A0 |
| E | PA1 | A1 |
| D4 | PA4 | A2 |
| D5 | PA5 | D13 |
| D6 | PA6 | D12 |
| D7 | PA7 | D11 |
| R/W | GND | GND |
| D0–D3 | Not connected | |

R/W is tied to ground, so the program only writes to the LCD and never reads its busy flag. PA5 also drives the board's green user LED (LD2), so it flickers while data is sent. That's expected.

## How it works

1. **GPIO setup:** enables the GPIOA clock in `RCC_AHB1ENR` and sets PA0, PA1 and PA4–PA7 to output in `GPIOA_MODER`.
2. **Initialization:** sends `0x03` three times to put the controller in a known state, then `0x02` to switch to 4-bit mode. It then configures the display with `0x28` (4-bit, 2 lines), `0x0C` (display on, cursor off), `0x06` (cursor moves right) and `0x01` (clear).
3. **4-bit transfers:** every byte is split into two nibbles, high nibble first (`x >> 4`, then `x & 0x0F`). Each nibble is placed on D4–D7 and latched by pulsing E high then low.
4. **Command vs. data:** `RS = 0` writes to the command register, `RS = 1` writes a character.
5. **Timing:** without the busy flag, a software `delay()` loop (about 28 ms at 16 MHz) gives the LCD time to finish each operation. It also sets the animation speed.

## LCD commands used

| Command | Meaning |
|---|---|
| `0x28` | 4-bit interface, 2 lines, 5x8 font |
| `0x0C` | Display on, cursor off |
| `0x06` | Entry mode: cursor moves right |
| `0x01` | Clear display |
| `0xC0` | Move cursor to line 2, column 0 |
| `0x18` | Shift entire display left |
| `0x1C` | Shift entire display right |

## Build and flash

1. In **STM32CubeIDE**, create a new **STM32 Project** for the **NUCLEO-F446RE** board, choosing the **Empty** project type.
2. Replace the generated `Src/main.c` with [`Src/main.c`](Src/main.c) from this repo.
3. Build, connect the board over USB, and run it. Adjust the contrast potentiometer if the screen looks blank.

## Project structure

```
.
├── Src/
│   └── main.c     # LCD driver functions and the bounce demo
├── LICENSE
└── README.md
```

## Concepts used

Memory-mapped I/O · RCC clock gating · GPIO MODER/ODR · HD44780 4-bit protocol · Nibble splitting · Software timing · Bit masking

## License

[MIT](LICENSE)
