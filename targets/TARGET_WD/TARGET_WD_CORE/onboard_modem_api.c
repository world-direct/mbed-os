/* mbed Microcontroller Library
 * Copyright (c) 2017 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#if MBED_CONF_NSAPI_PRESENT

#include "cellular/onboard_modem_api.h"
#include "gpio_api.h"
#include "platform/mbed_wait_api.h"
#include "PinNames.h"

#if MODEM_ON_BOARD

void onboard_modem_power_up()
{
    
    gpio_t gsm_pwr_pin;
    gpio_t gsm_vdd_ext_pin;

    gpio_init_out(&gsm_pwr_pin,     GSM_ON_OFF);
    gpio_init_in(&gsm_vdd_ext_pin,  GSM_PWRMON);

    if (gpio_read(&gsm_vdd_ext_pin)) {
		gpio_write(&gsm_pwr_pin, 1);
        wait_ms(850);
        gpio_write(&gsm_pwr_pin, 0);
		do {
			wait_ms(100);	
		} while (gpio_read(&gsm_vdd_ext_pin));
	}

    gpio_write(&gsm_pwr_pin, 1);
	do {
		wait_ms(100);	
	} while (gpio_read(&gsm_vdd_ext_pin) == 0);
	wait_ms(1500);
	gpio_write(&gsm_pwr_pin, 0);
    wait_ms(100);

}

void onboard_modem_power_down()
{
    
}

void onboard_modem_init()
{

}

void onboard_modem_deinit()
{

}

#endif //MODEM_ON_BOARD
#endif //MBED_CONF_NSAPI_PRESENT
