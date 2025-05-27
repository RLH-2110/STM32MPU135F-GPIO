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
