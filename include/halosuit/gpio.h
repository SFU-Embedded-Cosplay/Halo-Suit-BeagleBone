#pragma once
#include <stdbool.h>

struct GPIO_digital {
    const char name[64];
    const int pin;
    const bool high;
    int fd;
};

struct GPIO_analog {
    const char name[64];
    const int apin;
    int fd;
};

void gpio_init();

void gpio_export(int pin);
void gpio_unexport(int pin);

// return the file descriptor for the pin
int gpio_open_direction_file(int pin);

// write either high or low to a pin's file descriptor
void gpio_write_direction_file(int fd, bool high);

// return the file descriptor for the pin
int gpio_open_value_file(int pin);

int gpio_open_analog(int apin);

void gpio_exit();
