#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"


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

    // initialize IMU
    setPin(PWR_MGMT_1,0x00);
    setPin(ACCEL_CONFIG,0b00000000);
    setPin(GYRO_CONFIG,0b00011000);

    
    while (true){
        gpio_put(LED_PIN, 1);
        // sleep_ms(100);
        // gpio_put(LED_PIN, 0);
        // sleep_ms(500);


        //read raw data
        unsigned char allRawData[14];
        readAllIMU(0x3B,allRawData);
        // for (int i = 0; i < 14; i++) {
        // printf("Reading %d: %d\r\n", i, allRawData[i]);
        // }

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
    

        float accelx = new_data[0];
        float accely = new_data[1];

        printf("(%f,%f)\n",accelx,accely);
        sleep_ms(1000/30);
        
    }
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