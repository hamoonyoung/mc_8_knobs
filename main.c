#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "tusb.h"
#include "class/midi/midi_device.h"
#include "extra/SEGGER_RTT/SEGGER_RTT.h"

#define ENC_8_A 0
#define ENC_8_B 1
#define ENC_7_A 2
#define ENC_7_B 3
#define ENC_6_A 4
#define ENC_6_B 5
#define ENC_5_A 6
#define ENC_5_B 7
#define ENC_4_A 8
#define ENC_4_B 9
#define ENC_3_A 10
#define ENC_3_B 11
#define ENC_2_A 16
#define ENC_2_B 17
#define ENC_1_A 18
#define ENC_1_B 19
#define ENC_8_BT 20
#define ENC_7_BT 21
#define ENC_6_BT 22
#define ENC_5_BT 26
#define ENC_4_BT 27
#define ENC_3_BT 28
#define ENC_2_BT 12
#define ENC_1_BT 15

#define SPEED_ROTATION_TIME_US 50000  // 50ms
absolute_time_t last_rotation_time[8];

uint8_t current_enc_index[8] = {0};
uint8_t prev_enc_index[8] = {0};
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

#define NOTE 60       // Middle C
#define VELOCITY 100  // Note velocity
#define CHANNEL 0     // MIDI channel 1

// Send MIDI Note On
void send_midi_note_on(uint8_t note, uint8_t velocity) {
    // 0x90 is the status byte for Note On. We bitwise OR it with the channel.
    uint8_t msg[3] = { 0x90 | CHANNEL, note, velocity };
    tud_midi_stream_write(0, msg, 3); // cable 0
}

void send_midi_note_off(uint8_t note, uint8_t velocity) {
    // 0x80 is the status byte for Note Off.
    uint8_t msg[3] = { 0x80 | CHANNEL, note, velocity };
    tud_midi_stream_write(0, msg, 3); // cable 0
}

// Send MIDI Control Change (CC)
void send_midi_cc(uint8_t cc_num, uint8_t value) {
    // 0xB0 is the status byte for Control Change. We bitwise OR it with the channel.
    uint8_t msg[3] = { 0xB0 | CHANNEL, cc_num, value };
    tud_midi_stream_write(0, msg, 3); // cable 0
}

// Callback required by TinyUSB
void tud_task(void);

void sleep_led_ms(int milliseconds)
{
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    sleep_ms(milliseconds);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
}

void print_state()
{
    for (int i = 0; i < 8; i++)
        SEGGER_RTT_printf(0, "%d\t", bt_value[i]);

    SEGGER_RTT_printf(0, "\n");

    for (int i = 0; i < 8; i++)
        SEGGER_RTT_printf(0, "%d\t", enc_value[i]);

    SEGGER_RTT_printf(0, "\n");
}

int main() {
    sleep_ms(500);
    stdio_init_all();

    for (int i = 0; i < sizeof(pins)/sizeof(pins[0]); i++) {
        gpio_init(pins[i]);
        gpio_set_dir(pins[i], GPIO_IN);
        gpio_pull_up(pins[i]);
    }

    if (cyw43_arch_init()) {
        SEGGER_RTT_printf(0, "Wi-Fi init failed");
        return -1;
    }

    for (int i = 0; i < 8; i++)
    {
        last_rotation_time[i] = get_absolute_time();
    }

    // Start TinyUSB LAST
    tusb_init();

    while (true) {
        // MUST run constantly without interruption
        tud_task();

        for (int i = 0; i < 8; i++)
        {
            current_bt_state[i] = gpio_get(bt_table[i]) ? 0 : 127;

            if (current_bt_state[i] != prev_bt_state[i])
            {
                sleep_led_ms(5);

                //SEGGER_RTT_printf(0, "BT[%d]: %d\n", i, bt_value[i]);
                prev_bt_state[i] = current_bt_state[i];
                uint8_t note_number = 120 + i;
                if (current_bt_state[i] == 0)
                {
                    bt_value[i]++;
                    send_midi_note_on(note_number, 127);
                }
                print_state();
            }

        }
        for (int i = 0; i < 8; i++)
        {
            current_enc_state[i] = !gpio_get(enc_a_table[i]) << 1 | !gpio_get(enc_b_table[i]);

            if (current_enc_state[i] != prev_enc_state[i])
            {
                current_enc_index[i] = (prev_enc_state[i] << 2) | current_enc_state[i];
                if ((current_enc_state[i] == 0 &&  prev_enc_state[i] != 3) && (lookup_table[prev_enc_index[i]] == lookup_table[current_enc_index[i]]))
                {
                    absolute_time_t now = get_absolute_time();

                    uint32_t now_us  = (uint32_t)to_us_since_boot(now);
                    uint32_t last_us = (uint32_t)to_us_since_boot(last_rotation_time[i]);
                    uint32_t diff_us = (uint32_t)absolute_time_diff_us(last_rotation_time[i], now);

                    int32_t speed_factor = 1;

                    if (diff_us < SPEED_ROTATION_TIME_US)
                    {
                        int64_t x = (int64_t)SPEED_ROTATION_TIME_US - diff_us;

                        if (x > 0)
                        {
                            int64_t num = x * x;
                            int64_t den = ((int64_t)SPEED_ROTATION_TIME_US * SPEED_ROTATION_TIME_US) / 10;

                            speed_factor = 1 + (int32_t)(num / den);
                        }
                    }

                    enc_value[i] += lookup_table[current_enc_index[i]] * speed_factor;
                    SEGGER_RTT_printf(0, "[%d] now %u, last %u, difference %u, speed_factor: %d\n", i, now_us, last_us, diff_us, speed_factor);

                    last_rotation_time[i] = now;
                }
                if (enc_value[i] > 127 && lookup_table[current_enc_index[i]] < 0)
                {
                    enc_value[i] = 0;
                }else if (enc_value[i] > 127 && lookup_table[current_enc_index[i]] > 0){
                    enc_value[i] = 127;
                }
                //SEGGER_RTT_printf(0, "ENC[%d]: current %d, prev %d, currentlookup %d, prevlookup %d\n", i,  current_enc_state[i],  prev_enc_state[i], lookup_table[current_enc_index[i]], lookup_table[prev_enc_index[i]]);
                prev_enc_index[i] = current_enc_index[i];
                prev_enc_state[i] = current_enc_state[i];
                uint8_t cc_number = 7 + i; // Encoders 0-7 become CCs 7-14
                send_midi_cc(cc_number, enc_value[i]);
                print_state();
            }
        }
    }
}