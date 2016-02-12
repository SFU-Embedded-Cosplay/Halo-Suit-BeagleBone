#pragma once

void gpio_init();

void gpio_export(int pin);
void gpio_unexport(int pin);

// return the file descriptor for the pin
int gpio_open_direction_file(int pin);

// return the file descriptor for the pin
int gpio_open_value_file(int pin);

void gpio_exit();
