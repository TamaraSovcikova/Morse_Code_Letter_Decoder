#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "includes/seven_segment.h"
#include <time.h>
#include <stdbool.h>
// #include "hardware/pwm.h" BUZZER

// Global variables
#define BUTTON_PIN 16
#define DEBOUNCE_DELAY 200
#define DOT_THRESHOLD 250   // Threshold for a dot (quick press)
#define LETTER_GAP_THRESHOLD 2000  // Gap time for letter output (2 seconds)
#define MAX_MORSE_LENGTH 4  

clock_t start_time;
double time_taken;
char morseCodeInput[MAX_MORSE_LENGTH + 1] = "";  // Holds the Morse code input (e.g., "-.-")
bool is_button_pressed = false;
char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"; //TEMP

int decoder(char *morse_code);
char checkButton(double time_taken);
void sleep_ms_with_display_update(int ms);
void displayNewLetter();

int main() {
    stdio_init_all();
    seven_segment_init();
    seven_segment_off();
    // buzzer_init(); BUZZER

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_down(BUTTON_PIN);
    // gpio_init(BUZZER_PIN); BUZZER
    // gpio_set_dir(BUZZER_PIN, GPIO_OUT); BUZZER 

    bool button_released = true;

    while (true) {
        // Check if the button is pressed
        if (gpio_get(BUTTON_PIN)) {
            if (button_released) {
                // Button pressed for the first time
                button_released = false;
                start_time = clock();
            }
        } else {
            // Button released
            if (!button_released) {
                // Calculate press duration
                clock_t end_time = clock();
                time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC * 1000;

                // Determine if it was a dot or dash
                char symbol = checkButton(time_taken);
                printf("Symbol: %c\n", symbol);

                // Add the symbol to the Morse code string
                int len = strlen(morseCodeInput);
                if (len < MAX_MORSE_LENGTH) {
                    morseCodeInput[len] = symbol;
                    morseCodeInput[len + 1] = '\0';
                    printf("morseCodeInput: %s\n", morseCodeInput);
                }

                button_released = true;
                start_time = clock();  // Reset start time for letter gap tracking
            } else {
                // Check for letter gap
                clock_t current_time = clock();
                double time_gap = (double)(current_time - start_time) / CLOCKS_PER_SEC * 1000;
                if (time_gap > LETTER_GAP_THRESHOLD && strlen(morseCodeInput) > 0) {
                    printf("Letter gap detected\n");
                    displayNewLetter();
                }
            }
        }

        // DEBOUNCE_DELAY should ensure that the button press is only registered once per press to avoid multiple readings
        sleep_ms(DEBOUNCE_DELAY);
    }
    sleep_ms(200);
}

void displayNewLetter() {
    int index = decoder(morseCodeInput);
    if (index == -1) {
          printf("INPUT IS INVALID");
    }
    else {
        printf("Letter to display is: %c\n", alphabet[index]);
    }
    morseCodeInput[0] = '\0';  // Reset Morse code input
    printf("Resetting morseCodeInput: %s\n", morseCodeInput);
    seven_segment_off();
    if (index >= 0) {
        seven_segment_show(index);
    } else {
        printf("Invalid Morse code\n");
    }
}

int decoder(char *morse_code) {
    char *morse[] = {
        ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--", "-.",
        "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--.."
    };

    for (int i = 0; i < 26; i++) {
        if (strcmp(morse[i], morse_code) == 0) {
            return i;
        }
    }
    return -1;  // Return -1 if no match is found
}

char checkButton(double time_taken) {
    if (time_taken < DOT_THRESHOLD) {
        return '.';  // Quick press is a dot
    } else {
        return '-';  // Long press is a dash
    }
}

void sleep_ms_with_display_update(int ms) {
    sleep_ms(ms);
    seven_segment_off();
}

// // Function that generates a short beep
// void sound_dot() {
// 	playNote(DOT_FREQUENCY, 200);
// }

// // Function that generates a short beep
// void sound_dash() {
// 	playNote(DASH_FREQUENCY, 800);
// }

// // Function that generates a short beep
// void sound_negative() {
// 	playNote(NEGATIVE_FREQUENCY, 1000);
// }

// void playNote(unsigned int frequency, int duration_ms) {	
// 	buzzer_enable(frequency);
// 	sleep_ms(duration_ms);	
// }
