#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "font.h"
#include "hardware/adc.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9
#define IMU_ADDRESS 0x68
#define IODIR 0x00
#define OLAT 0x0A
#define GPIO 0x09

// config registers
#define CONFIG 0x1A
#define GYRO_CONFIG 0x1B
#define ACCEL_CONFIG 0x1C
#define PWR_MGMT_1 0x6B
#define PWR_MGMT_2 0x6C
// sensor data registers:
#define ACCEL_XOUT_H 0x3B
#define ACCEL_XOUT_L 0x3C
#define ACCEL_YOUT_H 0x3D
#define ACCEL_YOUT_L 0x3E
#define ACCEL_ZOUT_H 0x3F
#define ACCEL_ZOUT_L 0x40
#define TEMP_OUT_H   0x41
#define TEMP_OUT_L   0x42
#define GYRO_XOUT_H  0x43
#define GYRO_XOUT_L  0x44
#define GYRO_YOUT_H  0x45
#define GYRO_YOUT_L  0x46
#define GYRO_ZOUT_H  0x47
#define GYRO_ZOUT_L  0x48
#define WHO_AM_I     0x75


unsigned char SSD1306_ADDRESS = 0b0111100;
unsigned char ssd1306_buffer[513];
unsigned char readPin(unsigned char reg);
void setPin(unsigned char reg, unsigned char value);
void readAllIMU(unsigned char reg, unsigned char *buf);
int16_t convertData(unsigned char byte1, unsigned char byte2);

void drawLetter(unsigned char x, unsigned char y, unsigned char letter);
void drawPixel(unsigned char x, unsigned char y, unsigned char color);
void drawMessage(unsigned char x, unsigned char y, char *m);

int main()
{
    stdio_init_all();
    sleep_ms(1000);

    const uint LED_PIN = 16;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_setup();
    ssd1306_clear();
    ssd1306_update();

    // check IMU WHO_AM_I register
    uint8_t me = readPin(WHO_AM_I);
    char message_me[50]; 
    sprintf(message_me, "%d", me); 
    drawMessage(0,0,message_me);


    // initialize IMU
    setPin(PWR_MGMT_1,0x00);
    setPin(ACCEL_CONFIG,0b00000000);
    setPin(GYRO_CONFIG,0b00011000);

    

    while (true){
        gpio_put(LED_PIN, 1);
        sleep_ms(500);
        gpio_put(LED_PIN, 0);
        sleep_ms(500);
        printf("kayla\n");
        sleep_ms(100);

        //read raw data
        unsigned char allRawData[14];
        readAllIMU(0x3B,allRawData);
        for (int i = 0; i < 14; i++) {
        printf("Reading %d: %d\r\n", i, allRawData[i]);
        }

        // convert data to 16 bit integers
        float new_data[7];
        int index_count = 0;
        for (int i=0; i<14; i+=2){
            new_data[index_count] = convertData(allRawData[i], allRawData[i+1]);

            if (i<6){
                new_data[index_count] *= 0.000061; // convert accels to units of g
            }
            if (i>5 && i<8){
                new_data[index_count] = new_data[index_count]/340.0 + 36.53; // convert temp to units C
            }
            if (i>=8){
                new_data[index_count] *= 0.007630; // convert gyro to units of deg/sec
            }
            index_count++;
        }

        for (int i = 0; i < 7; i++) {
            printf("New Data %d: %f\r\n", i, new_data[i]);
        }

        // unsigned char my_letter = 'Y';
        // if (new_data[1] < 10){
        //     drawLetter(30,30,my_letter);
        // }
        ssd1306_update();
    }
    

    // int16_t accel = new_data[2];
    // char message_accel[50]; 
    // sprintf(message_accel, "%d", accel); 
    // drawMessage(0,30,message_accel);


}


void readAllIMU(unsigned char reg, unsigned char *buf){
    i2c_write_blocking(I2C_PORT, IMU_ADDRESS, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, IMU_ADDRESS, buf, 14, false);
}


unsigned char readPin(unsigned char reg){
    unsigned char buf;
    i2c_write_blocking(I2C_PORT, IMU_ADDRESS, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, IMU_ADDRESS, &buf, 1, false);
    return buf;
}

void setPin(unsigned char reg, unsigned char value){
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = value;
    i2c_write_blocking(I2C_PORT, IMU_ADDRESS, buf, 2, false);
}

int16_t convertData(unsigned char byte1, unsigned char byte2){
    int16_t converted_data = (byte1 << 8) | byte2;
    return converted_data;
}


/*
April 14th Lecture notes
look at datasheet to find bits to change for chihp initialization three registers

find which way is down by subtracting gyroscope from accelerometer

only care about x and y acceleration data

*/ 





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