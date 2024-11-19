#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "hardware/adc.h"
#include "pico/stdlib.h"
#include "includes/seven_segment.h"
#include "includes/buzzer.h"
#include "includes/potentiometer.h"
#include "potentiometer.c"
#include "buzzer.c"


// Global variables
#define BUTTON_PIN 16
#define DEBOUNCE_DELAY 200
#define DOT_THRESHOLD 250   // Threshold for a dot (quick press)
#define MAX_MORSE_LENGTH 4  
#define NEGATIVE_FREQUENCY 165  // Error beep (E3)
#define DOT_FREQUENCY 440       // Dot beep (A4)
#define DASH_FREQUENCY 349      // Dash beep (F4)
#define DEFAULT_TIME_LIMIT 4000   // Default time limit (4 seconds)
#define MAX_TIME_LIMIT 10000
#define MIN_TIME_LIMIT 1000
#define RED 13
#define GREEN 12 
#define BLUE 11
#define BRIGHTNESS 50              // Default brightness value 
#define MAX_COLOUR_VALUE 255       // Max parameter value in show_rgb
#define MAX_PWM_LEVEL 65535        // Max pulse level value  
#define A_NOTE 880
#define D_NOTE 587
#define F_NOTE 698
#define E_NOTE 659
#define G_NOTE 784
#define FSharp_NOTE 739 


clock_t start_time;
double time_taken;
char morseCodeInput[MAX_MORSE_LENGTH + 1] = "";  // Holds the Morse code input (e.g., "-.-")
bool is_button_pressed = false;
char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"; //TEMP
unsigned int time_limit = DEFAULT_TIME_LIMIT;
char word[4]; // TEMP to display word 
int count = 0; // count the number of correct inputs

int decoder(char *morse_code);
char checkButton(double time_taken);
void displayNewLetter();
void playNote(unsigned int frequency, int duration_ms);
void buzzer_negative();
void buzzer_short_beep();
void buzzer_long_beep();
unsigned int get_time_limit_from_potentiometer();
void setup_rgb();   // Used to intialize the parameters of the RGB LED 
void show_rgb(int r, int g, int b); // Used to input the colour value for R/G/B
void play_tune(); // tune if word is 4 letters long 


int main() {
    timer_hw->dbgpause = 0;
    stdio_init_all();
    seven_segment_init();
    seven_segment_off();
    buzzer_init(); 
    potentiometer_init();

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_down(BUTTON_PIN);
    setup_rgb();
    show_rgb(0, 0, 0); //Turning off LED 
 
    bool button_released = true;    
    //Getting inicial potentiometer value
    time_limit = get_time_limit_from_potentiometer();    
    //Promting for time limit change via the potentiometer
    printf("Do you want to change the time limit? (You have 10 seconds) \n");
    sleep_ms(10000);
    int temp_time_limit = get_time_limit_from_potentiometer();

    //Checking if potentiometer has changed
    if (temp_time_limit == time_limit){
        time_limit = DEFAULT_TIME_LIMIT;        
    }
    else {
        time_limit = temp_time_limit;         
    }

    //setting the time limit based on the potentiometer   
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
                else{ 
                   morseCodeInput[0] = '\0'; 
                   printf("Input too long - reset");
                   buzzer_negative();
                }

                button_released = true;
                start_time = clock();  // Reset start time for letter gap tracking
            } else {
                // Check for letter gap
                clock_t current_time = clock();

         
                double time_gap = (double)(current_time - start_time) / CLOCKS_PER_SEC * 1000;
                if (time_gap > time_limit && strlen(morseCodeInput) > 0) {
                    printf("Letter gap detected\n");
                    displayNewLetter();
                    //BELOW
                    if (count == 4){
                        // Display the 4 letter word
                        printf("The word inputted is:", word);
                        // Reset the array after word is displayed 
                        for (int i = 0; i < 5; i++){
                            word[i] = '\0';
                        }
                        play_tune();
                        count = 0; 
                    }
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

    morseCodeInput[0] = '\0';  // Reset Morse code input
    printf("Resetting morseCodeInput: %s\n", morseCodeInput);
    seven_segment_off();   
    if (index >= 0) {
        setup_rgb(); // Lab 8 function to intialize the parameters of the RGB LED 
        show_rgb(0, 255, 0); // Display green on the RBG LED        
        sleep_ms(1000);
        printf("Letter to display is: %c\n", alphabet[index]);
        seven_segment_show(index);   
        count = count + 1; // Increase count if input is correct
        strcat(word, alphabet[index]); // Add the alphabet into the word array
      
    }
    else {
        printf("Invalid Morse code\n");   
        buzzer_negative();
        setup_rgb();
        show_rgb(255, 0, 0); // Display red on the RBG LED        
        sleep_ms(1000); // Green stays on for 1 second     
    }
    show_rgb(0, 0, 0); // Turn the LED off 
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
    buzzer_init(); 
	buzzer_enable(frequency);
	sleep_ms(duration_ms);	    
    buzzer_disable();
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

// Code from Lab 8, setup of the RGB LEDs
void setup_rgb() {
    gpio_set_function(RED, GPIO_FUNC_PWM);
    gpio_set_function(GREEN, GPIO_FUNC_PWM);
    gpio_set_function(BLUE, GPIO_FUNC_PWM);

    uint slice_num = pwm_gpio_to_slice_num(RED);
    pwm_config config = pwm_get_default_config();
    pwm_init(slice_num, &config, true);

    slice_num = pwm_gpio_to_slice_num(GREEN);
    pwm_init(slice_num, &config, true);

    slice_num = pwm_gpio_to_slice_num(BLUE);
    pwm_init(slice_num, &config, true);
}

void show_rgb(int r, int g, int b) {
    pwm_set_gpio_level(RED, ~(MAX_PWM_LEVEL * r / MAX_COLOUR_VALUE * BRIGHTNESS / 100));
    pwm_set_gpio_level(GREEN, ~(MAX_PWM_LEVEL * g / MAX_COLOUR_VALUE * BRIGHTNESS / 100));
    pwm_set_gpio_level(BLUE, ~(MAX_PWM_LEVEL * b / MAX_COLOUR_VALUE * BRIGHTNESS / 100));
}

void play_tune(){
    // first run 
    playNote(D_NOTE, 200);
    sleep_ms(400);
    playNote(F_NOTE, 200);
    playNote(E_NOTE, 200);
    sleep_ms(200);
    playNote(D_NOTE, 200);
    sleep_ms(600);
    playNote(A_NOTE, 200);
    sleep_ms(200);
    playNote(G_NOTE, 200);
    sleep_ms(450);    
    playNote(F_NOTE, 200);
}







