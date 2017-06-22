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

#include "utest/utest.h"
#include "unity/unity.h"
#include "greentea-client/test_env.h"

#include "mbed.h"

using namespace utest::v1;

void utils_collection_add(void) {
	TEST_ASSERT_TRUE(0 == 0);
}

void utils_collection_remove(void) {
	TEST_ASSERT_TRUE(0 == 1);
}

utest::v1::status_t greentea_failure_handler(const Case *const source, const failure_t reason) {
	greentea_case_failure_abort_handler(source, reason);
	return STATUS_CONTINUE;
}

Case cases[] = {
	Case("Collection", utils_collection_add, utils_collection_remove)
};

utest::v1::status_t greentea_test_setup(const size_t number_of_cases) {
	GREENTEA_SETUP(20, "default_auto");
	return greentea_test_setup_handler(number_of_cases);
}

Specification specification(greentea_test_setup, cases, greentea_test_teardown_handler);

int main() {
	Harness::run(specification);
}
