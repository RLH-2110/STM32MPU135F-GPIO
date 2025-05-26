#ifndef INCLUDED_LED_H
#define INCLUDED_LED_H 

#include <stdint.h>

/* Placdholder value for funcion parameters whose value is ingnored*/
#define NONE 0

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
#define GPIO_PIN_INPUT_PULL_DOWN      0x02 /**< Input Pull Down Mode */

#define I2C_0_ADDRESS 0x21

#define I2C_B0_IODIRA 0x00
#define I2C_B0_IODIRB 0x01
#define I2C_B0_GPIOA  0x12
#define I2C_B0_GPIOB  0x13
#define I2C_B1_GPIOA  0x09
#define I2C_B1_GPIOB  0x19

/* because %b for printf is not standard, so we can use these to printf in binary*/
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

/* functions returns:
   0: an invalid parameter was given
   1: parameters were valid
   *mmap: pointer to the start of the mapped memory for the gpio (? verify if this is correct)
   line: the line of the gpio pin, you can find it with `gpioinfo`
*/
int set_gpio_dir(void *mmap, unsigned int direction, uint8_t line); /* direction options: GPIO_PIN_INPUT_DIRECTION,  GPIO_PIN_OUTPUT_DIRECTION,  GPIO_PIN_ALT_FUNC_DIRECTION, GPIO_PIN_ANALOG_DIRECTION  */
int set_otype(void *mmap, unsigned int outputType, uint8_t line); /* output type options:  GPIO_PIN_OUTPUT_PUSHPULL, GPIO_PIN_OUTPUT_OPENDRAIN   */
int gpio_pin_set(void *mmap, unsigned int state, uint8_t line); /* state opions: 0: low, 1: high */
int set_pull_type(void *mmap, unsigned int pullType, uint8_t line); /* pullType options: GPIO_PIN_INPUT_NO_PULL, GPIO_PIN_INPUT_PULL_UP,  GPIO_PIN_INPUT_PULL_DOWN */

int gpio_pin_read(void *mmap, uint8_t line); /* returns 0 when line is low, and a non-zero value if its high. also returns 0 if line is greater than 15 */

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
int set_ic2_led(uint8_t regval, int state);

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


/* writes value to the mcp gpio
  bus:  selects which i2c chip you want to use (for more info "man i2cset")
  chipAdr: the address for the device that the chip is connected to
  state: 0: low 1: high
  abSelect: 0: gpioa 1: gpiob
  registerAdr: the address of the register you want to write to
  registerMask: masks what bit you want to set/unset

  returns 0: failure OR 1: success
*/
int i2c_set_bits(uint8_t bus, uint8_t chipAdr, unsigned int state, unsigned int abSelect, uint8_t registerMask);

/* uses the i2c command line tool to use the i2c bus
  read: true if you want to read a value, false if you want to write
  bus:  selects which i2c chip you want to use (for more info "man i2cset")
  chipAddress: the address for the device that the chip is connected to
  registerAdr: the address of the register you want to read/write to
  data: what you want to write (ignored if read = true)

  returns: 
    The return value of system is returned, but you should be able to assume this, if no error occures:
    if read is set, returns the read value.
    if read is false, returns 0.
    for more detailed info, "man system"
      
*/
uint8_t send_i2c_shell_comand(bool read, uint8_t bus, uint8_t chipAddress, uint8_t registerAdr, uint8_t data);

/* prints help screen
  progname: name of this program (argv[0])*/
void print_help(char *progname);

/* INCLUDED_LED_H */
#endif
