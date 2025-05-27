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



/*        I2C FUNCTIONS          */


uint8_t send_i2c_shell_comand(bool read, uint8_t bus, uint8_t chipAddress, uint8_t registerAdr, uint8_t data)
{
  if (bus > 99){
    puts("bus greater than 99 is not supported");
    return -1;
  }
  char command[] = "i2cset -y 00 0x21 0x01 0x00\0      ";
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
  return 0; /* should not be reachable */
}

int i2c_set_bits(uint8_t bus, uint8_t chipAdr, unsigned int state, unsigned int abSelect, uint8_t registerMask)
{
  if (state > 0x1)
    return -1;
  if (abSelect > 0x1)
    return -1;

  /* select correct iodir register */
  uint8_t iodir = I2C_B0_IODIRA;
  if (abSelect == 1)
    iodir = I2C_B0_IODIRB;
  
  /* select correct gpio register*/
  uint8_t gpio = I2C_B0_GPIOA;
  if (abSelect == 1)
    gpio = I2C_B0_GPIOB;
 
  /* set unmaked state */ 
  uint8_t unmaskedState = 0x00;
  if(state == HIGH)
    unmaskedState = 0xFF;

  /* set the selected bits to output*/
  int outputs = send_i2c_shell_comand(true, bus, chipAdr, iodir, NONE);
  outputs = outputs & (~registerMask); /* set only the bits we selected to output */
  if (send_i2c_shell_comand(false, bus, chipAdr, iodir, outputs) != 0)
    return -1;

  /* write to the selected bits */
  outputs = send_i2c_shell_comand(true, bus, chipAdr, gpio, NONE);
  int registerVal = (outputs & (~registerMask)) | (unmaskedState & registerMask); /* set/unset the selected bits*/
  if (send_i2c_shell_comand(false, bus, chipAdr, gpio, registerVal) != 0)
    return -1;
  
  return 0;
}


/*       GPIO FUNCTIONS          */


int set_gpio_dir(void *mmap, unsigned int direction, uint8_t line)
{ /* MODER */
  if (direction > 0x3)
    return -1;
  if (line > 15)
    return -1;
 

 /* Load the different gpio register pointers with its address */
  volatile uint32_t *gpioRegAddr = mmap + GPIO_REG_MODER;
  /* Get the value of the gpio direction register */
  uint32_t gpioRegVal = *gpioRegAddr;
  /* Clear the GPIO, write it back to the direction register */    
  gpioRegVal &= ~(0x03 << (line * 2)); /* mask out the 2 bits giving the direction */
  gpioRegVal |= ((direction) << (line * 2));
  *gpioRegAddr = gpioRegVal;

  return 0;
}
 
int set_otype(void *mmap, unsigned int outputType, uint8_t line)
{ /* OTYPE */
  if (line > 15)
    return -1;
  if (outputType > 0x1)
    return -1;
 
  /* Load the different gpio register pointers with its address */
  volatile uint32_t *gpioRegAddr = mmap + GPIO_REG_OTYPER;
  /* Get the value of the gpio direction register */
  uint32_t gpioRegVal = *gpioRegAddr;
  /* Clear the GPIO, write it back to the direction register */    
  gpioRegVal &= ~(0x0 << (line)); /* mask out */
  gpioRegVal |= ((outputType) << (line));
  *gpioRegAddr = gpioRegVal;
  return 0;
}

int set_pull_type(void *mmap, unsigned int pullType, uint8_t line)
{ /* PUPDR */
  if (line > 15)
    return -1;
  if (pullType > 0x2)
    return -1;
 
  /* Load the different gpio register pointers with its address */
  volatile uint32_t *gpioRegAddr = mmap + GPIO_REG_PUPDR;
  /* Get the value of the gpio direction register */
  uint32_t gpioRegVal = *gpioRegAddr;
  /* Clear the GPIO, write it back to the direction register */    
  gpioRegVal &= ~(0x0 << (line)); /* mask out */
  gpioRegVal |= ((pullType) << (line));
  *gpioRegAddr = gpioRegVal;
  return 0;
}

int gpio_pin_read(void *mmap, uint8_t line)
{
  if (line > 15)
    return -1;
  
  volatile uint32_t *const gpioSetClearDataOutAddr = mmap + GPIO_REG_IDR;
  
  return ((uint16_t)*gpioSetClearDataOutAddr) & (1 << line); /* read the pin */
}
 
int gpio_pin_set(void *mmap, unsigned int state, uint8_t line)
{
  if (state > 0x1)
    return -1;
  if (line > 15)
    return -1;

  volatile uint32_t *const gpioSetClearDataOutAddr = mmap + GPIO_REG_BSRR;
  
  if (state == 1) 
    *gpioSetClearDataOutAddr = (uint16_t)(1 << line); /* set the pin */
  else 
    *gpioSetClearDataOutAddr = (uint32_t)((1 << line) << 16); /* reset the pin */
  
  return 0;
}


int gpio_init(void **mmapBase, GPIO_Desc gpio_desc)
{
  if (mmapBase == NULL)
    return -1;

  int fdMem = open(MEM_DEV, O_RDWR | O_SYNC);
  if (fdMem < 1)
    return -1;

  *mmapBase = mmap(NULL,gpio_desc.GPIO_MAP_SIZE,PROT_READ | PROT_WRITE, MAP_SHARED, fdMem, gpio_desc.GPIO_START_ADDR);
  if (*mmapBase == (void*) -1)
    return -1;
  
  return 0;
}

int gpio_pin_set_ws(void *mmapBase, int state,uint8_t line) 
{
  if (set_gpio_dir(mmapBase, GPIO_PIN_OUTPUT_DIRECTION, line) != 0)
    return -1; 
  
  if (set_otype(mmapBase, GPIO_PIN_OUTPUT_PUSHPULL, line) != 0)
    return -1;

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
/*
  gets the type of button. If the Most Significant bit is set, then its active low, if not, its active high
  btnname: name of the button (ie. B1, B2)

  values:
    0: invalid BTN
    1: GPIO0 BTN
    2: I2C0 BTN 
*/
uint16_t get_btn_type(char const *btnname){
  if (cmp_str(btnname, "b1") == 0)
    return 0x8001;
  if (cmp_str(btnname, "b2") == 0)
    return 0x8001;
}
uint8_t get_btn_data(char const *btnname)
{
  if (cmp_str(btnname, "b1") == 0)
    return 14;
  if (cmp_str(btnname, "b2") == 0)
    return 13;

  return 0;
}
