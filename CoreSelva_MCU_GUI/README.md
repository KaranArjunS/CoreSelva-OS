# CoreSelva MOS

CoreSelva MOS is a tiny bare-metal STM32WB55 project that turns a 1.8 inch ST7735 TFT into a small phone-style interface. It has a boot splash, home screen, app drawer, Snake game, shutdown screen, and an LED app that controls an external LED on PA0.

This project is written in embedded C without STM32 HAL. It talks directly to STM32 registers, so it is useful for learning how GPIO, SPI, SysTick, a simple scheduler, and a TFT display work together.

## Features

- CoreSelva OS boot splash
- Black, yellow, and white theme
- Phone-style home screen with app icons
- App drawer
- Snake game
- LED app
- Shutdown screen
- Six-button navigation
- Cooperative task scheduler
- ST7735 display driver over SPI
  
## Demo

<img width="1920" height="1080" alt="Videoshot_20260706_214916" src="https://github.com/user-attachments/assets/5144506e-f8d8-4287-b633-07ba158a6de3" />
<img width="1920" height="1080" alt="Videoshot_20260706_214944" src="https://github.com/user-attachments/assets/810bc6df-2a4d-4a61-9790-03f0a5d7c966" />
<img width="1920" height="1080" alt="Videoshot_20260706_214929" src="https://github.com/user-attachments/assets/c028e2a7-739e-4ff5-9224-e08a4e6c1f0d" />
<img width="1920" height="1080" alt="Videoshot_20260706_215008" src="https://github.com/user-attachments/assets/c64133aa-6848-42d5-a3a6-c119bd27c7a6" />

## Hardware

Target MCU:

- STM32WB55CGUX

Display:

- 1.8 inch ST7735 TFT, 128 x 160, SPI

TFT wiring used by this project:

| TFT signal | STM32 pin |
|---|---|
| SCK / SCL | PA5 |
| SDA / MOSI | PA7 |
| CS | PA4 |
| DC / A0 | PB0 |
| RESET | PB1 |
| VCC | 3.3 V |
| GND | GND |

Button wiring:

| Action | STM32 pin |
|---|---|
| Up | PB7 |
| Down | PB6 |
| Left | PB5 |
| Right | PB4 |
| Back / Cancel | PB3 |
| Enter / Open | PA15 |

The buttons are active-low. Connect one side of each button to the STM32 pin and the other side to GND. The firmware enables internal pull-up resistors.

LED wiring:

| LED signal | STM32 pin |
|---|---|
| LED output | PA0 |

Use a resistor in series with the LED.

## Controls

Home screen:

- Use Up/Down/Left/Right to move between icons.
- Press Enter on an icon to open it.
- Press Enter on `MENU` to open the app drawer.

Apps:

- `SNAKE`: play Snake.
- `LED`: control the PA0 LED.
- `SHUTDOWN`: show a shutdown screen.

Snake:

- Up/Down/Left/Right control the snake.
- Back returns to home.
- Enter restarts after game over.

LED app:

- Enter turns PA0 LED on.
- Back / Cancel turns PA0 LED off and returns home.

Shutdown:

- Enter wakes back to home.

## Build

The project uses the Arm GNU Toolchain:

```powershell
cd CoreSelva_MOS
make
```

On systems without `make`, compile with the same flags shown in the `Makefile`.

The output files are:

- `firmware.elf`: linked executable with debug symbols
- `firmware.bin`: raw binary to flash
- `firmware.map`: linker map

These generated files are not currently stored in the cleaned GitHub repo. Build the project locally to create them.

## Flash

Flash `firmware.bin` to the STM32WB55 at the flash base address:

```text
0x08000000
```

You can use your normal STM32 flashing tool, for example STM32CubeProgrammer, ST-Link tools, or OpenOCD.

## Project Structure

```text
CoreSelva_MOS/
  main.c                  OS UI, apps, Snake game, boot flow
  Makefile                Build rules
  startup_stm32wb55.s     Vector table and reset startup
  STM32WB55CGUX_FLASH.ld  Linker script
  drivers/
    buttons.c/.h          Button GPIO input driver
    gpio.c/.h             PA0 LED output driver
    spi.c/.h              SPI1 and TFT control pins
    systick.c/.h          1 ms system tick
  display/
    st7735.c/.h           TFT display driver
    tft_config.h          Display size and offsets
  kernel/
    kernel.c/.h           Small cooperative scheduler
```

## How It Works

The firmware starts in `main()`:

1. Initializes buttons, GPIO, SysTick, and the TFT.
2. Shows the boot splash.
3. Starts the tiny scheduler.
4. Runs `ui_task()` every few milliseconds.

`ui_task()` reads buttons, updates the current screen, and redraws only what changed. Full-screen redraws are used when entering a new screen. Small partial redraws are used for home selection, app drawer selection, Snake movement, and LED state changes so the UI feels faster.

For a full beginner-friendly, file-by-file code explanation, open:


## Notes

- The code uses direct register access instead of HAL.
- The ST7735 driver is configured for RGB color order with `MADCTL = 0xC0`.
- The UI theme is intentionally limited to black, yellow, and white.
- PA5 is used by SPI SCK, so the LED app uses PA0.
