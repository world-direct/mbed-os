#include "QUECTEL_EC2X_CellularContext.h"

namespace mbed {

QUECTEL_EC2X_CellularContext::QUECTEL_EC2X_CellularContext(mbed::ATHandler &at, mbed::CellularDevice *device, const char *apn, bool cp_req, bool nonip_req):
    AT_CellularContext(at, device, apn, cp_req, nonip_req),
    _network((QUECTEL_EC2X_CellularNetwork *)device->open_network()),
    _rat(mbed::CellularNetwork::RadioAccessTechnology::RAT_UNKNOWN)
{

}

QUECTEL_EC2X_CellularContext::~QUECTEL_EC2X_CellularContext()
{

}

CellularNetwork::RadioAccessTechnology QUECTEL_EC2X_CellularContext::getAccessTechnology() const
{
    return _rat;
}

char const *QUECTEL_EC2X_CellularContext::getApn() const
{
    return _apn;
}

QUECTEL_EC2X_CellularNetwork *QUECTEL_EC2X_CellularContext::getCellularNetwork() const
{
    return _network;
}

void QUECTEL_EC2X_CellularContext::do_connect()
{
    CellularNetwork::registration_params_t params;
    _network->get_registration_params(CellularNetwork::C_GREG, params);
    _rat = params._act;
}

} // namespace mbed
