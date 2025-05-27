#ifndef INCLUDED_LED_H
#define INCLUDED_LED_H 

#include <stdint.h>

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0') 

enum LedState {LedState_LOW, LedState_HIGH, LedState_BY_BUTTON_AH, LedState_BY_BUTTON_AL, LedState_LAST};
enum BtnState {BtnState_AH, BtnState_AL};


/* returns 0 if both string are the same, otherwhise retruns a non zero value*/
int cmp_str(char const *str1, char const *str2);



/* sets an LED to state.
  state:
    0: LOW
    1: High
  line: gpio line of the gpio chip
  regval: mask for the bit in the I2C register for the LED
  returns 0 on failure, and 1 on success. */
int set_gpio_led(uint8_t line, int state);
int set_i2c_led(uint8_t regval, int state);

/*
  gets the type of led (GPIO, I2C) if the Most Significant bit is set, then its active low, if not, its active high
  ledname: name of the led (ie. LD7, LD3)

  values:
    0: invalid LED
    1: GPIO0 LED
    2: I2C0 LED

    example returns
    GPIO0 LED (active high) > 0x0001
    GPIO0 LED (active  low) > 0x8001
    I2C0  LED (active high) > 0x0002
    I2C0  LED (active  low) > 0x8002
*/
uint16_t get_led_type(char const *ledname);
 
/* returns data needed to identify the LED, check if the LED is valid before using the function, by using get_led_type*/
uint8_t get_led_data(char const *ledname);


/* returns data needed to identify the Button, check if the Button is valid before using the function, by using get_btn_type*/
uint8_t get_btn_data(char const *ledname);

/*
  gets the type of button. If the Most Significant bit is set, then its active low, if not, its active high
  btnname: name of the button (ie. B1, B2)

  values:
    0: invalid BTN
    1: GPIO0 BTN
*/
uint16_t get_btn_type(char const *btnname);


/* sets up the button and leds, goes int an endless loop and toggles the LED when the button is held.
  returns: -1 on error.
*/
int handle_gpio_button_toggle(uint16_t ledType, unsigned int ledData, int ledState, unsigned int btnLine, int btnState);

/* inverts the state (high -> low | low -> high) 
  returns: either HIGH, LOW or -1, if the state was invalid.
*/
int invert_state(int state);

/* prints help screen
  progname: name of this program (argv[0])*/
void print_help(char *progname);

/* INCLUDED_LED_H */
#endif
