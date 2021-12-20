/*
 * Copyright (c) 2019, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
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

#include "QUECTEL_EC2X.h"

#include "PinNames.h"
#include "AT_CellularNetwork.h"
#include "rtos/ThisThread.h"
#include "drivers/BufferedSerial.h"
#include "drivers/DigitalIn.h"
#include "QUECTEL_EC2X_CellularNetwork.h"
#include "QUECTEL_EC2X_CellularContext.h"
#define TRACE_GROUP "EC2X"
#include "mbed-trace/mbed_trace.h"

using namespace std::chrono;
using namespace mbed;
using namespace rtos;
using namespace events;

#if !defined(MBED_CONF_QUECTEL_EC2X_PWR)
#define MBED_CONF_QUECTEL_EC2X_PWR    NC
#endif

#if !defined(MBED_CONF_QUECTEL_EC2X_RST)
#define MBED_CONF_QUECTEL_EC2X_RST    NC
#endif

#if !defined(MBED_CONF_QUECTEL_EC2X_TX)
#define MBED_CONF_QUECTEL_EC2X_TX    NC
#endif

#if !defined(MBED_CONF_QUECTEL_EC2X_RX)
#define MBED_CONF_QUECTEL_EC2X_RX    NC
#endif

#if !defined(MBED_CONF_QUECTEL_EC2X_POLARITY)
#define MBED_CONF_QUECTEL_EC2X_POLARITY    1 // active high
#endif

#if !defined(MBED_CONF_QUECTEL_EC2X_START_TIMEOUT)
#define MBED_CONF_QUECTEL_EC2X_START_TIMEOUT    15000
#endif


static const intptr_t cellular_properties[AT_CellularDevice::PROPERTY_MAX] = {
    AT_CellularNetwork::RegistrationModeLAC,    // C_EREG
    AT_CellularNetwork::RegistrationModeLAC,    // C_GREG
    AT_CellularNetwork::RegistrationModeLAC,    // C_REG
    0,  // AT_CGSN_WITH_TYPE
    1,  // AT_CGDATA
    0,  // AT_CGAUTH
    1,  // AT_CNMI
    1,  // AT_CSMP
    1,  // AT_CMGF
    1,  // AT_CSDH
    1,  // PROPERTY_IPV4_STACK
    1,  // PROPERTY_IPV6_STACK
    1,  // PROPERTY_IPV4V6_STACK
    0,  // PROPERTY_NON_IP_PDP_TYPE
    1,  // PROPERTY_AT_CGEREP,
    1,  // PROPERTY_AT_COPS_FALLBACK_AUTO
    0,  // PROPERTY_SOCKET_COUNT
    0,  // PROPERTY_IP_TCP
    0,  // PROPERTY_IP_UDP
    0,  // PROPERTY_AT_SEND_DELAY
};

QUECTEL_EC2X::QUECTEL_EC2X(FileHandle *fh, PinName pwr, bool polarity_pwr, PinName rst, bool polarity_rst)
    : AT_CellularDevice(fh),
      _active_high_pwr(polarity_pwr),
      _active_high_rst(polarity_rst),
      _pwr_key(pwr, !polarity_pwr),
      _rst(rst, polarity_rst)
{
    set_cellular_properties(cellular_properties);
}

#if MBED_CONF_QUECTEL_EC2X_PROVIDE_DEFAULT
CellularDevice *CellularDevice::get_default_instance()
{
    static BufferedSerial serial(MBED_CONF_QUECTEL_EC2X_TX,
                                 MBED_CONF_QUECTEL_EC2X_RX,
                                 MBED_CONF_QUECTEL_EC2X_BAUDRATE);
#if defined(MBED_CONF_QUECTEL_EC2X_RTS) && defined(MBED_CONF_QUECTEL_EC2X_CTS)
    serial.set_flow_control(SerialBase::RTSCTS, MBED_CONF_QUECTEL_EC2X_RTS, MBED_CONF_QUECTEL_EC2X_CTS);
#endif
    static QUECTEL_EC2X device(&serial,
                               MBED_CONF_QUECTEL_EC2X_PWR,
                               MBED_CONF_QUECTEL_EC2X_POLARITY_PWR,
                               MBED_CONF_QUECTEL_EC2X_RST,
                               MBED_CONF_QUECTEL_EC2X_POLARITY_RST);
    return &device;
}
#endif

nsapi_error_t QUECTEL_EC2X::init()
{
    auto ret = AT_CellularDevice::init();
    if (ret == NSAPI_ERROR_OK) {
        auto network = open_network();
        if (network != nullptr) {
            //ret = network->set_access_technology(mbed::CellularNetwork::RadioAccessTechnology::RAT_GSM);
            ret = network->set_access_technology(mbed::CellularNetwork::RadioAccessTechnology::RAT_UTRAN);
        }
    }
    return ret;
}

nsapi_error_t QUECTEL_EC2X::press_power_button(duration<uint32_t, std::milli> timeout)
{
    if (_pwr_key.is_connected()) {
        _pwr_key = _active_high_pwr;
        ThisThread::sleep_for(timeout);
        _pwr_key = !_active_high_pwr;
        ThisThread::sleep_for(100ms);
    }




    return NSAPI_ERROR_OK;
}

nsapi_error_t QUECTEL_EC2X::hard_power_on()
{
    mbed::DigitalIn _state(LTE_STATUS);
    if (_state.read() == 0) {
        tr_warning("Modem already on");
        return NSAPI_ERROR_OK;
    } else {
        tr_info("turning modem on.");
    }
    press_power_button(600ms);

    tr_info("wait for state getting low");
    auto begin = rtos::Kernel::Clock::now();
    bool currentState = _state.read() == 1;
    while (currentState && rtos::Kernel::Clock::now() - begin < std::chrono::seconds(10)) {
        currentState = _state.read() == 1;
        tr_info("state is %s", currentState ? "true" : "false");
        rtos::ThisThread::sleep_for(500ms);
    }
    tr_info("state is %s", currentState ? "true" : "false");
    //tr_info("waiting 10 sec for power on");
    //rtos::ThisThread::sleep_for(std::chrono::seconds(10));
    tr_info("powered on");
    return NSAPI_ERROR_OK;
}

nsapi_error_t QUECTEL_EC2X::hard_power_off()
{
    mbed::DigitalIn _state(LTE_STATUS);
    if (_state.read() == 1) {
        tr_warning("Modem already off");
        return NSAPI_ERROR_OK;
    } else {
        tr_info("turning modem off.");
    }
    press_power_button(750ms);

    tr_info("wait for state getting high");
    auto begin = rtos::Kernel::Clock::now();
    bool currentState = _state.read() == 1;
    while (!currentState && rtos::Kernel::Clock::now() - begin < std::chrono::seconds(65)) {
        currentState = _state.read() == 1;
        tr_info("state is %s", currentState ? "true" : "false");
        rtos::ThisThread::sleep_for(500ms);
    }
    tr_info("state is %s", currentState ? "true" : "false");
    tr_info("powered down");
    /*tr_info("waiting for POWERED DOWN");
    _at.lock();

    _at.set_at_timeout(milliseconds(65000));
    _at.resp_start();
    _at.set_stop_tag("POWERED DOWN");
    bool rdy = _at.consume_to_stop_tag();

    _at.unlock();
    if (!rdy) {
        tr_warn("did not receive power down");
        return NSAPI_ERROR_DEVICE_ERROR;
    }
    tr_info("powered down");*/
    return NSAPI_ERROR_OK;
}

nsapi_error_t QUECTEL_EC2X::soft_power_on()
{
    tr_info("soft_power_on");
    if (_rst.is_connected()) {
        _rst = !_active_high_rst;
        ThisThread::sleep_for(460ms);
        _rst = _active_high_rst;
        ThisThread::sleep_for(100ms);

        _at.lock();

        _at.set_at_timeout(milliseconds(MBED_CONF_QUECTEL_EC2X_START_TIMEOUT));
        _at.resp_start();
        _at.set_stop_tag("RDY");
        bool rdy = _at.consume_to_stop_tag();
        _at.set_stop_tag(OK);

        _at.unlock();

        if (!rdy) {
            return NSAPI_ERROR_DEVICE_ERROR;
        }
    }

    return NSAPI_ERROR_OK;
}

nsapi_error_t QUECTEL_EC2X::soft_power_off()
{
    tr_info("soft power off");
    return hard_power_off();
}


AT_CellularNetwork *QUECTEL_EC2X::open_network_impl(ATHandler &at)
{
    return new QUECTEL_EC2X_CellularNetwork(at, *this);
}

AT_CellularContext *QUECTEL_EC2X::create_context_impl(ATHandler &at, const char *apn, bool cp_req, bool nonip_req)
{
    return new QUECTEL_EC2X_CellularContext(at, this, apn, cp_req, nonip_req);
}