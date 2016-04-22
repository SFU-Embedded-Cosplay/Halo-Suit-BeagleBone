#ifndef LED
#define LED

typedef struct led {
	int const usr;
} led_t;

void led_init(led_t led);

void led_on(led_t led);
void led_off(led_t led);

#endif