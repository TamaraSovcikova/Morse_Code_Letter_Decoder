#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "hardware/adc.h"
#include "pico/stdlib.h"
#include "includes/seven_segment.h"
#include "includes/buzzer.h"
#include "includes/potentiometer.h"


// Global variables
#define BUTTON_PIN 16
#define DEBOUNCE_DELAY 200
#define DOT_THRESHOLD 250   // Threshold for a dot (quick press)
#define MAX_MORSE_LENGTH 4  
#define NEGATIVE_FREQUENCY 165  // Error beep (E3)
#define DOT_FREQUENCY 440       // Dot beep (A4)
#define DASH_FREQUENCY 349      // Dash beep (F4)
#define DEFAULT_TIME_LIMIT 4000   // Default time limit (4 seconds)


clock_t start_time;
double time_taken;
char morseCodeInput[MAX_MORSE_LENGTH + 1] = "";  // Holds the Morse code input (e.g., "-.-")
bool is_button_pressed = false;
char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"; //TEMP
unsigned int time_limit = DEFAULT_TIME_LIMIT;

int decoder(char *morse_code);
char checkButton(double time_taken);
void displayNewLetter();
void playNote(unsigned int frequency, int duration_ms);
void buzzer_negative();
void buzzer_short_beep();
void buzzer_long_beep();
unsigned int get_time_limit_from_potentiometer();

int main() {
    stdio_init_all();
    seven_segment_init();
    seven_segment_off();
    buzzer_init(); 
    potentiometer_init();


    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_down(BUTTON_PIN);

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

                //setting the time limit based on the potentiometer
                time_limit = get_time_limit_from_potentiometer();
                double time_gap = (double)(current_time - start_time) / CLOCKS_PER_SEC * 1000;
                if (time_gap > time_limit && strlen(morseCodeInput) > 0) {
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
          buzzer_negative();
    }
    else {
        printf("Letter to display is: %c\n", alphabet[index]);
        seven_segment_show(index);
    }

    morseCodeInput[0] = '\0';  // Reset Morse code input 
    seven_segment_off();
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
        buzzer_short_beep();
        return '.';  // Quick press is a dot
    } else {
        buzzer_long_beep();
        return '-';  // Long press is a dash
    }
}

void playNote(unsigned int frequency, int duration_ms) {	
	buzzer_enable(frequency);
	sleep_ms(duration_ms);	
    buzzer_disable(); //Not sure if this needs to be here
}

void buzzer_negative() {
	playNote(NEGATIVE_FREQUENCY, 1000);
}

void buzzer_short_beep() {
	playNote(DOT_FREQUENCY, 200);
}

void buzzer_long_beep() {
	playNote(DASH_FREQUENCY, 800);
}

unsigned int get_time_limit_from_potentiometer() {    
    unsigned int pot_value = potentiometer_read(9);
    // Map the potentiometer value (0 - 9) to the time limit range (1000 ms to 5000 ms)
    unsigned int mapped_time_limit = MIN_TIME_LIMIT + (pot_value * (MAX_TIME_LIMIT - MIN_TIME_LIMIT)) / 9;

    printf("Current time limit: %d ms\n", mapped_time_limit);
    return mapped_time_limit;
}




