#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "math.h"

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS_DAC 17
#define PIN_CS_RAM 20 
#define PIN_SCK  18
#define PIN_MOSI 19

static inline void cs_select(uint cs_pin);
static inline void cs_deselect(uint cs_pin);

void spi_ram_init();
void spi_ram_write(uint16_t addr, uint8_t * data, int len);
void spi_ram_read(uint16_t addr, uint8_t * data, int len);
void ram_write_sin();
void update_DAC_from_RAM(int i);

static inline void cs_select(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); 
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop"); 
}

static inline void cs_deselect(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); 
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop"); 
}

int main()
{
    stdio_init_all();

    // SPI initialisation using SPI at 1MHz
    spi_init(spi_default, 1000 * 1000); // the baud, or bits per second
    gpio_set_function(PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
    
    // initialize CS pin for DAC
    gpio_set_function(PIN_CS_DAC, GPIO_FUNC_SIO);
    gpio_set_dir(PIN_CS_DAC, GPIO_OUT);
    gpio_put(PIN_CS_DAC, 1);

    // initialize CS pin for RAM
    gpio_set_function(PIN_CS_RAM, GPIO_FUNC_SIO);
    gpio_set_dir(PIN_CS_RAM, GPIO_OUT);
    gpio_put(PIN_CS_RAM, 1);

    spi_ram_init();
    ram_write_sin(); 

    int idx = 0;
    while (true) {
        update_DAC_from_RAM((uint16_t)(idx * 2));
        idx = (idx + 1) % 1024;
        sleep_ms(1);
    }
}


void spi_ram_init(){
    uint8_t data[2];
    int len = 2;
    data[0] = 0b00000001;
    data[1] = 0b01000000; // set to sequential mode
    cs_select(PIN_CS_RAM);
    spi_write_blocking(SPI_PORT, data, len);
    cs_deselect(PIN_CS_RAM);
}

void spi_ram_write(uint16_t addr, uint8_t * data, int len){
    uint8_t packet[5];
    packet[0] = 0b00000010;
    packet[1] = addr>>8;
    packet[2] = addr&0xFF;
    packet[3] = data[0];
    packet[4] = data[1];
    cs_select(PIN_CS_RAM);
    spi_write_blocking(SPI_PORT, packet, 5); // where data is a uint8_t array with length len
    cs_deselect(PIN_CS_RAM);
}

void spi_ram_read(uint16_t addr, uint8_t * data, int len){
    uint8_t packet[5];
    packet[0] = 0b00000011;
    packet[1] = addr>>8;
    packet[2] = addr&0xFF;
    packet[3] = 0;
    packet[4] = 0;
    uint8_t dst[5];
    cs_select(PIN_CS_RAM);
    spi_write_read_blocking(SPI_PORT, packet, dst, 5);
    cs_deselect(PIN_CS_RAM);
    data[0] = dst[3]; // simultaneous read/write, so we read the stuff during packet[3] and packet[4]
    data[1] = dst[4];
}

void ram_write_sin(){
    for (int i = 0; i < 1024; i++) {
        float voltage = (sinf(2.0f * M_PI * i / 1024) + 1.0f) / 2.0f * 3.3f;
        uint16_t theV = (uint16_t)(voltage / 3.3f * 1023);

        uint8_t buf[2];
        buf[0] = 0b01110000 | (theV >> 6);
        buf[1] = (theV << 2) & 0xFF;

        spi_ram_write((uint16_t)(i * 2), buf, 2);
    }
}

void update_DAC_from_RAM(int i){
    uint8_t data[2];
    int len = 2;
    spi_ram_read(i,data,2);
    cs_select(PIN_CS_DAC);
    spi_write_blocking(SPI_PORT, data, len);
    cs_deselect(PIN_CS_DAC);
}


