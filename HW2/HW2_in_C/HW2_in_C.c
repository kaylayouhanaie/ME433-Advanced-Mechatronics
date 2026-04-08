#include <stdio.h> // set pico_enable_stdio_usb to 1 in CMakeLists.txt 
#include "pico/stdlib.h" // CMakeLists.txt must have pico_stdlib in target_link_libraries
#include "hardware/pwm.h" // CMakeLists.txt must have hardware_pwm in target_link_libraries
#include "hardware/adc.h" // CMakeLists.txt must have hardware_adc in target_link_libraries

#define PWMPIN 16


void set_position(float angle){
    uint16_t desired_pwm = (uint16_t)(500 + (angle/180) * 2000);
    pwm_set_gpio_level(PWMPIN, desired_pwm);

}

bool timer_interrupt_function(__unused struct repeating_timer *t) {
    // read the adc
    uint16_t result1 = adc_read();
    // print the voltage
    printf("%f\r\n",(float)result1/4095*3.3);
    return true;
}

int main()
{
    stdio_init_all();

    // turn on a timer interrupt
    struct repeating_timer timer;
    // -100 means call the function every 100ms
    // +100 would mean call the function 100ms after the function has ended
    add_repeating_timer_ms(-100, timer_interrupt_function, NULL, &timer);

    // turn on the pwm, in this example to 10kHz with a resolution of 1500
    gpio_set_function(PWMPIN, GPIO_FUNC_PWM); // Set the Pin to be PWM
    uint slice_num = pwm_gpio_to_slice_num(PWMPIN); // Get PWM slice number
    // the clock frequency is 150MHz divided by a float from 1 to 255
    float div = 125; // must be between 1-255
    pwm_set_clkdiv(slice_num, div); // sets the clock speed
    uint16_t wrap = 20000; // when to rollover, must be less than 65535
    pwm_set_wrap(slice_num, wrap); 
    pwm_set_enabled(slice_num, true); // turn on the PWM

    //pwm_set_gpio_level(PWMPIN, 1500/2); // set the duty cycle to 50%

    // turn on the adc
    adc_init();
    adc_gpio_init(26); // pin GP26 is pin ADC0
    adc_select_input(0); // sample from ADC0

    while (true) {
        for (int i = 0; i<180; i++){
            set_position(i);
            sleep_ms(20);
        }
        for (int i = 180; i>0; i--){
            set_position(i);
            sleep_ms(20);
        }
    }
}



// I2C Notes April 7th Class
// 1k or 10k pull-up resistors on SDA and SCL wires
// pins idle high (3.3V)
// multiple SCL and SDA shared in parallel between multiple sensors
// 9th bit after first 8 address is acknowledge coming from sensor, 0 is good
// to sent 10 bytes, send 12 bytes packets, which is 12x9 clock cycles

// TO WRITE: ADDR is hard coded address based on sensor data sheet
// TO READ: first write ending in true, then read

// MCP23008 Chip:
// we have PDIP package
// "must be biased externally" means you can't leave it unconnected
// address byte for this chip: 0100XXXW, where X depends on what address you set it as,
// and R is R or W depending on if you sent read or write function
// there are internal pull up resistors, but you have to turn them on with a register
// port register reads if pins are high or low
// output latch register determines if an output pin should be high or low
// must put pull up resistors on SDA and SCL
// RESET pin: pulse this pin to reset this chip, but igonre it for now unless your code freezes.
// if you're not using RESET pin, tigh high

// initialization function: initialize I2C pins in Pico code, write to IODIR to set 
// turn pin on off function
// read function