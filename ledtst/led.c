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
#include "extra_defines.h"

void test(void);
void get_pin_info_test(char* input, GPIOS exChip, uint8_t exLine);


int main(int argc, char **argv)
{
  /*test();*/ 

  int ret = EXIT_SUCCESS;

  unsigned int ledState;
  unsigned int btnState;

  PinType ledType = 0;
  PinType btnType = 0;

  if (argc != 3){

    if (argc == 4 && cmp_str(argv[ARG_COMMAND],"by") == 0){
      /* get button type */
      btnType = get_btn_type(argv[ARG_BUTTON]);
      if (btnType == PinType_INVALID){
        printf("%s is not a valid button!\n",argv[ARG_BUTTON]);
        print_help(argv[ARG_PROGRAM_NAME]);
        return EXIT_FAILURE;
      }
      /* active low detection */
      if (PinType_IS_ACTIVE_LOW(btnType) == true){
        puts("Active Low Button");
        btnState = BtnState_AL;
      }else{
        puts("Active High Button");
        btnState = BtnState_AH;
      }

    }
    else{
	/* runs when argc != 3 and ther is no valid expected 4th parameter */
    	print_help(argv[ARG_PROGRAM_NAME]);
    	return EXIT_SUCCESS;
    }
  }

  /* find out if the led should be on or off*/
  if      (cmp_str(argv[ARG_COMMAND],"on")  == 0)
    ledState = LedState_HIGH;
  else if (cmp_str(argv[ARG_COMMAND],"off") == 0)
    ledState = LedState_LOW;
  else if (cmp_str(argv[ARG_COMMAND],"by") == 0){
    ledState = LedState_BY_BUTTON_AH;
  }
  else{
    printf("%s is not a valid state! it must be ON, OFF or BY!\n",argv[ARG_COMMAND]);
    print_help(argv[ARG_PROGRAM_NAME]);
    return EXIT_FAILURE;
  }

  /* get led type */
  ledType = get_led_type(argv[ARG_LED]);
  if (ledType == PinType_INVALID){
    printf("%s is not a valid led!\n",argv[1]);
    print_help(argv[ARG_PROGRAM_NAME]);
    return EXIT_FAILURE;
  }

  /* high/low switch, if nessesary (for handleing active low) */
  if (PinType_IS_ACTIVE_LOW(ledType) == true){
    puts("Active Low LED");

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


  uint8_t ledData = get_led_data(argv[ARG_LED]);  
  uint8_t btnData; 
  
  if (btnType != PinType_INVALID){
    btnData = get_btn_data(argv[ARG_BUTTON]);

    if (btnType == ledType && btnData == ledData){
      puts("Error: you cant use same GPIO port as input and ouput.");
      return EXIT_FAILURE;
    }
  }

  if (ledState >= LedState_LAST){
    printf("LED state %d not implemented!\n",ledState);
    return EXIT_SUCCESS;
  }
  
  if (ledState == LedState_LOW || ledState == LedState_HIGH){
    switch (PinType_GET_CATEGORY_ONLY(ledType)){
      case PinType_GPIO:
        ret = set_gpio_led(ledData,ledState == LedState_HIGH ? HIGH : LOW); 
        break;

      case PinType_I2C:
        ret = set_i2c_led(ledData,ledState == LedState_HIGH ? HIGH : LOW);
        break;
  
      default:
        puts("reached unreachable code in swith case!");
        return EXIT_FAILURE;
    }
  }else if (ledState == LedState_BY_BUTTON_AH || ledState == LedState_BY_BUTTON_AL){
     printf("GPIO BUTTION with line %d\n",btnData);
     ret = handle_gpio_button_toggle(ledType, ledData, ledState, btnData, btnState);
  }

  if (ret != RETURN_SUCCESS)
    puts("an error has occured");  
  return EXIT_SUCCESS;
}

void print_help(char* progname){
  printf("\n%s LEDNAME ON|OFF|BY [BTNNAME]\n\tLEDNAMES: LD3, LD4, LD6, LD7\n\tBTNNAMES: B1, B2\n\n\tON: turns on the LED\n\tOFF: turns off the LED\n\tBY: Uses the BTNNAME argument, and activates the LED when the button is pressed\n",progname);
}


/* 	Controll FUNCTIONS       */

int set_i2c_led(uint8_t regval, int state){
  printf("regval: %X | state: %d\n",regval,state);

  return i2c_set_bits(MCP_0_ADDRESS,state,false,true,regval);
}

int set_gpio_led(uint8_t line, int state)
{
  static void *mmapBase = NULL; /* Virtual base address */

  if (gpio_init(&mmapBase,GPIOA_DESC) != 0)
    return RETURN_ERROR;

  printf("line: %d | state: %d\n",line,state);
  return gpio_pin_set_ws(mmapBase,state, line);
}
/*int blink_i2c_led();
int blink_gpio_led();
*/

int handle_gpio_button_toggle(PinType ledType, unsigned int ledData, int ledState, unsigned int btnLine, int btnState){
   
  if (ledState != LedState_BY_BUTTON_AL && ledState != LedState_BY_BUTTON_AH) 
    return RETURN_ERROR;
  if (btnLine > GPIO_MAX_LINE) /* max 15 gpio lines */ 
    return RETURN_ERROR;
  if (ledType == PinType_GPIO && ledData > GPIO_MAX_LINE) /* max 15 gpio */ 
    return RETURN_ERROR;
 
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
  if (gpio_init(&mmapBase,GPIOA_DESC) != RETURN_SUCCESS)
    return RETURN_ERROR;

  if (set_gpio_dir(mmapBase,GPIO_PIN_INPUT_DIRECTION,btnLine) != RETURN_SUCCESS)
    return RETURN_ERROR;
 
  if (set_otype(mmapBase,GPIO_PIN_INPUT_NO_PULL, btnLine) != RETURN_SUCCESS)
    return RETURN_ERROR;
  

  if (PinType_GET_CATEGORY_ONLY(ledType) == PinType_GPIO){ /* if GPIO LED*/
    printf("GPIO LED WITH LINE %d\n",ledData);
    if (set_gpio_dir(mmapBase,GPIO_PIN_OUTPUT_DIRECTION,ledData) != RETURN_SUCCESS)
      return RETURN_ERROR;
 
    if (set_otype(mmapBase,GPIO_PIN_OUTPUT_PUSHPULL, ledData) != RETURN_SUCCESS)
      return RETURN_ERROR;
  }
  
  while(true){   
 
    if (toggled){
      switch (PinType_GET_CATEGORY_ONLY(ledType)){
        case PinType_GPIO:
          printf("line: %d | state: %d\n",ledData,state);
          if (gpio_pin_set(mmapBase,state, ledData) != RETURN_SUCCESS)
            return RETURN_ERROR;
          break;

        case PinType_I2C:
          ret = set_i2c_led(ledData,state);
          break;
  
        default:
          puts("reached unreachable code in swith case!");
          return RETURN_ERROR;
      }
     
      toggled = false;
      oldState = state;
      if (setup){
        oldState = offState;
	setup = false;
      }
    }
 
  state = gpio_pin_read(mmapBase,btnLine); /* -1: error | 0: LOW | Positive Non-Zero: HIGH */
  if (state == RETURN_ERROR) /* if -1 */
    return RETURN_ERROR;
  if (state != LOW) 
    state = HIGH;

  if (btnState == BtnState_AL && ledState == LedState_BY_BUTTON_AH)
    state = invert_state(state);
  if (btnState == BtnState_AH && ledState == LedState_BY_BUTTON_AL)
    state = invert_state(state);
  
  if (state != oldState)
    toggled = true; 

  }

  return RETURN_SUCCESS;
}

/* util */

int invert_state(int state){
  if (state > HIGH){
    printf("Invalid state: %d! It must either be 0 for LOW or 1 for HIGH!");
    return RETURN_ERROR;
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

void get_pin_info_test(char* input, GPIOS exChip, uint8_t exLine){
  
  if (get_pin_info(input).gpioChip != exChip && get_pin_info("").gpioLine != exLine)
    printf("test fail pininfo '%s'",input);


}

void test(void){

  if (get_GPIO_Desc(0).GPIO_START_ADDR != GPIOA_DESC.GPIO_START_ADDR && get_GPIO_Desc(0).GPIO_MAP_SIZE != GPIOA_DESC.GPIO_MAP_SIZE)
    puts("test fail gpio A");

  if (get_GPIO_Desc(8).GPIO_START_ADDR != GPIOI_DESC.GPIO_START_ADDR && get_GPIO_Desc(8).GPIO_MAP_SIZE != GPIOI_DESC.GPIO_MAP_SIZE)
    puts("test fail gpio I");

  if (get_GPIO_Desc(9).GPIO_START_ADDR != GPIO_INVALID_DESC.GPIO_START_ADDR && get_GPIO_Desc(9).GPIO_MAP_SIZE != GPIO_INVALID_DESC.GPIO_MAP_SIZE)
    puts("test fail gpio invalid");


  get_pin_info_test(""    ,GPIO_INVALID,0);
  get_pin_info_test(NULL  ,GPIO_INVALID,0);
  get_pin_info_test("xt7" ,GPIO_INVALID,0);
  get_pin_info_test("xt17",GPIO_INVALID,0);
  get_pin_info_test("px0" ,GPIO_INVALID,0);
  get_pin_info_test("pa0" ,GPIOA,0);
  get_pin_info_test("pa1" ,GPIOA,1);
  get_pin_info_test("pi15",GPIOI,15);
  get_pin_info_test("pj5" ,GPIO_INVALID,0);
  get_pin_info_test("pa16",GPIO_INVALID,0);
  get_pin_info_test("pa00",GPIO_INVALID,0);
  get_pin_info_test("paF",GPIO_INVALID,0);
  get_pin_info_test("pa ",GPIO_INVALID,0);

  puts("tests complete!");
}
