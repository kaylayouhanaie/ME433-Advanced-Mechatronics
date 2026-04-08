#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9
#define chip_address 0x20
#define IODIR 0x00
#define OLAT 0x0A
#define GPIO 0x09


void setPin(unsigned char reg, unsigned char value);
unsigned char readPin(unsigned char reg);

int main()
{
    stdio_init_all();

    const uint LED_PIN = 16;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);



    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

    // set IODIR for LED
    setPin(IODIR,0b01111111);

    while (true) {
        gpio_put(LED_PIN, 1); // LED On
        sleep_ms(500);
        gpio_put(LED_PIN, 0); // LED Off
        sleep_ms(500);

        uint8_t gpio_read_val = readPin(GPIO);

        if(gpio_read_val & 0x01){
            setPin(OLAT,0x00);
        } else {
            setPin(OLAT,0b10000000);
        }

        sleep_ms(100);
    }
}


void setPin(unsigned char reg, unsigned char value){
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = value;
    i2c_write_blocking(I2C_PORT, chip_address, buf, 2, false);
}

unsigned char readPin(unsigned char reg){
    unsigned char buf;
    i2c_write_blocking(I2C_PORT, chip_address, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, chip_address, &buf, 1, false);
    return buf;
}
