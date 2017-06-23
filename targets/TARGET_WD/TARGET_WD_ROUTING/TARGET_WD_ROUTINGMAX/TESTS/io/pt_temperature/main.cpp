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

#include "mbed.h"
#include "utest/utest.h"
#include "unity/unity.h"
#include "greentea-client/test_env.h"
#include "wd_logging.h"

#include "io.h"

using namespace utest::v1;

void test_pt_temperature_read_value(void) {
	
	// wait for all measurements to refresh (mux selection + median window)
	wait_ms(8000);
	
	for(int i=0; i<PTCount; i++) {
		wd_log_info("measured value: %.2f", routingmax_io.PTs[i].getValue());
		TEST_ASSERT_FLOAT_WITHIN_MESSAGE(2.0f, 0, routingmax_io.PTs[i].getValue(), "Temperature value was not within expected range!");
	}
	
}

utest::v1::status_t greentea_failure_handler(const Case *const source, const failure_t reason) {
	greentea_case_failure_abort_handler(source, reason);
	return STATUS_CONTINUE;
}

Case cases[] = {
	Case("PT100/PT1000 TEMPERATURE read and verify value", test_pt_temperature_read_value, greentea_failure_handler)
};

utest::v1::status_t greentea_test_setup(const size_t number_of_cases) {
	GREENTEA_SETUP(20, "default_auto");
	return greentea_test_setup_handler(number_of_cases);
}

Specification specification(greentea_test_setup, cases, greentea_test_teardown_handler);

int main() {
	Harness::run(specification);
}
