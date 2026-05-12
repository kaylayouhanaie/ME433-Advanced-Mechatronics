#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include <string.h>

#define UART_ID uart1
#define BAUD_RATE 115200
#define UART_TX_PIN 4 
#define UART_RX_PIN 5  

int main() {
    stdio_init_all(); 

    // Set up UART0 for STM32 communication
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);


    while (true) {

	    char c;
        char buffer[100];
        char m[100];

        int index = 0;

        int ch = getchar_timeout_us(0);

        if (ch != PICO_ERROR_TIMEOUT) {

            sprintf(m, "%c\n", ch);
            // printf("here\n");
            uart_puts(UART_ID,m);
            
        }
        // STM32 -> Pico USB
        if (uart_is_readable(UART_ID)) {

            char c = uart_getc(UART_ID);

            printf("Received: %c\r\n", c);
        }
    }
    }









