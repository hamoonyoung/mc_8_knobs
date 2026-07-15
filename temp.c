#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "extra/SEGGER_RTT/SEGGER_RTT.h"

#define ENC_1_A 0
#define ENC_1_B 1
#define ENC_2_A 2
#define ENC_2_B 3
#define ENC_3_A 4
#define ENC_3_B 5
#define ENC_4_A 6
#define ENC_4_B 7
#define ENC_5_A 8
#define ENC_5_B 9
#define ENC_6_A 10
#define ENC_6_B 11
#define ENC_7_A 16
#define ENC_7_B 17
#define ENC_8_A 18
#define ENC_8_B 19
#define ENC_1_BT 20
#define ENC_2_BT 21
#define ENC_3_BT 22
#define ENC_4_BT 26
#define ENC_5_BT 27
#define ENC_6_BT 28
#define ENC_7_BT 12
#define ENC_8_BT 15

void sleep_led_ms(int milliseconds)
{
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    sleep_ms(milliseconds);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
}

int main() {
    uint8_t enc_index[8] = {0};
    uint8_t current_enc_state[8] = {0};
    uint8_t current_bt_state[8] = {0};
    uint8_t prev_enc_state[8] = {0};
    uint8_t prev_bt_state[8] = {0};
    uint8_t enc_value[8] = {0};
    uint8_t bt_value[8] = {0};

    uint8_t enc_a_table[8] = {
        ENC_1_A, ENC_2_A, ENC_3_A, ENC_4_A, ENC_5_A, ENC_6_A, ENC_7_A, ENC_8_A
    };
    uint8_t enc_b_table[8] = {
        ENC_1_B, ENC_2_B, ENC_3_B, ENC_4_B, ENC_5_B, ENC_6_B, ENC_7_B, ENC_8_B
    };
    uint8_t bt_table[8] = {
        ENC_1_BT, ENC_2_BT, ENC_3_BT, ENC_4_BT, ENC_5_BT, ENC_6_BT, ENC_7_BT, ENC_8_BT
    };
    int8_t lookup_table[16] = {
        0, -1,  1,  0,
        1,  0,  0, -1,
       -1,  0,  0,  1,
        0,  1, -1,  0
    };

    stdio_init_all();

    const uint pins[] = {
        ENC_1_A, ENC_1_B,
        ENC_2_A, ENC_2_B,
        ENC_3_A, ENC_3_B,
        ENC_4_A, ENC_4_B,
        ENC_5_A, ENC_5_B,
        ENC_6_A, ENC_6_B,
        ENC_7_A, ENC_7_B,
        ENC_8_A, ENC_8_B,
        ENC_1_BT, ENC_2_BT, ENC_3_BT, ENC_4_BT, ENC_5_BT, ENC_6_BT, ENC_7_BT, ENC_8_BT
    };

    for (int i = 0; i < sizeof(pins)/sizeof(pins[0]); i++) {
        gpio_init(pins[i]);
        gpio_set_dir(pins[i], GPIO_IN);
        gpio_pull_up(pins[i]);
    }

    if (cyw43_arch_init()) {
        SEGGER_RTT_printf(0, "Wi-Fi init failed");
        return -1;
    }
    while (true) {
        for (int i = 0; i < 8; i++)
        {
            current_bt_state[i] = !gpio_get(bt_table[i]);

            if (current_bt_state[i] != prev_bt_state[i])
            {
                sleep_led_ms(5);
                bt_value[i] = current_bt_state[i];
                //SEGGER_RTT_printf(0, "BT[%d]: %d\n", i, bt_value[i]);
                prev_bt_state[i] = current_bt_state[i];
            }

        }
        for (int i = 0; i < 8; i++)
        {
            current_enc_state[i] = !gpio_get(enc_a_table[i]) << 1 | !gpio_get(enc_b_table[i]);

            if (current_enc_state[i] != prev_enc_state[i])
            {
                sleep_led_ms(5);
                // SEGGER_RTT_printf(0, "AB:");
                // for (int i = 7; i >= 0; i--)
                // {
                //     SEGGER_RTT_printf(0, "%d", current_ENC_8_state >> i & 1);
                // }
                // SEGGER_RTT_printf(0, "\n");
                enc_index[i] = (prev_enc_state[i] << 2) | current_enc_state[i];
                enc_value[i] += lookup_table[enc_index[i]];
                prev_enc_state[i] = current_enc_state[i];
                //SEGGER_RTT_printf(0, "ENC[%d]: %d\n", i, enc_value[i]);
            }
        }
    }
}




//////
///// Get the current time in milliseconds
uint32_t current_time = to_ms_since_boot(get_absolute_time());

// --- NON-BLOCKING LED OFF TIMER ---
// If the LED is on and 5ms have passed, turn it off!
if (led_is_on && current_time >= led_turn_off_time) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    led_is_on = false;
}

// --- NON-BLOCKING MIDI TIMER ---
// Send a note exactly once every 1000ms

// --- NON-BLOCKING TIMER VARIABLES ---
uint32_t last_note_time = 0;
bool note_is_on = false;

uint32_t led_turn_off_time = 0;
bool led_is_on = false;

//while
if (current_time - last_note_time >= 1000) {
    last_note_time = current_time;

    if (note_is_on) {
        send_midi_note_off(NOTE, VELOCITY);
        note_is_on = false;
    } else {
        send_midi_note_on(NOTE, VELOCITY);
        note_is_on = true;
    }
}