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
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

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

int i2c_init(char *device, uint8_t address){
  if (device == NULL)
    return RETURN_ERROR;
 
  int mcp = open(device, O_RDWR);
  if (mcp == -1)
    return RETURN_ERROR;

  if (ioctl(mcp, I2C_SLAVE, address) == -1){
    close(mcp);
    return RETURN_ERROR;
  } 
  return mcp;
}

void i2c_cleanup(int mcp){
  if (mcp != -1)
    close(mcp);
}

/* translates bus 0 address to a bus 1, if usingBus0 is false.
returns new address on success, and -1 on failure
*/
static uint8_t i2c_bus_addr_translation(bool usingBus0, int registerAddress){
  if (usingBus0)
    return registerAddress;

  switch (registerAddress){
    case I2C_B0_IODIRA:
      return I2C_B1_IODIRA;

    case I2C_B0_IODIRB:
      return I2C_B1_IODIRB;

    case I2C_B0_GPIOA:
      return I2C_B1_GPIOA;

    case I2C_B0_GPIOB:
      return I2C_B1_GPIOB;

    default:
      return RETURN_ERROR;
  }
}

/* translates port a addresses to port b address, if usingGPIA is false.
returns new address on success, and -1 on failure
*/
static uint8_t i2c_port_addr_translation(bool usingGPIOA, int registerAddress){
  if (usingGPIOA)
    return registerAddress;

  switch (registerAddress){
    case I2C_B0_IODIRA:
      return I2C_B0_IODIRB;

    /*case I2C_B1_IODIRA: *//* duplicate case, because I2C_B0_IODIRA == I2C_B1_IODIRA*//*
      return I2C_B1_IODIRB;
    */
    case I2C_B0_GPIOA:
      return I2C_B0_GPIOB;

    case I2C_B1_GPIOA:
      return I2C_B1_GPIOB;

    default:
      return RETURN_ERROR;
  }
}

uint8_t i2c_get_port(bool usingGPIOA,bool usingBus0, int registerAddress){
  return i2c_bus_addr_translation(usingBus0,i2c_port_addr_translation(usingGPIOA, registerAddress)); 
}

uint8_t use_i2c_gpio(bool readData, int mcp, uint8_t registerAdr, uint8_t data, uint8_t *out_readData)
{
  if (mcp == -1)
    return RETURN_ERROR;
    
  uint8_t buff[2]; 
  buff[0] = registerAdr;

  if (readData){
    if (out_readData == NULL)
      return RETURN_ERROR;
    
    int tmp;
    if(write(mcp,buff,1) != 1)
      return RETURN_ERROR;

    if (read(mcp, &tmp, 1) != 1)    
      return RETURN_ERROR;

    *out_readData = tmp;  
  }
  else{ /* !readData */
    buff[1] = data;
    if(write(mcp,buff,2) != 2)
      return RETURN_ERROR;
  }

  return RETURN_SUCCESS;
}

#define i2c_set_bits_ERROR_CLEAN { close(mcp);  return RETURN_ERROR;}
int i2c_set_bits(uint8_t chipAdr, unsigned int state, bool portA, bool bus0, uint8_t registerMask)
{
  if (state > HIGH)
    return RETURN_ERROR;
 
  int mcp = i2c_init(I2C_0_DEVICE,chipAdr);
  if (mcp == -1)
    return RETURN_ERROR;
  

  /* set unmaked state */ 
  uint8_t unmaskedState = 0x00; /* all off*/
  if(state == HIGH)
    unmaskedState = 0xFF; /* all on*/

  /* set the selected bits to output*/
  uint8_t outputs;
  if (use_i2c_gpio(true, mcp, i2c_get_port(portA,bus0,I2C_B0_IODIRA), NONE, &outputs) != RETURN_SUCCESS)
    i2c_set_bits_ERROR_CLEAN
  
  outputs = outputs & (~registerMask); /* set only the bits we selected to output */
  if (use_i2c_gpio(false, mcp, i2c_get_port(portA,bus0,I2C_B0_IODIRA), outputs, NULL) != RETURN_SUCCESS)
    i2c_set_bits_ERROR_CLEAN

  /* write to the selected bits */
  if (use_i2c_gpio(true, mcp, i2c_get_port(portA,bus0,I2C_B0_GPIOA), NONE, &outputs) != RETURN_SUCCESS)
    i2c_set_bits_ERROR_CLEAN

  uint8_t registerVal = (outputs & (~registerMask)) | (unmaskedState & registerMask); /* set/unset the selected bits*/
  if (use_i2c_gpio(false, mcp, i2c_get_port(portA,bus0,I2C_B0_GPIOA), registerVal, NULL) != RETURN_SUCCESS)
    i2c_set_bits_ERROR_CLEAN
  
  close(mcp); 
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


int gpio_init(GPIO_cleanup *out_data, GPIO_Desc gpio_desc)
{
  if (out_data == NULL)
    return RETURN_ERROR;

  out_data->fdMem = open(MEM_DEV, O_RDWR | O_SYNC);
  if (out_data->fdMem < 1)
    return RETURN_ERROR;

  out_data->mmapBase = mmap(NULL,gpio_desc.GPIO_MAP_SIZE,PROT_READ | PROT_WRITE, MAP_SHARED, out_data->fdMem, gpio_desc.GPIO_START_ADDR);
  if (out_data->mmapBase == (void*) -1)
    return RETURN_ERROR;
  
  out_data->size = gpio_desc.GPIO_MAP_SIZE;

  return RETURN_SUCCESS;
}

int gpio_cleanup(GPIO_cleanup data)
{
  if(close(data.fdMem) == -1)
    return RETURN_ERROR; 
  return munmap(data.mmapBase,data.size);
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

