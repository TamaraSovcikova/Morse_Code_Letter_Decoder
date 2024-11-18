#include "includes/seven_segment.h"
#include <stdio.h>
#include "pico/stdlib.h"

uint8_t values[] = {
    0b11101110, // A
    0b00111110, // b
    0b10011100, // c
    0b01111010, // d
    0b10011110, // e
    0b10001110, // f
    0b11110110, // g
    0b01101110, // h
    0b00001100, // i
    0b01111000, // j
    0b01101110, // k
    0b00011100, // l
    0b00101000, // m
    0b00101010, // n
    0b11111100, // o
    0b11001110, // p
    0b11100110, // q
    0b00001010, // r
    0b10110110, // s
    0b00011110, // t
    0b01111100, // u
    0b00111000, // v
    0b01010100, // w
    0b01101110, // x
    0b01110110, // y
    0b11011010  // z
};

// ------------------------------------------------------------------

void seven_segment_init()
{
    for (unsigned int i = 0; i < ALL_SEGMENTS_COUNT; i++)
    {
        gpio_init(ALL_SEGMENTS[i]);
        gpio_set_dir(ALL_SEGMENTS[i], GPIO_OUT);
    }
}

void seven_segment_off()
{
    for (unsigned int i = 0; i < ALL_SEGMENTS_COUNT; i++)
    {
        gpio_put(ALL_SEGMENTS[i], true);
    }
}

void seven_segment_on()
{
    for (unsigned int i = 0; i < ALL_SEGMENTS_COUNT; i++)
    {
        gpio_put(ALL_SEGMENTS[i], false);
    }
}

unsigned int seven_segment_show(unsigned int number)
{
    // Ensure number is within valid range
    if (number >= sizeof(values) / sizeof(values[0]))
    {
        // Handle invalid input (e.g., trigger a buzzer or error signal)
        return -1;
    }

    for (unsigned int i = 0; i < 7; i++)
    {
        unsigned int segmentBit = 1 << (6 - i); // Adjusting the bitshift to match segment count

        bool illuminateSegment = (segmentBit & values[number]); // != 0;

        gpio_put(ALL_SEGMENTS[i], !illuminateSegment);
    }
    printf("%d \n", number); // Debug output for the number being shown
    return 0;
}
