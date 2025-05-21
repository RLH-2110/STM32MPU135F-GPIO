/*led应用程序*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>

#define MEM_DEV                 "/dev/mem"
#define GPIOA_START_ADDR        0x50002000
#define GPIOA_END_ADDR          0x500023FF
#define GPIOA_MAP_SIZE          (GPIOA_END_ADDR - GPIOA_START_ADDR)
 
#define GPIO_REG_MODER 0x00     /**< GPIO port mode register */
#define GPIO_REG_OTYPER 0x04    /**< GPIO port output type register */
#define GPIO_REG_BSRR 0x18      /**< GPIO port bit set/reset register */
 
#define GPIO_PIN_13                    ((uint16_t)0x2000U)

#define GPIO_PIN_OUTPUT_DIRECTION     0x01 /**< Output */
#define GPIO_PIN_OUTPUT_PUSHPULL      0x00 /**< Output Push Pull Mode */
#define GPIO_PIN_13_REG_DEFINITION   (GPIOA_START_ADDR+GPIO_PIN_13)
int main(int argc, char **argv)
{
  int ret = EXIT_SUCCESS;
  static void *mmapBase = NULL; /* Virtual base address */

 
  /* Try to open the mem device special file */
  int fdMem = open(MEM_DEV, O_RDWR | O_SYNC);
  if (fdMem < 1)
  {
    exit(1);
  }
  
  /* Map memory region for the gpio registers */
  mmapBase = mmap(NULL,
          GPIOA_MAP_SIZE,
          PROT_READ | PROT_WRITE,
          MAP_SHARED,
          fdMem,
          GPIOA_START_ADDR);
  if (mmapBase == (void*) -1)
  {
    exit(1);
  }
 
  { /* MODER */
    /* Load the different gpio register pointers with its address */
    volatile uint32_t *gpioRegAddr = mmapBase + GPIO_REG_MODER;
    /* Get the value of the gpio direction register */
    uint32_t gpioRegVal = *gpioRegAddr;
    /* Clear the GPIO, write it back to the direction register */    
    gpioRegVal &= ~(0x03 << (13 * 2)); /* mask out the 2 bits giving the direction */
    gpioRegVal |= ((GPIO_PIN_OUTPUT_DIRECTION) << (13 * 2));
    *gpioRegAddr = gpioRegVal;
  }
 
  { /* OTYPE */
    /* Load the different gpio register pointers with its address */
    volatile uint32_t *gpioRegAddr = mmapBase + GPIO_REG_OTYPER;
    /* Get the value of the gpio direction register */
    uint32_t gpioRegVal = *gpioRegAddr;
    /* Clear the GPIO, write it back to the direction register */    
    gpioRegVal &= ~(0x0 << (13)); /* mask out */
    gpioRegVal |= ((GPIO_PIN_OUTPUT_PUSHPULL) << (13));
    *gpioRegAddr = gpioRegVal;
  }
      
  volatile uint32_t *const gpioSetClearDataOutAddr = mmapBase + GPIO_REG_BSRR;

  if (argc == 2){
    if (strlen(argv[1]) == 2){ // like string "on"
      *gpioSetClearDataOutAddr = GPIO_PIN_13; /* set the pin */
      return ret;
    }
    if (strlen(argv[1] == 3){ // like string "off"
      *gpioSetClearDataOutAddr = (uint32_t)(GPIO_PIN_13 << 16); /* reset the pin */
      return ret;
    }
  }

  while(1)
  {
    *gpioSetClearDataOutAddr = GPIO_PIN_13; /* set the pin */
    usleep(50*1000);
    *gpioSetClearDataOutAddr = (uint32_t)(GPIO_PIN_13 << 16); /* reset the pin */
    usleep(50*1000);
  }
  return ret;
}

