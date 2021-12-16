#include "QUECTEL_EC2X_CellularNetwork.h"
#include "CellularLog.h"

using namespace mbed;

QUECTEL_EC2X_CellularNetwork::QUECTEL_EC2X_CellularNetwork(ATHandler &atHandler, AT_CellularDevice &device) : AT_CellularNetwork(atHandler, device)
{
}

QUECTEL_EC2X_CellularNetwork::~QUECTEL_EC2X_CellularNetwork()
{
}

nsapi_error_t QUECTEL_EC2X_CellularNetwork::set_access_technology_impl(RadioAccessTechnology opsAct)
{
    // TODO: networkscan sequence depending on access technology
    _at.lock();

    switch (opsAct) {
        case RAT_GSM:
        case RAT_GSM_COMPACT:
        case RAT_EGPRS:
            // GSM
            _at.at_cmd_discard("+QCFG", "=\"nwscanmode\",1,1");
            break;
        case RAT_HSDPA:
        case RAT_UTRAN:
        case RAT_HSUPA:
        case RAT_HSDPA_HSUPA:
            // UMTS //maybe change 5 to 2
            _at.at_cmd_discard("+QCFG", "=\"nwscanmode\",5,1");
            break;
        case RAT_E_UTRAN:
            // LTE
            _at.at_cmd_discard("+QCFG", "=\"nwscanmode\",3,1");
            break;
        default:
            //_at.at_cmd_discard("+QCFG", "=\"nwscanseq\",020301");
            _at.at_cmd_discard("+QCFG", "=\"nwscanmode\",0,1");

            _at.unlock();
            _op_act = RAT_UNKNOWN;
            return NSAPI_ERROR_UNSUPPORTED;
    }

    return _at.unlock_return_error();
}

void QUECTEL_EC2X_CellularNetwork::get_context_state_command()
{
    // read active contexts
    _at.cmd_start_stop("+QIACT", "?");
    _at.resp_start("+QIACT:");
}

