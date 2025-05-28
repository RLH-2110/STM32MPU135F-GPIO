/*led应用程序*/
/* progamm modified by Roland Hartung */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdbool.h>

#include "riolib.h"
#include "extra_defines.h"


/*      UTILITY FUNCTIONS        */

/* returns 0 if both string are the same, otherwhise retruns a non zero value*/
static int cmp_str(char const *str1, char const *str2)
{
  for (;; str1++, str2++) {
        int d = tolower((unsigned char)*str1) - tolower((unsigned char)*str2);
        if (d != 0 || *str1 == 0)
            return d;
    }
  return 0;
}   

/*        I2C FUNCTIONS          */


uint8_t send_i2c_shell_comand(bool read, uint8_t bus, uint8_t chipAddress, uint8_t registerAdr, uint8_t data)
{
  if (bus > 99){ /* 3 characters are too much for the buffer */
    puts("bus greater than 99 is not supported");
    return RETURN_ERROR;
  }
/* MONO SPACED FONT REQUIED TO VIEW       22
                       3      10   15   20|  25
                       V      V    V    V V  V  */ 
  char command[] = "i2cset -y 00 0x21 0x01 0x00\0      "; /* DO NOT CHANGE, UNLESS YOU ALSO CHANGE ALL DEFINES*/
  
  char buff[] = "00\0 ";

  if (read == true)
  {
    memcpy(command + 3,"get",3);
    command[22] = 0; /* terminate string early, because read does not need the last byte */
  }

  snprintf(buff,3,"%02d",bus); 
  memcpy(  command + 10, buff,2); 

  snprintf(buff,3,"%02X",chipAddress); 
  memcpy(  command + 15, buff,2); 

  snprintf(buff,3,"%02X",registerAdr); 
  memcpy(  command + 20, buff,2); 

  snprintf(buff,3,"%02X",data); 
  memcpy(  command + 25, buff,2); 


  if (read == false)
    return system(command);
  else { /* because we need to get the result printed in the console*/
   
    FILE *fp = popen(command,"r");
    if (fp == NULL) {
      return system(command); /*return whatever system() returned as error code*/ 
    }
    
    /*read from the output*/
    char inbuff[6] = "0x00\0";
    if (fgets(inbuff, sizeof(inbuff)-1, fp) != NULL) {
        pclose(fp);
        return (uint8_t)strtol(inbuff, NULL, 0); // strtol handles 0x prefix
    }

    pclose(fp);
  
  }
  return RETURN_ERROR; /* should not be reachable */
}

int i2c_set_bits(uint8_t bus, uint8_t chipAdr, unsigned int state, unsigned int abSelect, uint8_t registerMask)
{
  if (state > HIGH)
    return RETURN_ERROR;
  if (abSelect > I2C_GPIOB_SELECT)
    return RETURN_ERROR;

  /* select correct iodir register */
  uint8_t iodir = I2C_B0_IODIRA;
  if (abSelect == I2C_GPIOB_SELECT)
    iodir = I2C_B0_IODIRB;
  
  /* select correct gpio register*/
  uint8_t gpio = I2C_B0_GPIOA;
  if (abSelect == I2C_GPIOB_SELECT)
    gpio = I2C_B0_GPIOB;
 
  /* set unmaked state */ 
  uint8_t unmaskedState = 0x00; /* all off*/
  if(state == HIGH)
    unmaskedState = 0xFF; /* all on*/

  /* set the selected bits to output*/
  int outputs = send_i2c_shell_comand(true, bus, chipAdr, iodir, NONE);
  outputs = outputs & (~registerMask); /* set only the bits we selected to output */
  if (send_i2c_shell_comand(false, bus, chipAdr, iodir, outputs) != RETURN_SUCCESS)
    return RETURN_ERROR;

  /* write to the selected bits */
  outputs = send_i2c_shell_comand(true, bus, chipAdr, gpio, NONE);
  int registerVal = (outputs & (~registerMask)) | (unmaskedState & registerMask); /* set/unset the selected bits*/
  if (send_i2c_shell_comand(false, bus, chipAdr, gpio, registerVal) != RETURN_SUCCESS)
    return RETURN_ERROR;
  
  return RETURN_SUCCESS;
}


/*       GPIO FUNCTIONS          */


int set_gpio_dir(void *mmap, unsigned int direction, uint8_t line)
{ /* MODER */
  if (direction > GPIO_MAX_DIRECTION)
    return RETURN_ERROR;
  if (line > GPIO_MAX_LINE)
    return RETURN_ERROR;
 

 /* Load the different gpio register pointers with its address */
  volatile uint32_t *gpioRegAddr = mmap + GPIO_REG_MODER;
  /* Get the value of the gpio direction register */
  uint32_t gpioRegVal = *gpioRegAddr;
  /* Clear the GPIO, write it back to the direction register */    
  gpioRegVal &= ~(0x03 << (line * 2)); /* mask out the 2 bits giving the direction */
  gpioRegVal |= ((direction) << (line * 2));
  *gpioRegAddr = gpioRegVal;

  return RETURN_SUCCESS;
}
 
int set_otype(void *mmap, unsigned int outputType, uint8_t line)
{ /* OTYPE */
  if (line > GPIO_MAX_LINE)
    return RETURN_ERROR;
  if (outputType > GPIO_MAX_OUTPUT_DIRECTION)
    return RETURN_ERROR;
 
  /* Load the different gpio register pointers with its address */
  volatile uint32_t *gpioRegAddr = mmap + GPIO_REG_OTYPER;
  /* Get the value of the gpio direction register */
  uint32_t gpioRegVal = *gpioRegAddr;
  /* Clear the GPIO, write it back to the direction register */    
  gpioRegVal &= ~(0x0 << (line)); /* mask out */
  gpioRegVal |= ((outputType) << (line));
  *gpioRegAddr = gpioRegVal;
  return RETURN_SUCCESS;
}

int set_pull_type(void *mmap, unsigned int pullType, uint8_t line)
{ /* PUPDR */
  if (line > GPIO_MAX_LINE)
    return RETURN_ERROR;
  if (pullType > GPIO_MAX_INPUT_DIRECTION)
    return RETURN_ERROR;
 
  /* Load the different gpio register pointers with its address */
  volatile uint32_t *gpioRegAddr = mmap + GPIO_REG_PUPDR;
  /* Get the value of the gpio direction register */
  uint32_t gpioRegVal = *gpioRegAddr;
  /* Clear the GPIO, write it back to the direction register */    
  gpioRegVal &= ~(0x0 << (line)); /* mask out */
  gpioRegVal |= ((pullType) << (line));
  *gpioRegAddr = gpioRegVal;
  return RETURN_SUCCESS;
}

int gpio_pin_read(void *mmap, uint8_t line)
{
  if (line > GPIO_MAX_LINE)
    return RETURN_ERROR;
  
  volatile uint32_t *const gpioSetClearDataOutAddr = mmap + GPIO_REG_IDR;
  
  return ((uint16_t)*gpioSetClearDataOutAddr) & (1 << line); /* read the pin */
}
 
int gpio_pin_set(void *mmap, unsigned int state, uint8_t line)
{
  if (state > HIGH)
    return RETURN_ERROR;
  if (line > GPIO_MAX_LINE)
    return RETURN_ERROR;

  volatile uint32_t *const gpioSetClearDataOutAddr = mmap + GPIO_REG_BSRR;
  
  if (state == HIGH) 
    *gpioSetClearDataOutAddr = (uint16_t)(1 << line); /* set the pin */
  else 
    *gpioSetClearDataOutAddr = (uint32_t)((1 << line) << 16); /* reset the pin */
  
  return RETURN_SUCCESS;
}


int gpio_init(void **mmapBase, GPIO_Desc gpio_desc)
{
  if (mmapBase == NULL)
    return RETURN_ERROR;

  int fdMem = open(MEM_DEV, O_RDWR | O_SYNC);
  if (fdMem < 1)
    return RETURN_ERROR;

  *mmapBase = mmap(NULL,gpio_desc.GPIO_MAP_SIZE,PROT_READ | PROT_WRITE, MAP_SHARED, fdMem, gpio_desc.GPIO_START_ADDR);
  if (*mmapBase == (void*) -1)
    return RETURN_ERROR;
  
  return RETURN_SUCCESS;
}

int gpio_pin_set_ws(void *mmapBase, int state,uint8_t line) 
{
  if (set_gpio_dir(mmapBase, GPIO_PIN_OUTPUT_DIRECTION, line) != RETURN_SUCCESS)
    return RETURN_ERROR; 
  
  if (set_otype(mmapBase, GPIO_PIN_OUTPUT_PUSHPULL, line) != RETURN_SUCCESS)
    return RETURN_ERROR;

  return gpio_pin_set(mmapBase, state, line);
}



/*         DATA FUNCTIONS         */

/*
  gets the type of led (GPIO, I2C). If the Most Significant bit is set, then its active low, if not, its active high
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
PinType get_led_type(char const *ledname)
{
  if (cmp_str(ledname, "ld3") == 0)
    return PinType_GPIO + PinType_ACTIVE_LOW;
  if (cmp_str(ledname, "ld4") == 0)
    return PinType_GPIO + PinType_ACTIVE_LOW;
  if (cmp_str(ledname, "ld6") == 0)
    return PinType_I2C;
  if (cmp_str(ledname, "ld7") == 0)
    return PinType_I2C;

  return PinType_INVALID;
}
/* returns data needed to identify the LED, check if the LED is valid before using the function, by using get_led_type*/
uint8_t get_led_data(char const *ledname)
{
  if (ledname == NULL)
    return PinType_INVALID;

  if (cmp_str(ledname, "ld3") == 0)
    return GPIO_LINE_14;
  if (cmp_str(ledname, "ld4") == 0)
    return GPIO_LINE_13;
  if (cmp_str(ledname, "ld6") == 0)
    return I2C_GPx7;
  if (cmp_str(ledname, "ld7") == 0)
    return I2C_GPx6;

  return DATA_INVALID;
}
/*
  gets the type of button. If the Most Significant bit is set, then its active low, if not, its active high
  btnname: name of the button (ie. B1, B2)

  values:
    0: invalid BTN
    1: GPIO0 BTN
    2: I2C0 BTN 
*/
PinType get_btn_type(char const *btnname){
  if (btnname == NULL)
    return PinType_INVALID;

  if (cmp_str(btnname, "b1") == 0)
    return PinType_GPIO + PinType_ACTIVE_LOW;
  if (cmp_str(btnname, "b2") == 0)
    return PinType_GPIO + PinType_ACTIVE_LOW;

  return PinType_INVALID;
}

uint8_t get_btn_data(char const *btnname)
{
  if (cmp_str(btnname, "b1") == 0)
    return GPIO_LINE_14;
  if (cmp_str(btnname, "b2") == 0)
    return GPIO_LINE_13;

  return DATA_INVALID;
}



/* GPIO CHIP AND PIN INFO */

GPIO_Desc get_GPIO_Desc(GPIOS gpio_id){
  switch (gpio_id){
    case 0:
      return GPIOA_DESC;
    case 1:
      return GPIOB_DESC;
    case 2:
      return GPIOC_DESC;
    case 3:
      return GPIOD_DESC;
    case 4:
      return GPIOE_DESC;
    case 5:
      return GPIOF_DESC;
    case 6:
      return GPIOG_DESC;
    case 7:
      return GPIOH_DESC;
    case 8:
      return GPIOI_DESC;
    default:
      return GPIO_INVALID_DESC; 
  }
}


GPIO_Pin_desc get_pin_info(char* pin){
  if (pin == NULL)
    return GPIO_INVALID_PIN_DESC;
    
  /*
     expected pin names: Px1n, Pxn
     x = A-I
     n = 0-9
  */
  int len = strlen(pin);
  if (len < 3 || len > 4)
    return GPIO_INVALID_PIN_DESC;

  if (tolower(pin[0]) != 'p')
    return GPIO_INVALID_PIN_DESC;

  char *pinLine = pin+2; /* start where the number begins */  
  uint8_t lineNum;
  int temp;

  /* get the value for the last digit*/
  temp = pinLine[len == 3 ? 0 : 1] - '0';
  if (temp > 9)  
    return GPIO_INVALID_PIN_DESC;

  lineNum = temp; 

  if (len == 4 && pinLine[0] != '1')
    return GPIO_INVALID_PIN_DESC; 

  if (len == 4)
    lineNum += 10; /* line can never have a digit greater than 1 in the 10th place, so we can just add 10, if the lenght of the number is 2*/



  /* get chip name*/
  GPIOS chip;

  switch (tolower(pin[1])){
    case 'a':
      chip = GPIOA; break;
    case 'b':
      chip = GPIOB; break;
    case 'c':
      chip = GPIOC; break;
    case 'd':
      chip = GPIOD; break;
    case 'e':
      chip = GPIOE; break;
    case 'f':
      chip = GPIOF; break;
    case 'g':
      chip = GPIOG; break;
    case 'h':
      chip = GPIOH; break;
    case 'i':
      chip = GPIOI; break;
    default:
      return GPIO_INVALID_PIN_DESC; 
  }
 /* GPIO_Pin_desc */
 return (GPIO_Pin_desc) {chip,lineNum};
} 

