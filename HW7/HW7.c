#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "math.h"

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

void set_DAC(int channel, float v);
void update_DAC();
static inline void cs_select(uint cs_pin);
static inline void cs_deselect(uint cs_pin);

static inline void cs_select(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop"); // FIXME
}

static inline void cs_deselect(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop"); // FIXME
}

int main()
{
    stdio_init_all();

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(spi_default, 1000 * 1000); // the baud, or bits per second
    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

    float t = 0;
    while (true) {
        t=t+0.001;
        int f = 2;
        // sine wave on output A
        float voltageA = ((sin(2*M_PI*f*t) + 1)/2.0)*3.3;
        // triangle wave on output B
        float tri_pos = fmod(t,1.0);
        float voltageB;
        if(tri_pos < 0.5){
            voltageB = tri_pos * 2.0 * 3.3;
        } else {
            voltageB = (1.0 - tri_pos) * 2.0 * 3.3;
        }
        set_DAC(0,voltageA);
        set_DAC(1,voltageB);
        sleep_ms(1);
    }
}


void set_DAC(int channel, float v){
    int len = 2;
    uint8_t data[2]; // note: uint8_t is same as unsigned char
    data[0] = 0b01110000;
    data[0] = data[0] | ((channel & 0b1)<< 7); // put channel bit in right spot
    uint16_t theV = (v/3.3)*1023; // unit conversion voltage to bits. result is 10 bit number
    data[0] = data[0] | ((theV >> 6)&0b00001111);
    data[1] = (theV << 2) & 0xFF;
    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, data, len); // where data is a uint8_t array with length len
    cs_deselect(PIN_CS);
}


