#ifndef QUECTEL_EC2X_CELLULAR_NETWORK_H_
#define QUECTEL_EC2X_CELLULAR_NETWORK_H_

#include "AT_CellularNetwork.h"

namespace mbed {

class QUECTEL_EC2X_CellularNetwork : public AT_CellularNetwork {
public:
    QUECTEL_EC2X_CellularNetwork(ATHandler &atHandler, AT_CellularDevice &device);
    virtual ~QUECTEL_EC2X_CellularNetwork();

    nsapi_error_t set_access_technology_scan_sequence(RadioAccessTechnology *ops, size_t const size);

protected:
    virtual nsapi_error_t set_access_technology_impl(RadioAccessTechnology opRat);
    virtual void get_context_state_command();

};

} // namespace mbed

#endif
