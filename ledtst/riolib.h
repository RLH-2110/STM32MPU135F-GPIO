#ifndef INCLUDED_RIOLIB_H
#define INCLUDED_RIOLIB_H

#include <stdint.h>
#include <stdbool.h>

/* Placdholder value for funcion parameters whose value is ingnored*/
#define NONE 0

#define MEM_DEV                 "/dev/mem"

#define GPIOA_START_ADDR        0x50002000
#define GPIOA_END_ADDR          0x500023FF
#define GPIOA_MAP_SIZE          (GPIOA_END_ADDR - GPIOA_START_ADDR)
 
#define GPIOB_START_ADDR        0x50003000
#define GPIOB_END_ADDR          0x500033FF
#define GPIOB_MAP_SIZE          (GPIOB_END_ADDR - GPIOB_START_ADDR)

#define GPIOC_START_ADDR        0x50004000
#define GPIOC_END_ADDR          0x500043FF
#define GPIOC_MAP_SIZE          (GPIOC_END_ADDR - GPIOC_START_ADDR)
 
#define GPIOD_START_ADDR        0x50005000
#define GPIOD_END_ADDR          0x500053FF
#define GPIOD_MAP_SIZE          (GPIOD_END_ADDR - GPIOD_START_ADDR)
 
#define GPIOE_START_ADDR        0x50006000
#define GPIOE_END_ADDR          0x500063FF
#define GPIOE_MAP_SIZE          (GPIOE_END_ADDR - GPIOE_START_ADDR)
 
#define GPIOF_START_ADDR        0x50007000
#define GPIOF_END_ADDR          0x500073FF
#define GPIOF_MAP_SIZE          (GPIOF_END_ADDR - GPIOF_START_ADDR)
 
#define GPIOG_START_ADDR        0x50008000
#define GPIOG_END_ADDR          0x500083FF
#define GPIOG_MAP_SIZE          (GPIOG_END_ADDR - GPIOG_START_ADDR)
 
#define GPIOH_START_ADDR        0x50009000
#define GPIOH_END_ADDR          0x500093FF
#define GPIOH_MAP_SIZE          (GPIOH_END_ADDR - GPIOH_START_ADDR)
 
#define GPIOI_START_ADDR        0x5000A000
#define GPIOI_END_ADDR          0x5000A3FF
#define GPIOI_MAP_SIZE          (GPIOI_END_ADDR - GPIOI_START_ADDR)
 
#define GPIO_REG_MODER  0x00    /**< GPIO port mode register */
#define GPIO_REG_OTYPER 0x04    /**< GPIO port output type register */
#define GPIO_REG_IDR    0x10    /**< GPIO port input bits */
#define GPIO_REG_PUPDR  0x0C    /**< GPIO port pull direction register */
#define GPIO_REG_BSRR   0x18    /**< GPIO port bit set/reset register */

#define HIGH 0x01
#define LOW  0x00

#define GPIO_PIN_INPUT_DIRECTION      0b00 /**< Input  */
#define GPIO_PIN_OUTPUT_DIRECTION     0b01 /**< Output */
#define GPIO_PIN_ALT_FUNC_DIRECTION   0x02 /**< Alternate function mode */
#define GPIO_PIN_ANALOG_DIRECTION     0x04 /**< Analog mode */
#define GPIO_MAX_DIRECTION GPIO_PIN_ANALOG_DIRECTION               

#define GPIO_PIN_OUTPUT_PUSHPULL      0x00 /**< Output Push Pull Mode */
#define GPIO_PIN_OUTPUT_OPENDRAIN     0x01 /**< Output Open Drain Mode */
#define GPIO_MAX_OUTPUT_DIRECTION GPIO_PIN_OUTPUT_OPENDRAIN         

#define GPIO_PIN_INPUT_NO_PULL        0x00 /**< Input no Pull Up/Down Mode */
#define GPIO_PIN_INPUT_PULL_UP        0x01 /**< Input Pull Up Mode */
#define GPIO_PIN_INPUT_PULL_DOWN      0x02 /**< Input Pull Down Mode */
#define GPIO_MAX_INPUT_DIRECTION GPIO_PIN_INPUT_PULL_DOWN           

#define GPIO_MAX_LINE 15

#define I2C_0_ADDRESS 0x21

#define I2C_B0_IODIRA 0x00
#define I2C_B0_IODIRB 0x01
#define I2C_B0_GPIOA  0x12
#define I2C_B0_GPIOB  0x13
#define I2C_B1_GPIOA  0x09
#define I2C_B1_GPIOB  0x19

#define I2C_BUS_0 0
#define I2C_GPIOA_SELECT 0
#define I2C_GPIOB_SELECT 1

typedef enum : uint8_t { GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG, GPIOH, GPIOI, GPIO_INVALID = 0xFF} GPIOS;

typedef enum : uint16_t { PinType_INVALID = 0, PinType_GPIO, PinType_I2C, PinType_ACTIVE_LOW = 0x8000} PinType;
#define PinType_IS_ACTIVE_LOW(PIN) ((PIN & PinType_ACTIVE_LOW) == 0 ? false : true)
#define PinType_GET_CATEGORY_ONLY(PIN) (PIN & 0x7FFF)

typedef struct GPIO_Desc {
  off_t GPIO_START_ADDR;
  size_t GPIO_MAP_SIZE;
} GPIO_Desc;

typedef struct GPIO_Pin_desc {
  GPIOS gpioChip;
  uint8_t gpioLine;
} GPIO_Pin_desc;

static const GPIO_Pin_desc GPIO_INVALID_PIN_DESC = {GPIO_INVALID, 0};

static const GPIO_Desc GPIOA_DESC = {GPIOA_START_ADDR, GPIOA_MAP_SIZE};
static const GPIO_Desc GPIOB_DESC = {GPIOB_START_ADDR, GPIOB_MAP_SIZE};
static const GPIO_Desc GPIOC_DESC = {GPIOC_START_ADDR, GPIOC_MAP_SIZE};
static const GPIO_Desc GPIOD_DESC = {GPIOD_START_ADDR, GPIOD_MAP_SIZE};
static const GPIO_Desc GPIOE_DESC = {GPIOE_START_ADDR, GPIOE_MAP_SIZE};
static const GPIO_Desc GPIOF_DESC = {GPIOF_START_ADDR, GPIOF_MAP_SIZE};
static const GPIO_Desc GPIOG_DESC = {GPIOG_START_ADDR, GPIOG_MAP_SIZE};
static const GPIO_Desc GPIOH_DESC = {GPIOH_START_ADDR, GPIOH_MAP_SIZE};
static const GPIO_Desc GPIOI_DESC = {GPIOI_START_ADDR, GPIOI_MAP_SIZE};
static const GPIO_Desc GPIO_INVALID_DESC = {GPIO_INVALID, 0};

/* functions returns:
   -1: an invalid parameter was given
   0: parameters were valid
   *mmap: pointer to the start of the mapped memory for the gpio 
   line: the line of the gpio pin, you can find it with `gpioinfo`
*/
int set_gpio_dir(void *mmap, unsigned int direction, uint8_t line); /* direction options: GPIO_PIN_INPUT_DIRECTION,  GPIO_PIN_OUTPUT_DIRECTION,  GPIO_PIN_ALT_FUNC_DIRECTION, GPIO_PIN_ANALOG_DIRECTION  */
int set_otype(void *mmap, unsigned int outputType, uint8_t line); /* output type options:  GPIO_PIN_OUTPUT_PUSHPULL, GPIO_PIN_OUTPUT_OPENDRAIN   */
int gpio_pin_set(void *mmap, unsigned int state, uint8_t line); /* state opions: 0: low, 1: high */
int set_pull_type(void *mmap, unsigned int pullType, uint8_t line); /* pullType options: GPIO_PIN_INPUT_NO_PULL, GPIO_PIN_INPUT_PULL_UP,  GPIO_PIN_INPUT_PULL_DOWN */

int gpio_pin_read(void *mmap, uint8_t line); /* returns 0 when line is low, and a non-zero value if its high. also returns 0 if line is greater than 15 */


/* writes value to the mcp gpio
  bus:  selects which i2c chip you want to use (for more info "man i2cset")
  chipAdr: the address for the device that the chip is connected to
  state: 0: low 1: high
  abSelect: 0: gpioa 1: gpiob
  registerAdr: the address of the register you want to write to
  registerMask: masks what bit you want to set/unset

  returns -1: failure OR 0: success
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
    if read is set, returns the read value *(1)
    if read is false, returns 0 *(1)
    *(1) these are only true if there are no errors, if an error occured, you will find the error codes returned from system. for more detailed info, "man system"
      
*/
uint8_t send_i2c_shell_comand(bool read, uint8_t bus, uint8_t chipAddress, uint8_t registerAdr, uint8_t data);

/* initalizes mmapBase, which is needed for all gpio functions
  **mmapBase: output parameter that will be set to a pointer to memory
  gpioStartAddr: start address for the gpio, use MACROS like GPIOA_START_ADDR

  returns: -1 on error and 0 on success
*/
int gpio_init(void **mmapBase, GPIO_Desc gpio_desc);

/* sets up the gpio line (setting it to output and pushpull) and then writes to it.
   *mmapBase: pointer to the start of the mapped memory for the gpio 
   line: the line of the gpio pin, you can find it with `gpioinfo`
   state: what the line will be set to. 0: LOW  1: HIGH
*/
int gpio_pin_set_ws(void *mmapBase, int state, uint8_t line);






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
PinType get_led_type(char const *ledname);

/* returns data needed to identify the LED, check if the LED is valid before using the function, by using get_led_type*/
uint8_t get_led_data(char const *ledname);


/* returns data needed to identify the Button, check if the Button is valid before using the function, by using get_btn_type*/
uint8_t get_btn_data(char const *ledname);

/*
  gets the type of button. If the Most Significant bit is set, then its active low, if not, its active high
  btnname: name of the button (ie. B1, B2)

  values:
    0: invalid BTN
    1: GPIO0 BTN
    2: I2C0 BTN
*/
PinType get_btn_type(char const *btnname);

/* gets the GPIO_Desc by the id of the gpio. returns GPIO_INVALID_DESC, if an invalid id was used. */
GPIO_Desc get_GPIO_Desc(GPIOS gpio_id);

/* finds out what gpio chip and line a pin belongs to
  pin: the name of the pin

  returns: a GPIO_Pin_desc with the info. Returns GPIO_INVALID_PIN_DESC, if the pin is invalid*/
GPIO_Pin_desc get_pin_info(char* pin);

/* INCLUDED_RIOLIB_H */
#endif
