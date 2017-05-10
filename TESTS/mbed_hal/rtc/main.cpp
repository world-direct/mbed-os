/* mbed Microcontroller Library
 * Copyright (c) 2016 ARM Limited
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

#if !DEVICE_RTC
    #error [NOT_SUPPORTED] RTC not supported for this target
#endif

#include "utest/utest.h"
#include "unity/unity.h"
#include "greentea-client/test_env.h"

#include "mbed.h"
#include "rtc_api.h"

using namespace utest::v1;

#define TOLERANCE (1)

void rtc_set_and_verify_time(void) {
	time_t t = 1256729759; // Set RTC time to Wednesday, 28-Oct-09 11:35:59 UTC
	int delay_s = 5;
	
	rtc_write(t);
	
	// wait and read current value
	wait_ms(delay_s * 1000);

	time_t cur = rtc_read();
	
	TEST_ASSERT_UINT64_WITHIN(TOLERANCE, t + delay_s, cur);
}

void rtc_is_running(void) {
	time_t t = 1256729759; // Set RTC time to Wednesday, 28-Oct-09 11:35:59 UTC
	int delay_s = 2;
	
	rtc_write(t);
	
	// wait and read current value
	wait_ms(delay_s * 1000);

	time_t cur = rtc_read();
	
	TEST_ASSERT_TRUE(cur > t);
}

void rtc_is_enabled_after_init(void) {
	bool enabled = rtc_isenabled();
	TEST_ASSERT_TRUE(enabled);
}

utest::v1::status_t greentea_failure_handler(const Case *const source, const failure_t reason) {
    greentea_case_failure_abort_handler(source, reason);
    return STATUS_CONTINUE;
}

Case cases[] = {
	Case("RTC is enabled after init", rtc_is_enabled_after_init, greentea_failure_handler),
	Case("RTC is running", rtc_is_running, greentea_failure_handler),
	Case("RTC set and verify time", rtc_set_and_verify_time, greentea_failure_handler)
};

utest::v1::status_t greentea_test_setup(const size_t number_of_cases) {
    GREENTEA_SETUP(20, "default_auto");
	rtc_init();
    return greentea_test_setup_handler(number_of_cases);
}

Specification specification(greentea_test_setup, cases, greentea_test_teardown_handler);

int main() {
    Harness::run(specification);
}
