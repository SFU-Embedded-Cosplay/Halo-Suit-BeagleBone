#pragma once
#include <stdbool.h>

struct GPIO_device {
    const char name[64];
    const int pin;
    const bool high;
    int fd;
};

void gpio_init();

void gpio_export(int pin);
void gpio_unexport(int pin);

// return the file descriptor for the pin
int gpio_open_direction_file(int pin);

void gpio_write_direction_file(int fd, bool high);

// return the file descriptor for the pin
int gpio_open_value_file(int pin);

void gpio_exit();
