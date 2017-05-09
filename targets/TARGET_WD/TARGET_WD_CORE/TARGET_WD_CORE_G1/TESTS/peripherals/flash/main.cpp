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

#include "W25X40CL.h"

using namespace utest::v1;

void test_flash_read_id(PinName cs) {
	
	flash_init(cs, SPI_MOSI, SPI_MISO, SPI_SCK);
	
	uint32_t jedec_id;
	int result = flash_read_jedec_id(&jedec_id);
	
	// test if read was successful
	TEST_ASSERT_TRUE(result == 0);
	
	// validate id -> Winbond manufacturer id = 0xEF
	TEST_ASSERT_TRUE(((uint8_t)(jedec_id >> 16)) == 0xEF);
}

void test_flash_1_read_id(void) {
	test_flash_read_id(SPI_CS3);
}

void test_flash_2_read_id(void) {
	test_flash_read_id(SPI_CS4);
}

utest::v1::status_t greentea_failure_handler(const Case *const source, const failure_t reason) {
	greentea_case_failure_abort_handler(source, reason);
	return STATUS_CONTINUE;
}

Case cases[] = {
	Case("FLASH 1 read JEDEC ID", test_flash_1_read_id, greentea_failure_handler)
};

utest::v1::status_t greentea_test_setup(const size_t number_of_cases) {
	GREENTEA_SETUP(20, "default_auto");
	return greentea_test_setup_handler(number_of_cases);
}

Specification specification(greentea_test_setup, cases, greentea_test_teardown_handler);

int main() {
	Harness::run(specification);
}
