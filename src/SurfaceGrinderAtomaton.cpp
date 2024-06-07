#include <stdio.h>
#include "hardware/watchdog.h"
#include "pico/stdlib.h"
#include <string>
#include <string.h>
#include <iostream>
#include "blink.pio.h"

#include "drivers/Motor/Stepper.hpp"

void InitBlinkPIO(PIO pio, uint sm, uint offset, uint pin, uint freq) {
    blink_program_init(pio, sm, offset, pin);
    pio_sm_set_enabled(pio, sm, true);

    printf("Blinking pin %d at %d Hz\n", pin, freq);

    // PIO counter program takes 3 more cycles in total than we pass as
    // input (wait for n + 1; mov; jmp)
    pio->txf[sm] = (125000000 / (2 * freq)) - 3;
}

int main() {
    stdio_init_all();

    sleep_ms(1000); // time for UART to connect for debugging, comment out later
    printf("\n\n--------------\n\n");
    // Watchdog example code
    if (watchdog_caused_reboot()) {
        printf("Rebooted by Watchdog!\n");
        // Whatever action you may take if a watchdog caused a reboot
    }

    // PIO Blinking example
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &blink_program);
    printf("Loaded program at %d\n", offset);

#ifdef PICO_DEFAULT_LED_PIN
    InitBlinkPIO(pio, 0, offset, PICO_DEFAULT_LED_PIN, 3);
#else
    InitBlinkPIO(pio, 0, offset, 6, 3);
#endif

    // Enable the watchdog, requiring the watchdog to be updated every 100ms or the chip will reboot
    // second arg is pause on debug which means the watchdog will pause when stepping through code
    // watchdog_enable(12000, 1);

    // You need to call this function at least more often than the 100ms in the enable call to prevent a reboot
    watchdog_update();

    Stepper stepper1(10, 9, 200.0f, 50.0f, pio, 1); // Step pin is 10, direction pin is 9

    stepper1.InitPIO();

    while (true) {
        printf("Moving stepper left\n");
        watchdog_update();
        stepper1.SetDirection(true);
        stepper1.Move(1000);
        sleep_ms(1000);

        printf("Moving stepper right\n");
        watchdog_update();
        stepper1.SetDirection(false);
        stepper1.Move(1000);
        sleep_ms(1000);
    }
}