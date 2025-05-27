/*led应用程序*/
/* progamm modified by Roland Hartung */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>

#include "riolib.h"
#include "led.h"

enum  LedState {LedState_LOW, LedState_HIGH, LedState_BY_BUTTON_AH, LedState_BY_BUTTON_AL};

int main(int argc, char **argv)
{
  int ret = EXIT_SUCCESS;

  unsigned int ledState;
  unsigned int btnState;

  if (argc != 3){
    
    if (argc == 4 && cmp_str(argv[3],"by"));
    else{
	/* runs when argc != 3 and ther is no valid expected 4th parameter */
    	print_help(argv[0]);
    	return 0;
    }
  }

  /* find out if the led should be on or off*/
  if      (cmp_str(argv[2],"on")  == 0)
    ledState = LedState_HIGH;
  else if (cmp_str(argv[2],"off") == 0)
    ledState = LedState_LOW;
  else if (cmp_str(argv[2],"by") == 0){
    ledState = LedState_BY_BUTTON_AH;
  }
  else{
    printf("%s is not a valid state! it must be on or off!\n",argv[2]);
    print_help(argv[0]);
    return 1;
  }

  /* get led type */
  uint16_t ledType = get_led_type(argv[1]);
  if (ledType == 0){
    printf("%s is not a valid led!\n",argv[1]);
    print_help(argv[0]);
    return 1;
  }

  /* high/low switch, if nessesary (for handleing active low) */
  if ((ledType & 0x8000) != 0){
    puts("Active Low LED");

    /* no xor, because ledState 2+ are reserved for other actions */
    if      (ledState == LedState_LOW)
      ledState = LedState_HIGH;
    else if (ledState == LedState_HIGH)
      ledState = LedState_LOW;
    else if (ledState == LedState_BY_BUTTON_AH)
      ledState = LedState_BY_BUTTON_AL;
  }else{
    puts("Active High LED");
  }

  if (ledState == LedState_HIGH) puts("LedState: Set to HIGH");  
  if (ledState == LedState_LOW) puts("LedState: Set to LOW");  
  if (ledState == LedState_BY_BUTTON_AH) puts("LedState: Set to By Button Active HIGH");  
  if (ledState == LedState_BY_BUTTON_AL) puts("LedState: Set to By Button Active Low");  




  /* enum implimentation and input is swill wip, and should be more complete in the next commit */

  uint8_t ledData =  get_led_data(argv[1]);  

  if (ledState > 1){
    printf("LED state %d not implemented!\n",ledState);
    return 1;
  }

  switch (ledType & 0x7FFF){
    case 1:
      printf("GPIO LED on line %d\n",ledData);
      ret = set_gpio_led(ledData,ledState); 
      break;

    case 2:
      printf("I2C LED with register value %X\n",ledData);
      ret = set_i2c_led(ledData,ledState);
      break;

    default:
      puts("reached unreachable code in swith case!");
      return 1;
  }
  if (ret != 0)
    puts("an error has occured");  
  return 0;
}

void print_help(char* progname){
  printf("\n%s LEDNAME ON|OFF\n\tLEDNAMES: LD3, LD4, LD6, LD7\n\n\tknown issue: you can only have one I2C LED on at once\n",progname);
}


/* 	Controll FUNCTIONS       */

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
uint16_t get_led_type(char const *ledname)
{
  if (cmp_str(ledname, "ld3") == 0)
    return 0x8001;
  if (cmp_str(ledname, "ld4") == 0)
    return 0x8001;
  if (cmp_str(ledname, "ld6") == 0)
    return 0x0002;
  if (cmp_str(ledname, "ld7") == 0)
    return 0x0002;
  
  return 0;
}
/* returns data needed to identify the LED, check if the LED is valid before using the function, by using get_led_type*/
uint8_t get_led_data(char const *ledname)
{
  if (cmp_str(ledname, "ld3") == 0)
    return 14;
  if (cmp_str(ledname, "ld4") == 0)
    return 13;
  if (cmp_str(ledname, "ld6") == 0)
    return 0x80;
  if (cmp_str(ledname, "ld7") == 0)
    return 0x40;
  
  return 0;
}

int set_i2c_led(uint8_t regval, int state){
  printf("regval: %X | state: %d\n",regval,state);

  return i2c_set_bits(0,0x21,state,1,regval);
}

int set_gpio_led(uint8_t line, int state)
{
  static void *mmapBase = NULL; /* Virtual base address */

  if (gpio_init(&mmapBase,GPIOA_DESC) != 0)
    return -1;

  printf("line: %d | state: %d\n",line,state);
  return gpio_pin_set_ws(mmapBase,state, line);
}
/*int blink_i2c_led();
int blink_gpio_led();
*/


/* util */

/* returns 0 if both string are the same, otherwhise retruns a non zero value*/
int cmp_str(char const *str1, char const *str2)
{
  for (;; str1++, str2++) {
        int d = tolower((unsigned char)*str1) - tolower((unsigned char)*str2);
        if (d != 0 || *str1 == 0)
            return d;
    }
  return 0;
}
