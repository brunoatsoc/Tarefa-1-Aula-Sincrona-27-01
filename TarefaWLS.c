#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"
#include "hardware/timer.h"

#define LED_PIN_RED 13
#define PIN_BUTTON_A 5
#define PIN_BUTTON_B 6
#define TIME_BLINK 100
#define NUM_PIXELS 25
#define NUM_NUMBERS 10
#define WS2812_PIN 7
#define IS_RGBW 0

const uint8_t LED_RED = 0;
const uint8_t LED_GREEN = 0;
const uint8_t LED_BLUE = 200;
static volatile uint32_t LAST_TIME_A = 0;
static volatile uint32_t LAST_TIME_B = 0;
int current_number = 0;

const bool led_buffer[NUM_NUMBERS][NUM_PIXELS] = {
    {1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1},
    {0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0},
    {1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1},
    {0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1}
};

void blink_led(uint led, uint time_led);
static inline void put_pixel(uint32_t pixel_grb);
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b);
void gpio_irq_handler(uint gpio, uint32_t events);
void show_number_leds(int number);

int main() {
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    gpio_init(LED_PIN_RED);
    gpio_set_dir(LED_PIN_RED, GPIO_OUT);
    gpio_put(LED_PIN_RED, 0);

    gpio_init(PIN_BUTTON_A);
    gpio_set_dir(PIN_BUTTON_A, GPIO_IN);
    gpio_pull_up(PIN_BUTTON_A);
    gpio_set_irq_enabled_with_callback(PIN_BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    gpio_init(PIN_BUTTON_B);
    gpio_set_dir(PIN_BUTTON_B, GPIO_IN);
    gpio_pull_up(PIN_BUTTON_B);
    gpio_set_irq_enabled_with_callback(PIN_BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    show_number_leds(0);

    while(1) {
        blink_led(LED_PIN_RED, TIME_BLINK);
    }
}

void blink_led(uint led, uint time_led) {
    gpio_put(led, 1);
    sleep_ms(time_led);
    gpio_put(led, 0);
    sleep_ms(time_led);
}

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    if (gpio == PIN_BUTTON_A && current_time - LAST_TIME_A > 200000) {
        LAST_TIME_A = current_time;

        if(current_number < 9) {
            current_number = (current_number + 1) % 10;
            show_number_leds(current_number);
        }
    } else if(gpio == PIN_BUTTON_B && current_time - LAST_TIME_B > 200000) {
        LAST_TIME_B = current_time;

        if(current_number > 0) {
            current_number = (current_number + 9) % 10;
            show_number_leds(current_number);
        }
    }
}

void show_number_leds(int number) {
    int i;

    for (i = 0; i < NUM_PIXELS; i++) {
        put_pixel(led_buffer[number][i] ? urgb_u32(0, 0, 200) : 0);
    }
}
