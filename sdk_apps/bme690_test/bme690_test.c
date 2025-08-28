#include "badgevms/device.h"

#include <stdio.h>

#include <unistd.h>

int main(int argc, char *argv[]) {
    sleep(5);

    gas_device_t *gas;

    gas = (gas_device_t *)device_get("GAS0");
    if (gas == NULL) {
        printf("Well, no device found");
        return 0;
    }

    printf("Get BME690...\n");
    printf("Temperature in Celsius(.2f): %.2f \n", gas->_get_temperature(gas));
    printf("Humidity in Rel. Percentage(.2f): %.2f \n", gas->_get_humidity(gas));
    printf("Pressure in Pascal(.2f): %.2f \n", gas->_get_pressure(gas));
    printf("Gas Resistance in Ohm(.2f): %.2f \n", gas->_get_gas_resistance(gas));
}