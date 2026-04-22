/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board_api.h"
#include "tusb.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "font.h"

#include "usb_descriptors.h"

#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9
#define IMU_ADDRESS 0x68
#define IODIR 0x00
#define OLAT 0x0A
#define GPIO 0x09
#define BUTTON_PIN 17
#define LED_PIN 16

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


unsigned char readPin(unsigned char reg);
void setPin(unsigned char reg, unsigned char value);
void readAllIMU(unsigned char reg, unsigned char *buf);
int16_t convertData(unsigned char byte1, unsigned char byte2);
void drawLetter(unsigned char x, unsigned char y, unsigned char letter);
void drawPixel(unsigned char x, unsigned char y, unsigned char color);
void drawMessage(unsigned char x, unsigned char y, char *m);

unsigned char SSD1306_ADDRESS = 0b0111100;
unsigned char ssd1306_buffer[513];


//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void led_blinking_task(void);
void hid_task(void);



/*------------- MAIN -------------*/
int main(void)
{
  //stdio_init_all();
  board_init();
  i2c_init(I2C_PORT, 400*1000);

  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(I2C_SDA);
  gpio_pull_up(I2C_SCL);

  // initialize IMU
  setPin(PWR_MGMT_1,0x00);
  setPin(ACCEL_CONFIG,0b00000000);
  setPin(GYRO_CONFIG,0b00011000);

  // initialize LED
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, 1);

  // initialize button
  gpio_init(BUTTON_PIN);
  gpio_set_dir(BUTTON_PIN, GPIO_IN);
  gpio_pull_up(BUTTON_PIN);

  ssd1306_setup();
  ssd1306_clear();
  ssd1306_update();

  // init device stack on configured roothub port
  tud_init(BOARD_TUD_RHPORT);

  if (board_init_after_tusb) {
    board_init_after_tusb();
  }

  uint8_t me = readPin(WHO_AM_I);
  char message_me[50]; 
  sprintf(message_me, "%d", me); 
  drawMessage(20,25,message_me);

  while (true)
  {
    tud_task(); // tinyusb device task
    led_blinking_task();
    
    hid_task(); // update bits based on what you need to do (change this only)
  }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED;
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

static void send_hid_report(uint8_t report_id, uint32_t btn)
{
  // skip if hid is not ready yet
  if ( !tud_hid_ready() ) return;

  switch(report_id)
  {
    case REPORT_ID_KEYBOARD:
    {
      // use to avoid send multiple consecutive zero report for keyboard
      static bool has_keyboard_key = false;

      if ( btn )
      {
        uint8_t keycode[6] = { 0 };
        keycode[0] = HID_KEY_A;

        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
        has_keyboard_key = true;
      }else
      {
        // send empty key report if previously has key pressed
        if (has_keyboard_key) tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
        has_keyboard_key = false;
      }
    }
    break;

    case REPORT_ID_MOUSE:
    {
      // generally delta should be small (like 5, not 100)

      // take xaccel and y accel from IMU, put into deltax and deltay
      // but make sure to convert to a small enough number:
      // bin the IMU data or divide by some large number to convert 


      // read button
      uint8_t button = gpio_get(BUTTON_PIN);

      //Move Mouse In Square (mode=0):
      if (button==1){
        gpio_put(LED_PIN, 1);
        int8_t deltax = 0;
        int8_t deltay = 0;
        static int time = 0;
        static int direction = 0;
          
        if (direction==0){ // move right
        deltax = 5;
        deltay = 0;
        }
        if (direction==1){ // move down
        deltax = 0;
        deltay = 5;
        }
        if (direction==2){ // left
        deltax = -5;
        deltay = 0;
        }
        if (direction==3){ // up
        deltax = 0;
        deltay = -5;
        }
        if (direction>3){
        direction=0;
        }
        tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, deltax, deltay, 0, 0);
        time++;
        if (time==10){ // this number is how long to move in a given direction before changing
        direction = direction + 1;
        time = 0;
        }
      }

      // Move Mouse Using IMU
      if (button==0){
        gpio_put(LED_PIN, 0);
        //read raw data
        unsigned char allRawData[14];
        readAllIMU(0x3B,allRawData);

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

        char message_accelx[50]; 
        sprintf(message_accelx, "ACCELX=%f", accelx); 
        drawMessage(0,0,message_accelx);
        char message_accely[50]; 
        sprintf(message_accely, "ACCELY=%f", accely); 
        drawMessage(0,9,message_accely);
        ssd1306_update();

        int8_t deltax = -accelx*10;
        int8_t deltay = -accely*10;

        tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, deltax, deltay, 0, 0);
      }

      
      
    }
    break;

    case REPORT_ID_CONSUMER_CONTROL:
    {
      // use to avoid send multiple consecutive zero report
      static bool has_consumer_key = false;

      if ( btn )
      {
        // volume down
        uint16_t volume_down = HID_USAGE_CONSUMER_VOLUME_DECREMENT;
        tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &volume_down, 2);
        has_consumer_key = true;
      }else
      {
        // send empty key report (release key) if previously has key pressed
        uint16_t empty_key = 0;
        if (has_consumer_key) tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &empty_key, 2);
        has_consumer_key = false;
      }
    }
    break;

    case REPORT_ID_GAMEPAD:
    {
      // use to avoid send multiple consecutive zero report for keyboard
      static bool has_gamepad_key = false;

      hid_gamepad_report_t report =
      {
        .x   = 0, .y = 0, .z = 0, .rz = 0, .rx = 0, .ry = 0,
        .hat = 0, .buttons = 0
      };

      if ( btn )
      {
        report.hat = GAMEPAD_HAT_UP;
        report.buttons = GAMEPAD_BUTTON_A;
        tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));

        has_gamepad_key = true;
      }else
      {
        report.hat = GAMEPAD_HAT_CENTERED;
        report.buttons = 0;
        if (has_gamepad_key) tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
        has_gamepad_key = false;
      }
    }
    break;

    default: break;
  }
}

// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
// this function is what we're changing
void hid_task(void)
{
  // Poll every 10ms
  const uint32_t interval_ms = 10;
  static uint32_t start_ms = 0;

  if ( board_millis() - start_ms < interval_ms) return; // not enough time
  start_ms += interval_ms;

  uint32_t const btn = board_button_read();

  // Remote wakeup
  if ( tud_suspended() && btn )
  {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    tud_remote_wakeup();
  }else
  {
    // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
    send_hid_report(REPORT_ID_MOUSE, btn); // we changed this to mouse from keyboard
  }
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
  (void) instance;
  (void) len;

  uint8_t next_report_id = report[0] + 1u;

  if (next_report_id < REPORT_ID_COUNT)
  {
    send_hid_report(next_report_id, board_button_read());
  }
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  (void) instance;

  if (report_type == HID_REPORT_TYPE_OUTPUT)
  {
    // Set keyboard LED e.g Capslock, Numlock etc...
    if (report_id == REPORT_ID_KEYBOARD)
    {
      // bufsize should be (at least) 1
      if ( bufsize < 1 ) return;

      uint8_t const kbd_leds = buffer[0];

      if (kbd_leds & KEYBOARD_LED_CAPSLOCK)
      {
        // Capslock On: disable blink, turn led on
        blink_interval_ms = 0;
        board_led_write(true);
      }else
      {
        // Caplocks Off: back to normal blink
        board_led_write(false);
        blink_interval_ms = BLINK_MOUNTED;
      }
    }
  }
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // blink is disabled
  if (!blink_interval_ms) return;

  // Blink every interval ms
  if ( board_millis() - start_ms < blink_interval_ms) return; // not enough time
  start_ms += blink_interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
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
    i2c_write_blocking(I2C_PORT, SSD1306_ADDRESS, buf, 2, false);
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

    i2c_write_blocking(I2C_PORT, SSD1306_ADDRESS, ptr, 513, false);
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
