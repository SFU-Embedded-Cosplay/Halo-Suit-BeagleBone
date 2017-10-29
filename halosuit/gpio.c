#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>

#include <halosuit/gpio.h>

static int get_digits_in_number(int number) {
    int count = 0;
    while(number != 0) {
        number /= 10;             /* n=n/10 */
        count++;
    }

    return count;
}

void gpio_init() 
{

}

void gpio_export(int pin) 
{	
    // check this resource to learn about file I/O with  fopen and fputs.
    // http://www.tutorialspoint.com/cprogramming/c_file_io.htm
    FILE *export_fd = fopen("/sys/class/gpio/export", "w");

    int digitsInNumber = get_digits_in_number(pin);
    char pinValue[digitsInNumber];
    sprintf(pinValue, "%d", pin);

    fputs(pinValue, export_fd);

    fclose(export_fd);
}

void gpio_unexport(int pin) 
{
    int unexport_fd = open("/sys/class/gpio/unexport", O_WRONLY);
    char str[2];
    sprintf(str,"%d",pin);
    write(unexport_fd,str,2);
    close(unexport_fd);
}

// return the file descriptor for the pin
int gpio_open_direction_file(int pin) 
{
    // File path shouldn't be greater than 256 characters
    char path[256] = "/sys/class/gpio/gpio";
    char * str;
    sprintf(str, "%d", pin);
    strcat(path,str);
    strcat(path, "/direction");
    return open(path, O_WRONLY);
}

// sets the pin to either high or low
void gpio_write_direction_file(int fd, bool high)
{
    if (!high) {
        write(fd,"low",3);
    }
    else {
        write(fd,"high",4);
    }
}

// return the file descriptor for the pin
int gpio_open_value_file(int pin) 
{
    // File path shouldn't be greater than 256 characters
    char path[256] = "/sys/class/gpio/gpio";
    char * str;
    sprintf(str, "%d", pin);
    strcat(path,str);
    strcat(path, "/value");
    return open(path, O_RDWR);
}

int gpio_open_analog(int apin)
{
    // File path shouldn't be greater than 256 characters
    char path[256] = "/sys/bus/iio/devices/iio:device";
    char * str;
    sprintf(str, "%d", apin);
    strcat(path,str);
    strcat(path, "_raw");
    return open(path, O_RDONLY);
}

void gpio_exit() 
{

}
