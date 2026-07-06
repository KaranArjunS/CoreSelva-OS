#include "buttons.h"
#include "gpio.h"
#include "kernel.h"
#include "systick.h"
#include "st7735.h"
#include "tft_config.h"
#include <stdint.h>

#define CS_YELLOW       ST7735_YELLOW
#define CS_BLACK        ST7735_BLACK
#define CS_WHITE        ST7735_WHITE
#define CS_DIM          ST7735_WHITE
#define CS_PANEL        ST7735_BLACK

#define UI_TICK_MS      10U
#define SNAKE_STEP_MS   120U
#define SNAKE_CELL      8U
#define SNAKE_COLS      16U
#define SNAKE_ROWS      14U
#define SNAKE_TOP       40U
#define SNAKE_MAX       (SNAKE_COLS * SNAKE_ROWS)

typedef enum {
    SCREEN_HOME,
    SCREEN_APPS,
    SCREEN_SNAKE,
    SCREEN_LED,
    SCREEN_SHUTDOWN
} Screen;

typedef enum {
    KEY_UP    = 1U << 0,
    KEY_DOWN  = 1U << 1,
    KEY_LEFT  = 1U << 2,
    KEY_RIGHT = 1U << 3,
    KEY_BACK  = 1U << 4,
    KEY_ENTER = 1U << 5
} KeyMask;

typedef struct {
    int8_t x;
    int8_t y;
} Point;

static Screen screen = SCREEN_HOME;
static uint8_t home_index = 3U;
static uint8_t app_index = 0U;
static uint8_t led_enabled = 0U;
static uint8_t dirty = 1U;

static Point snake[SNAKE_MAX];
static uint16_t snake_len = 0U;
static Point food;
static int8_t snake_dx = 1;
static int8_t snake_dy = 0;
static uint16_t snake_score = 0U;
static uint8_t snake_over = 0U;
static uint32_t snake_tick = 0U;
static uint32_t prng = 0xC05E17A5UL;

static const char *menu_items[] = {
    "SNAKE",
    "LED",
    "SHUTDOWN"
};

static uint8_t glyph(char c, uint8_t col)
{
    static const uint8_t digits[10][5] = {
        {0x3E,0x51,0x49,0x45,0x3E}, {0x00,0x42,0x7F,0x40,0x00},
        {0x42,0x61,0x51,0x49,0x46}, {0x21,0x41,0x45,0x4B,0x31},
        {0x18,0x14,0x12,0x7F,0x10}, {0x27,0x45,0x45,0x45,0x39},
        {0x3C,0x4A,0x49,0x49,0x30}, {0x01,0x71,0x09,0x05,0x03},
        {0x36,0x49,0x49,0x49,0x36}, {0x06,0x49,0x49,0x29,0x1E}
    };
    static const uint8_t letters[26][5] = {
        {0x7E,0x11,0x11,0x11,0x7E}, {0x7F,0x49,0x49,0x49,0x36},
        {0x3E,0x41,0x41,0x41,0x22}, {0x7F,0x41,0x41,0x22,0x1C},
        {0x7F,0x49,0x49,0x49,0x41}, {0x7F,0x09,0x09,0x09,0x01},
        {0x3E,0x41,0x49,0x49,0x7A}, {0x7F,0x08,0x08,0x08,0x7F},
        {0x00,0x41,0x7F,0x41,0x00}, {0x20,0x40,0x41,0x3F,0x01},
        {0x7F,0x08,0x14,0x22,0x41}, {0x7F,0x40,0x40,0x40,0x40},
        {0x7F,0x02,0x0C,0x02,0x7F}, {0x7F,0x04,0x08,0x10,0x7F},
        {0x3E,0x41,0x41,0x41,0x3E}, {0x7F,0x09,0x09,0x09,0x06},
        {0x3E,0x41,0x51,0x21,0x5E}, {0x7F,0x09,0x19,0x29,0x46},
        {0x46,0x49,0x49,0x49,0x31}, {0x01,0x01,0x7F,0x01,0x01},
        {0x3F,0x40,0x40,0x40,0x3F}, {0x1F,0x20,0x40,0x20,0x1F},
        {0x3F,0x40,0x38,0x40,0x3F}, {0x63,0x14,0x08,0x14,0x63},
        {0x07,0x08,0x70,0x08,0x07}, {0x61,0x51,0x49,0x45,0x43}
    };

    if (c >= 'a' && c <= 'z') c = (char)(c - 32);
    if (c >= 'A' && c <= 'Z') return letters[(uint8_t)(c - 'A')][col];
    if (c >= '0' && c <= '9') return digits[(uint8_t)(c - '0')][col];
    if (c == '-') return (col == 1U || col == 2U || col == 3U) ? 0x08U : 0x00U;
    if (c == '>') return (col == 0U) ? 0x41U : (col == 1U ? 0x22U : (col == 2U ? 0x14U : (col == 3U ? 0x08U : 0x00U)));
    if (c == '.') return (col == 2U) ? 0x40U : 0x00U;
    return 0x00U;
}

static void draw_char(uint16_t x, uint16_t y, char c, uint16_t color,
                      uint16_t bg, uint8_t scale)
{
    for (uint8_t cx = 0U; cx < 6U; cx++) {
        uint8_t bits = (cx < 5U) ? glyph(c, cx) : 0U;
        for (uint8_t cy = 0U; cy < 7U; cy++) {
            uint16_t px = (uint16_t)(x + (uint16_t)cx * scale);
            uint16_t py = (uint16_t)(y + (uint16_t)cy * scale);
            ST7735_FillRect(px, py, scale, scale,
                            (bits & (1U << cy)) ? color : bg);
        }
    }
}

static void draw_text(uint16_t x, uint16_t y, const char *text,
                      uint16_t color, uint16_t bg, uint8_t scale)
{
    while (*text != '\0') {
        draw_char(x, y, *text, color, bg, scale);
        x = (uint16_t)(x + (uint16_t)6U * scale);
        text++;
    }
}

static void draw_number(uint16_t x, uint16_t y, uint16_t value,
                        uint16_t color, uint16_t bg, uint8_t scale)
{
    char buf[6];
    int8_t i = 4;
    buf[5] = '\0';
    do {
        buf[i--] = (char)('0' + (value % 10U));
        value /= 10U;
    } while (value != 0U && i >= 0);
    draw_text(x, y, &buf[i + 1], color, bg, scale);
}

static uint8_t read_keys(void)
{
    static uint8_t previous = 0U;
    uint8_t now = 0U;
    uint8_t edge;

    if (Button_Up()) now |= KEY_UP;
    if (Button_Down()) now |= KEY_DOWN;
    if (Button_Left()) now |= KEY_LEFT;
    if (Button_Right()) now |= KEY_RIGHT;
    if (Button_Back()) now |= KEY_BACK;
    if (Button_Enter()) now |= KEY_ENTER;

    edge = (uint8_t)(now & (uint8_t)~previous);
    previous = now;
    return edge;
}

static void splash(void)
{
    ST7735_FillScreen(CS_BLACK);
    draw_text(16U, 50U, "CORESELVA", CS_YELLOW, CS_BLACK, 2U);
    draw_text(44U, 72U, "OS", CS_YELLOW, CS_BLACK, 3U);
    draw_text(22U, 126U, "OPEN ENOUGH", CS_DIM, CS_BLACK, 1U);
}

static void draw_header(const char *title)
{
    ST7735_FillRect(0U, 0U, TFT_WIDTH, 22U, CS_BLACK);
    draw_text(4U, 6U, title, CS_YELLOW, CS_BLACK, 1U);
    ST7735_FillRect(0U, 22U, TFT_WIDTH, 1U, CS_YELLOW);
}

static void draw_status_bar(void)
{
    ST7735_FillRect(0U, 0U, TFT_WIDTH, 12U, CS_BLACK);
    draw_text(34U, 3U, "CORESELVA", CS_YELLOW, CS_BLACK, 1U);
    ST7735_FillRect(0U, 12U, TFT_WIDTH, 1U, CS_YELLOW);
}

static void draw_app_icon(uint16_t x, uint16_t y, const char *label,
                          uint8_t selected, uint8_t shape)
{
    uint16_t icon_bg = selected ? CS_YELLOW : CS_BLACK;
    uint16_t icon_fg = selected ? CS_BLACK : CS_YELLOW;
    uint16_t text_fg = selected ? CS_YELLOW : CS_WHITE;

    ST7735_FillRect((uint16_t)(x - 4U), (uint16_t)(y - 4U), 40U, 46U, CS_BLACK);
    ST7735_FillRect(x, y, 28U, 28U, icon_bg);
    ST7735_FillRect((uint16_t)(x + 2U), (uint16_t)(y + 2U), 24U, 24U, icon_fg);
    ST7735_FillRect((uint16_t)(x + 5U), (uint16_t)(y + 5U), 18U, 18U, icon_bg);

    if (shape == 0U) {
        ST7735_FillRect((uint16_t)(x + 9U), (uint16_t)(y + 9U), 10U, 3U, icon_fg);
        ST7735_FillRect((uint16_t)(x + 9U), (uint16_t)(y + 16U), 10U, 3U, icon_fg);
    } else if (shape == 1U) {
        ST7735_FillRect((uint16_t)(x + 8U), (uint16_t)(y + 8U), 12U, 12U, icon_fg);
        ST7735_FillRect((uint16_t)(x + 11U), (uint16_t)(y + 11U), 6U, 6U, icon_bg);
    } else {
        ST7735_FillRect((uint16_t)(x + 12U), (uint16_t)(y + 7U), 4U, 12U, icon_fg);
        ST7735_FillRect((uint16_t)(x + 8U), (uint16_t)(y + 17U), 12U, 3U, icon_fg);
    }

    draw_text((uint16_t)(x - 3U), (uint16_t)(y + 33U), label, text_fg, selected ? CS_PANEL : CS_BLACK, 1U);
}

static void draw_home_item(uint8_t index)
{
    if (index == 0U) {
        draw_app_icon(12U, 88U, "SNAKE", home_index == 0U, 0U);
    } else if (index == 1U) {
        draw_app_icon(50U, 88U, "LED", home_index == 1U, 1U);
    } else if (index == 2U) {
        draw_app_icon(88U, 88U, "POWER", home_index == 2U, 2U);
    } else {
        ST7735_FillRect(26U, 138U, 76U, 18U, home_index == 3U ? CS_YELLOW : CS_BLACK);
        draw_text(52U, 144U, "MENU", home_index == 3U ? CS_BLACK : CS_YELLOW,
                  home_index == 3U ? CS_YELLOW : CS_BLACK, 1U);
    }
}

static void draw_home(void)
{
    ST7735_FillScreen(CS_BLACK);
    draw_status_bar();

    draw_text(14U, 28U, "CORESELVA", CS_WHITE, CS_BLACK, 2U);
    draw_text(26U, 50U, "OS", CS_YELLOW, CS_BLACK, 3U);

    for (uint8_t i = 0U; i < 4U; i++) {
        draw_home_item(i);
    }
}

static void draw_app_row(uint8_t index)
{
    uint16_t y = (uint16_t)(46U + index * 30U);
    uint8_t selected = (index == app_index);
    uint16_t fg = selected ? CS_BLACK : CS_WHITE;
    uint16_t bg = selected ? CS_YELLOW : CS_BLACK;

    ST7735_FillRect(8U, (uint16_t)(y - 6U), 112U, 22U, bg);
    ST7735_FillRect(16U, (uint16_t)(y - 2U), 8U, 8U, fg);
    draw_text(32U, y, menu_items[index], fg, bg, 1U);
}

static void draw_apps(void)
{
    ST7735_FillScreen(CS_BLACK);
    draw_header("APPS");

    for (uint8_t i = 0U; i < 3U; i++) {
        draw_app_row(i);
    }

    draw_text(12U, 146U, "B3 HOME  A15 OPEN", CS_DIM, CS_BLACK, 1U);
}

static uint32_t next_rand(void)
{
    prng = prng * 1664525UL + 1013904223UL;
    return prng;
}

static uint8_t snake_hits(uint8_t x, uint8_t y)
{
    for (uint16_t i = 0U; i < snake_len; i++) {
        if (snake[i].x == (int8_t)x && snake[i].y == (int8_t)y) return 1U;
    }
    return 0U;
}

static void place_food(void)
{
    do {
        food.x = (int8_t)(next_rand() % SNAKE_COLS);
        food.y = (int8_t)(next_rand() % SNAKE_ROWS);
    } while (snake_hits((uint8_t)food.x, (uint8_t)food.y));
}

static void snake_new(void)
{
    snake_len = 4U;
    snake[0].x = 8; snake[0].y = 7;
    snake[1].x = 7; snake[1].y = 7;
    snake[2].x = 6; snake[2].y = 7;
    snake[3].x = 5; snake[3].y = 7;
    snake_dx = 1;
    snake_dy = 0;
    snake_score = 0U;
    snake_over = 0U;
    snake_tick = 0U;
    place_food();
}

static void draw_snake(void)
{
    ST7735_FillScreen(CS_BLACK);
    draw_header("SNAKE");
    draw_text(4U, 28U, "SCORE", CS_DIM, CS_BLACK, 1U);
    draw_number(44U, 28U, snake_score, CS_YELLOW, CS_BLACK, 1U);

    ST7735_FillRect(0U, (uint16_t)(SNAKE_TOP - 1U), TFT_WIDTH, 1U, CS_YELLOW);
    ST7735_FillRect(0U, (uint16_t)(SNAKE_TOP + SNAKE_ROWS * SNAKE_CELL), TFT_WIDTH, 1U, CS_YELLOW);

    ST7735_FillRect((uint16_t)(food.x * SNAKE_CELL + 2), (uint16_t)(SNAKE_TOP + food.y * SNAKE_CELL + 2),
                    4U, 4U, CS_YELLOW);

    for (uint16_t i = 0U; i < snake_len; i++) {
        uint16_t c = (i == 0U) ? CS_YELLOW : CS_WHITE;
        ST7735_FillRect((uint16_t)(snake[i].x * SNAKE_CELL + 1),
                        (uint16_t)(SNAKE_TOP + snake[i].y * SNAKE_CELL + 1),
                        6U, 6U, c);
    }

    if (snake_over) {
        ST7735_FillRect(18U, 74U, 92U, 34U, CS_PANEL);
        draw_text(34U, 80U, "GAME OVER", CS_YELLOW, CS_PANEL, 1U);
        draw_text(28U, 94U, "ENTER AGAIN", CS_WHITE, CS_PANEL, 1U);
    }
}

static void draw_snake_cell(Point p, uint16_t color)
{
    ST7735_FillRect((uint16_t)(p.x * SNAKE_CELL), (uint16_t)(SNAKE_TOP + p.y * SNAKE_CELL),
                    SNAKE_CELL, SNAKE_CELL, CS_BLACK);
    if (color != CS_BLACK) {
        ST7735_FillRect((uint16_t)(p.x * SNAKE_CELL + 1),
                        (uint16_t)(SNAKE_TOP + p.y * SNAKE_CELL + 1),
                        6U, 6U, color);
    }
}

static void draw_food(void)
{
    ST7735_FillRect((uint16_t)(food.x * SNAKE_CELL), (uint16_t)(SNAKE_TOP + food.y * SNAKE_CELL),
                    SNAKE_CELL, SNAKE_CELL, CS_BLACK);
    ST7735_FillRect((uint16_t)(food.x * SNAKE_CELL + 2),
                    (uint16_t)(SNAKE_TOP + food.y * SNAKE_CELL + 2),
                    4U, 4U, CS_YELLOW);
}

static void draw_score_value(void)
{
    ST7735_FillRect(44U, 28U, 42U, 8U, CS_BLACK);
    draw_number(44U, 28U, snake_score, CS_YELLOW, CS_BLACK, 1U);
}

static void draw_game_over(void)
{
    ST7735_FillRect(18U, 74U, 92U, 34U, CS_BLACK);
    ST7735_FillRect(18U, 74U, 92U, 1U, CS_YELLOW);
    ST7735_FillRect(18U, 107U, 92U, 1U, CS_YELLOW);
    draw_text(34U, 80U, "GAME OVER", CS_YELLOW, CS_BLACK, 1U);
    draw_text(28U, 94U, "ENTER AGAIN", CS_WHITE, CS_BLACK, 1U);
}

static void snake_step(void)
{
    Point head = snake[0];
    Point old_head = snake[0];
    Point old_tail = snake[(uint16_t)(snake_len - 1U)];
    uint8_t ate;
    uint16_t hit_len;

    head.x = (int8_t)(head.x + snake_dx);
    head.y = (int8_t)(head.y + snake_dy);
    ate = (head.x == food.x && head.y == food.y);
    hit_len = ate ? snake_len : (uint16_t)(snake_len - 1U);

    if (head.x < 0 || head.x >= (int8_t)SNAKE_COLS ||
        head.y < 0 || head.y >= (int8_t)SNAKE_ROWS) {
        snake_over = 1U;
        draw_game_over();
        return;
    }

    for (uint16_t i = 0U; i < hit_len; i++) {
        if (snake[i].x == head.x && snake[i].y == head.y) {
            snake_over = 1U;
            draw_game_over();
            return;
        }
    }

    if (ate && snake_len < SNAKE_MAX) snake_len++;

    for (uint16_t i = (uint16_t)(snake_len - 1U); i > 0U; i--) {
        snake[i] = snake[i - 1U];
    }
    snake[0] = head;

    if (!ate) draw_snake_cell(old_tail, CS_BLACK);
    draw_snake_cell(old_head, CS_WHITE);
    draw_snake_cell(head, CS_YELLOW);

    if (ate) {
        snake_score++;
        place_food();
        draw_score_value();
        draw_food();
    }
}

static void draw_led_app(void)
{
    ST7735_FillScreen(CS_BLACK);
    draw_header("LED");
    draw_text(30U, 46U, "PA0 LED", CS_WHITE, CS_BLACK, 2U);

    ST7735_FillRect(42U, 82U, 44U, 36U, led_enabled ? CS_YELLOW : CS_BLACK);
    ST7735_FillRect(46U, 86U, 36U, 28U, led_enabled ? CS_BLACK : CS_YELLOW);
    ST7735_FillRect(54U, 94U, 20U, 12U, led_enabled ? CS_YELLOW : CS_BLACK);

    draw_text(42U, 126U, led_enabled ? "ON" : "OFF", CS_YELLOW, CS_BLACK, 2U);
    draw_text(8U, 146U, "A15 ON  B3 CANCEL", CS_DIM, CS_BLACK, 1U);
}

static void draw_led_state(void)
{
    ST7735_FillRect(42U, 82U, 44U, 58U, CS_BLACK);
    ST7735_FillRect(42U, 82U, 44U, 36U, led_enabled ? CS_YELLOW : CS_BLACK);
    ST7735_FillRect(46U, 86U, 36U, 28U, led_enabled ? CS_BLACK : CS_YELLOW);
    ST7735_FillRect(54U, 94U, 20U, 12U, led_enabled ? CS_YELLOW : CS_BLACK);
    draw_text(42U, 126U, led_enabled ? "ON" : "OFF", CS_YELLOW, CS_BLACK, 2U);
}

static void draw_shutdown(void)
{
    ST7735_FillScreen(CS_BLACK);
    draw_text(22U, 58U, "CORESELVA", CS_YELLOW, CS_BLACK, 2U);
    draw_text(20U, 86U, "SHUTDOWN", CS_WHITE, CS_BLACK, 2U);
    draw_text(20U, 132U, "A15 WAKE", CS_DIM, CS_BLACK, 1U);
}

static void open_app(uint8_t index)
{
    if (index == 0U) {
        screen = SCREEN_SNAKE;
        snake_new();
    } else if (index == 1U) {
        led_enabled = 0U;
        GPIO_LED_Off();
        screen = SCREEN_LED;
    } else {
        screen = SCREEN_SHUTDOWN;
    }
    dirty = 1U;
}

static void move_home(int8_t delta)
{
    uint8_t old_index = home_index;

    if (delta < 0) {
        home_index = (home_index == 0U) ? 3U : (uint8_t)(home_index - 1U);
    } else {
        home_index = (uint8_t)((home_index + 1U) % 4U);
    }

    if (!dirty) {
        draw_home_item(old_index);
        draw_home_item(home_index);
    }
}

void ui_task(void)
{
    uint8_t keys = read_keys();

    if (screen == SCREEN_HOME) {
        if (keys & (KEY_LEFT | KEY_UP)) move_home(-1);
        if (keys & (KEY_RIGHT | KEY_DOWN)) move_home(1);
        if (keys & KEY_ENTER) {
            if (home_index == 3U) {
                screen = SCREEN_APPS;
                app_index = 0U;
                dirty = 1U;
            } else {
                open_app(home_index);
            }
        }
    } else if (screen == SCREEN_APPS) {
        if (keys & KEY_BACK) {
            screen = SCREEN_HOME;
            dirty = 1U;
        }
        if (keys & KEY_UP) {
            uint8_t old_index = app_index;
            app_index = (app_index == 0U) ? 2U : (uint8_t)(app_index - 1U);
            if (!dirty) {
                draw_app_row(old_index);
                draw_app_row(app_index);
            }
        }
        if (keys & KEY_DOWN) {
            uint8_t old_index = app_index;
            app_index = (uint8_t)((app_index + 1U) % 3U);
            if (!dirty) {
                draw_app_row(old_index);
                draw_app_row(app_index);
            }
        }
        if (keys & KEY_ENTER) open_app(app_index);
    } else if (screen == SCREEN_LED) {
        if (keys & KEY_BACK) {
            led_enabled = 0U;
            GPIO_LED_Off();
            screen = SCREEN_HOME;
            dirty = 1U;
        }
        if (keys & KEY_ENTER) {
            led_enabled = 1U;
            GPIO_LED_On();
            if (!dirty) draw_led_state();
        }
    } else if (screen == SCREEN_SHUTDOWN) {
        if (keys & KEY_ENTER) {
            screen = SCREEN_HOME;
            dirty = 1U;
        }
    } else {
        if (keys & KEY_BACK) {
            screen = SCREEN_HOME;
            dirty = 1U;
        } else if (snake_over) {
            if (keys & KEY_ENTER) {
                snake_new();
                dirty = 1U;
            }
        } else {
            if ((keys & KEY_UP) && snake_dy == 0) { snake_dx = 0; snake_dy = -1; }
            if ((keys & KEY_DOWN) && snake_dy == 0) { snake_dx = 0; snake_dy = 1; }
            if ((keys & KEY_LEFT) && snake_dx == 0) { snake_dx = -1; snake_dy = 0; }
            if ((keys & KEY_RIGHT) && snake_dx == 0) { snake_dx = 1; snake_dy = 0; }

            snake_tick = (uint32_t)(snake_tick + UI_TICK_MS);
            if (snake_tick >= SNAKE_STEP_MS) {
                snake_tick = 0U;
                snake_step();
            }
        }
    }

    if (dirty) {
        dirty = 0U;
        if (screen == SCREEN_HOME) draw_home();
        else if (screen == SCREEN_APPS) draw_apps();
        else if (screen == SCREEN_LED) draw_led_app();
        else if (screen == SCREEN_SHUTDOWN) draw_shutdown();
        else draw_snake();
    }

    Kernel_Delay(UI_TICK_MS);
}

int main(void)
{
    Button_Init();
    GPIO_Init();
    SysTick_Init();
    ST7735_Init();

    splash();
    delay_ms(1600U);

    Kernel_Init();
    Kernel_AddTask(ui_task);
    Kernel_Start();

    while (1) {
    }
}
