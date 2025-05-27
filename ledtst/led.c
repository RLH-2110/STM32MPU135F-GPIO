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

int main(int argc, char **argv)
{
  int ret = EXIT_SUCCESS;

  unsigned int ledState;
  unsigned int btnState;

  uint16_t ledType = 0;
  uint16_t btnType = 0;

  if (argc != 3){

    if (argc == 4 && cmp_str(argv[2],"by") == 0){
      /* get button type */
      btnType = get_btn_type(argv[3]);
      if (btnType == 0){
        printf("%s is not a valid button!\n",argv[3]);
        print_help(argv[0]);
        return 1;
      }
      /* active low detection */
      if ((btnType & 0x8000) != 0){
        puts("Active Low Button");
        btnState = BtnState_AL;
      }else{
        puts("Active High Button");
        btnState = BtnState_AH;
      }

    }
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
    printf("%s is not a valid state! it must be ON, OFF or BY!\n",argv[2]);
    print_help(argv[0]);
    return 1;
  }

  /* get led type */
  ledType = get_led_type(argv[1]);
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


  uint8_t ledData = get_led_data(argv[1]);  
  uint8_t btnData; 
  
  if (btnType != 0){
    btnData = get_btn_data(argv[3]);

    if (btnType == ledType && btnData == ledData){
      puts("Error: you cant use same GPIO port as input and ouput.");
      return 1;
    }
  }

  if (ledState >= LedState_LAST){
    printf("LED state %d not implemented!\n",ledState);
    return 1;
  }
  
  if (ledState == LedState_LOW || ledState == LedState_HIGH){
    switch (ledType & 0x7FFF){
      case 1:
        ret = set_gpio_led(ledData,ledState == LedState_HIGH ? HIGH : LOW); 
        break;

      case 2:
        ret = set_i2c_led(ledData,ledState == LedState_HIGH ? HIGH : LOW);
        break;
  
      default:
        puts("reached unreachable code in swith case!");
        return 1;
    }
  }else if (ledState == LedState_BY_BUTTON_AH || ledState == LedState_BY_BUTTON_AL){
     printf("GPIO BUTTION with line %d\n",btnData);
     ret = handle_gpio_button_toggle(ledType, ledData, ledState, btnData, btnState);
  }

  if (ret != 0)
    puts("an error has occured");  
  return 0;
}

void print_help(char* progname){
  printf("\n%s LEDNAME ON|OFF|BY [BTNNAME]\n\tLEDNAMES: LD3, LD4, LD6, LD7\n\tBTNNAMES: B1, B2\n\n\tON: turns on the LED\n\tOFF: turns off the LED\n\tBY: Uses the BTNNAME argument, and activates the LED when the button is pressed\n",progname);
}


/* 	Controll FUNCTIONS       */

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

int handle_gpio_button_toggle(uint16_t ledType, unsigned int ledData, int ledState, unsigned int btnLine, int btnState){
   
  if (ledState != LedState_BY_BUTTON_AL && ledState != LedState_BY_BUTTON_AH) 
    return -1;
  if (btnLine > 15) /* max 15 gpio lines */ 
    return -1;
  if (ledType == 1 && ledData > 15) /* max 15 gpio */ 
    return -1;
 
  int ret = 0;
  static void *mmapBase = NULL; /* Virtual base address */
  bool toggled = true; /* turns off led if button is not pressed, turns on when button is pressed */
  int state, oldState;
  bool setup = true;
 
  int offState = LOW;
  if (ledState == LedState_BY_BUTTON_AL)
    offState = HIGH;
  

  state = offState;
  oldState = invert_state(offState);

  /* gpio setup */
  if (gpio_init(&mmapBase,GPIOA_DESC) != 0)
    return -1;

  if (set_gpio_dir(mmapBase,GPIO_PIN_INPUT_DIRECTION,btnLine) != 0)
    return -1;
 
  if (set_otype(mmapBase,GPIO_PIN_INPUT_NO_PULL, btnLine) != 0)
    return -1;
  

  if ((ledType & 0x7FFF) == 1){ /* if GPIO LED*/
    printf("GPIO LED WITH LINE %d\n",ledData);
    if (set_gpio_dir(mmapBase,GPIO_PIN_OUTPUT_DIRECTION,ledData) != 0)
      return -1;
 
    if (set_otype(mmapBase,GPIO_PIN_OUTPUT_PUSHPULL, ledData) != 0)
      return -1;
  }
  
  while(true){   
 
    if (toggled){
      switch (ledType & 0x7FFF){
        case 1:
          printf("line: %d | state: %d\n",ledData,state);
          if (gpio_pin_set(mmapBase,state, ledData) != 0)
            return -1;
          break;

        case 2:
          ret = set_i2c_led(ledData,state);
          break;
  
        default:
          puts("reached unreachable code in swith case!");
          return 1;
      }
     
      toggled = false;
      oldState = state;
      if (setup){
        oldState = offState;
	setup = false;
      }
    }
 
  state = gpio_pin_read(mmapBase,btnLine); /* -1: error | 0: LOW | Positive Non-Zero: HIGH */
  if (state == -1)
    return -1;
  if (state != LOW) 
    state = HIGH;

  if (btnState == BtnState_AL && ledState == LedState_BY_BUTTON_AH)
    state = invert_state(state);
  if (btnState == BtnState_AH && ledState == LedState_BY_BUTTON_AL)
    state = invert_state(state);
  
  if (state != oldState)
    toggled = true; 

  }

  return 0;
}

/* util */

int invert_state(int state){
  if (state > 1){
    printf("Invalid state: %d! It must either be 0 for LOW or 1 for HIGH!");
    return -1;
  }

  if (state == HIGH)
    return LOW;
  return HIGH;
}

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
