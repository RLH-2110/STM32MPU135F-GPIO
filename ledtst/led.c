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

 
#define GPIO_REG_MODER  0x00    /**< GPIO port mode register */
#define GPIO_REG_OTYPER 0x04    /**< GPIO port output type register */
#define GPIO_REG_IDR    0x10    /**< GPIO port input bits */ 
#define GPIO_REG_PUPDR  0x0C    /**< GPIO port pull direction register */
#define GPIO_REG_BSRR   0x18    /**< GPIO port bit set/reset register */

#define HIGH 0x01
#define LOW  0x00

#define GPIO_PIN_INPUT_DIRECTION      0x00 /**< Input  */
#define GPIO_PIN_OUTPUT_DIRECTION     0x01 /**< Output */
#define GPIO_PIN_ALT_FUNC_DIRECTION   0x10 /**< Alternate function mode */
#define GPIO_PIN_ANALOG_DIRECTION     0x11 /**< Analog mode */

#define GPIO_PIN_OUTPUT_PUSHPULL      0x00 /**< Output Push Pull Mode */
#define GPIO_PIN_OUTPUT_OPENDRAIN     0x01 /**< Output Open Drain Mode */

#define GPIO_PIN_INPUT_NO_PULL        0x00 /**< Input no Pull Up/Down Mode */
#define GPIO_PIN_INPUT_PULL_UP        0x01 /**< Input Pull Up Mode */
#define GPIO_PIN_INPUT_PULL_Down      0x02 /**< Input Pull Down Mode */


/* #define GPIO_PIN_13_REG_DEFINITION   (GPIOA_START_ADDR+GPIO_PIN_13) */

/* functions returns:
   0: an invalid parameter was given
   1: parameters were valid 
   *mmap: pointer to the start of the mapped memory for the gpio (? verify if this is correct)
   line: the line of the gpio pin, you can find it with `gpioinfo`
*/
int set_gpio_dir(void *mmap, unsigned int direction, unsigned int line); /* direction options: GPIO_PIN_INPUT_DIRECTION,  GPIO_PIN_OUTPUT_DIRECTION,  GPIO_PIN_ALT_FUNC_DIRECTION,GPIO_PIN_ANALOG_DIRECTION  */
int set_otype(void *mmap, unsigned int outputType, unsigned int line); /* output type options:  GPIO_PIN_OUTPUT_PUSHPULL, GPIO_PIN_OUTPUT_OPENDRAIN   */
int gpio_pin_set(void *mmap, unsigned int state,unsigned int line); /* state opions: 0: low, 1: high */
int set_pull_type(void *mmap, unsigned int pullType, unsigned int line); /* pullType options: GPIO_PIN_INPUT_NO_PULL, GPIO_PIN_INPUT_PULL_UP,  GPIO_PIN_INPUT_PULL_Down */

int gpio_pin_read(void *mmap, unsigned int line); /* returns 0 when line is low, and a non-zero value if its high. also returns 0 if line is greater than 15 */

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
 
  /* MODER */
  set_gpio_dir(mmapBase, GPIO_PIN_OUTPUT_DIRECTION, 13); 
  
  /* OTYPE */ 
  set_otype(mmapBase, GPIO_PIN_OUTPUT_PUSHPULL, 13);

  if (argc == 2){
    if (strlen(argv[1]) == 1){ /* like string "h" */
      puts("\nhelp:\n\th or any 1 char string: prints this screen.\n\ton or any 2 char string: toggles pin on\n\toff or any 3 char string: toggles pin off.\n\tread or any 4 char sting: prints when buttion is pressed\n\t[no arg]:  toggles pin on and off");
    }
    if (strlen(argv[1]) == 2){ /* like string "on" */
      gpio_pin_set(mmapBase, 1, 13);
      return ret;
    }
    if (strlen(argv[1]) == 3){ /* like string "off" */
      gpio_pin_set(mmapBase, 0, 13);
      return ret;
    }
    if (strlen(argv[1]) == 4){ /* like string "read" */

      usleep(50*10000); /* better wait a bit, probably unessesary. */
      set_gpio_dir(mmapBase, GPIO_PIN_INPUT_DIRECTION, 13); 
      set_pull_type(mmapBase, GPIO_PIN_INPUT_NO_PULL, 13);
      
      int i = 1;      
      while(1){
        
        if (gpio_pin_read(mmapBase, 13) != 0){
          printf("%d. Read\n",i); i+=1;
        }
        usleep(50*10000);
   
      }
      return ret;
    }
  }

  while(1)
  {
    /**gpioSetClearDataOutAddr = GPIO_PIN_13; *//* set the pin */
    gpio_pin_set(mmapBase, 1, 13);
    usleep(50*10000);

    /**gpioSetClearDataOutAddr = (uint32_t)(GPIO_PIN_13 << 16); *//* reset the pin */
    gpio_pin_set(mmapBase, 0, 13);
    usleep(50*10000);
  }
  return ret;
}


int set_gpio_dir(void *mmap, unsigned int direction, unsigned int line)
{ /* MODER */
  if (direction > 0x3)
    return 0;
  if (line > 15)
    return 0;
 

 /* Load the different gpio register pointers with its address */
  volatile uint32_t *gpioRegAddr = mmap + GPIO_REG_MODER;
  /* Get the value of the gpio direction register */
  uint32_t gpioRegVal = *gpioRegAddr;
  /* Clear the GPIO, write it back to the direction register */    
  gpioRegVal &= ~(0x03 << (line * 2)); /* mask out the 2 bits giving the direction */
  gpioRegVal |= ((direction) << (line * 2));
  *gpioRegAddr = gpioRegVal;

  return 1;
}
 
int set_otype(void *mmap, unsigned int outputType, unsigned int line)
{ /* OTYPE */
  if (line > 15)
    return 0;
  if (outputType > 0x1)
    return 0;
 
  /* Load the different gpio register pointers with its address */
  volatile uint32_t *gpioRegAddr = mmap + GPIO_REG_OTYPER;
  /* Get the value of the gpio direction register */
  uint32_t gpioRegVal = *gpioRegAddr;
  /* Clear the GPIO, write it back to the direction register */    
  gpioRegVal &= ~(0x0 << (line)); /* mask out */
  gpioRegVal |= ((outputType) << (line));
  *gpioRegAddr = gpioRegVal;
  return 1;
}

int set_pull_type(void *mmap, unsigned int pullType, unsigned int line)
{ /* PUPDR */
  if (line > 15)
    return 0;
  if (pullType > 0x2)
    return 0;
 
  /* Load the different gpio register pointers with its address */
  volatile uint32_t *gpioRegAddr = mmap + GPIO_REG_PUPDR;
  /* Get the value of the gpio direction register */
  uint32_t gpioRegVal = *gpioRegAddr;
  /* Clear the GPIO, write it back to the direction register */    
  gpioRegVal &= ~(0x0 << (line)); /* mask out */
  gpioRegVal |= ((pullType) << (line));
  *gpioRegAddr = gpioRegVal;
  return 1;
}

int gpio_pin_read(void *mmap, unsigned int line){
  if (line > 15)
    return 0;
  
  volatile uint32_t *const gpioSetClearDataOutAddr = mmap + GPIO_REG_IDR;
  
  return ((uint16_t)*gpioSetClearDataOutAddr) & (1 << line); /* read the pin */
}
 
int gpio_pin_set(void *mmap, unsigned int state,unsigned int line){
  if (state > 0x1)
    return 0;
  if (line > 15)
    return 0;

  volatile uint32_t *const gpioSetClearDataOutAddr = mmap + GPIO_REG_BSRR;
  
  if (state == 1) 
    *gpioSetClearDataOutAddr = (uint16_t)(1 << line); /* set the pin */
  else 
    *gpioSetClearDataOutAddr = (uint32_t)((1 << line) << 16); /* reset the pin */
  
  return 1;
}
