// ssd1306.c merged with this file
//based on adafruit and sparkfun libraries
#include <stdio.h>
#include <string.h> // for memset
#include "ssd1306.h"
#include "font.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"

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

unsigned char SSD1306_ADDRESS = 0b0111100; // 7bit i2c address
unsigned char ssd1306_buffer[513]; // 128x32/8. Every bit is a pixel except first byte

void drawLetter(unsigned char x, unsigned char y, unsigned char letter);
void drawPixel(unsigned char x, unsigned char y, unsigned char color);
void drawMessage(unsigned char x, unsigned char y, char *m);

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

    // call ssd setup, clear, update here
    ssd1306_setup();
    ssd1306_clear();
    ssd1306_update();

    while (true) {
        // heartbeat LED
        gpio_put(LED_PIN, 1);
        sleep_ms(500);
        gpio_put(LED_PIN, 0);
        sleep_ms(500);
        //
        
        unsigned int t1 = to_us_since_boot(get_absolute_time()); // time at beginning

        // drawLetter example
        // unsigned char my_letter1 = 'K';
        // unsigned char my_letter2 = 'Y';
        // drawLetter(0,0,my_letter1);
        // drawLetter(7,0,my_letter2);

        char message1[50]; 
        sprintf(message1,"kaylakaylakaylakaylakayla");
        drawMessage(0,0,message1);

        char message2[50]; 
        sprintf(message2,"kaylakaylakaylakaylakayla");
        drawMessage(0,8,message2);

        char message3[50]; 
        sprintf(message3,"kaylakaylakaylakaylakayla");
        drawMessage(0,16,message3);

        // char message4[50]; 
        // sprintf(message4,"kaylakay");
        // drawMessage(0,24,message4);

        //read adc0
        adc_init();
        adc_gpio_init(26);
        adc_select_input(0);
        uint8_t read_adc0 = adc_read();

        // display adc0 value
        char message_adc[50]; 
        sprintf(message_adc, "ADC0=%d", read_adc0); 
        drawMessage(0,24,message_adc);

        // find and display fps
        unsigned int t2 = to_us_since_boot(get_absolute_time()); // time at end
        float runtime = (float)(t2 - t1)/1000000; // convert microseconds to seconds
        float fps = 1/runtime;
        char message_time[50]; 
        sprintf(message_time, "FPS=%f", fps); 
        drawMessage(50,24,message_time);

        ssd1306_update();
    }
}



void drawLetter(unsigned char x, unsigned char y, unsigned char letter){
    for (int i=0; i<5; i++){
        char col = ASCII[letter - 0x20][i];
        for (int j=0; j<7; j++){
            if ((col >> j) & 0b1 == 0b1){
                drawPixel(x+i, y+j, 1);
            } else {
                drawPixel(x+i, y+j, 0);
            }
        }

    }
}



void drawMessage(unsigned char x, unsigned char y, char *m){
    int i = 0;
    //int count = 0;
    while (m[i] != 0){
        drawLetter(x+i*5,y,m[i]);
        i = i+1;
        //count = count+1;
    }
}


void ssd1306_setup() {
    // first byte in ssd1306_buffer is a command
    ssd1306_buffer[0] = 0x40;
    // give a little delay for the ssd1306 to power up
    //_CP0_SET_COUNT(0);
    //while (_CP0_GET_COUNT() < 48000000 / 2 / 50) {
    //}
    sleep_ms(20);
    ssd1306_command(SSD1306_DISPLAYOFF);
    ssd1306_command(SSD1306_SETDISPLAYCLOCKDIV);
    ssd1306_command(0x80);
    ssd1306_command(SSD1306_SETMULTIPLEX);
    ssd1306_command(0x1F); // height-1 = 31
    ssd1306_command(SSD1306_SETDISPLAYOFFSET);
    ssd1306_command(0x0);
    ssd1306_command(SSD1306_SETSTARTLINE);
    ssd1306_command(SSD1306_CHARGEPUMP);
    ssd1306_command(0x14);
    ssd1306_command(SSD1306_MEMORYMODE);
    ssd1306_command(0x00);
    ssd1306_command(SSD1306_SEGREMAP | 0x1);
    ssd1306_command(SSD1306_COMSCANDEC);
    ssd1306_command(SSD1306_SETCOMPINS);
    ssd1306_command(0x02);
    ssd1306_command(SSD1306_SETCONTRAST);
    ssd1306_command(0x8F);
    ssd1306_command(SSD1306_SETPRECHARGE);
    ssd1306_command(0xF1);
    ssd1306_command(SSD1306_SETVCOMDETECT);
    ssd1306_command(0x40);
    ssd1306_command(SSD1306_DISPLAYON);
    ssd1306_clear();
    ssd1306_update();
}

// send a command instruction (not pixel data)
void ssd1306_command(unsigned char c) {
    //i2c_master_start();
    //i2c_master_send(ssd1306_write);
    //i2c_master_send(0x00); // bit 7 is 0 for Co bit (data bytes only), bit 6 is 0 for DC (data is a command))
    //i2c_master_send(c);
    //i2c_master_stop();

    uint8_t buf[2];
    buf[0] = 0x00;
    buf[1] =c;
    i2c_write_blocking(i2c_default, SSD1306_ADDRESS, buf, 2, false);
}

// update every pixel on the screen
void ssd1306_update() {
    ssd1306_command(SSD1306_PAGEADDR);
    ssd1306_command(0);
    ssd1306_command(0xFF);
    ssd1306_command(SSD1306_COLUMNADDR);
    ssd1306_command(0);
    ssd1306_command(128 - 1); // Width

    unsigned short count = 512; // WIDTH * ((HEIGHT + 7) / 8)
    unsigned char * ptr = ssd1306_buffer; // first address of the pixel buffer
    /*
    i2c_master_start();
    i2c_master_send(ssd1306_write);
    i2c_master_send(0x40); // send pixel data
    // send every pixel
    while (count--) {
        i2c_master_send(*ptr++);
    }
    i2c_master_stop();
    */

    i2c_write_blocking(i2c_default, SSD1306_ADDRESS, ptr, 513, false);
}

// set a pixel value. Call update() to push to the display)
void drawPixel(unsigned char x, unsigned char y, unsigned char color) {
    if ((x < 0) || (x >= 128) || (y < 0) || (y >= 32)) {
        return;
    }

    if (color == 1) {
        ssd1306_buffer[1 + x + (y / 8)*128] |= (1 << (y & 7));
    } else {
        ssd1306_buffer[1 + x + (y / 8)*128] &= ~(1 << (y & 7));
    }
}

// zero every pixel value
void ssd1306_clear() {
    memset(ssd1306_buffer, 0, 512); // make every bit a 0, memset in string.h
    ssd1306_buffer[0] = 0x40; // first byte is part of command
}







// we need to initialize i2c before calling display setup
// do not need pull up resistors, just 4 wires

// set pins watn to use
// call setup function
// go into loop
// ASCII letters are 5x8. first byte is first column, there are five columns
// write function drawLetter and then drawString
// sprintf converts message into number
// read value of timer, call all functions, then read time again to figure out time taken

// function pseudo code
// we have drawPixel already that takes (x,y,bit)
// ASCII(I,J): I is row which is which character you want
// before calling drawPixel:   
// char message[50]
// sprintf(message,"Hello")
// call drawMessage(message,x,y)
// drawLetter(message[0],x,y) : x,y is location of pixel on screen


// drawLetter(char L,x,y) function:
// for i = 0 to 4:
// char colm = ASCII(L)(i) <-- one 8 bit number
// drawPixel(x+i,y+j,bit)
// for j = 0 to 7:
// if colm >> j & 0b1 == 0b1:
// set it to 1 using drawPixel
// else its a 0


// drawMessage(char*m,x,y) function:
// i=0
// while m[i] != 0
// drawLetter(m[i],x+i*5,y)
// i=i+1


