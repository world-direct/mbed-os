#include "QUECTEL_EC2X_CellularNetwork.h"
#include "CellularLog.h"

using namespace mbed;

QUECTEL_EC2X_CellularNetwork::QUECTEL_EC2X_CellularNetwork(ATHandler &atHandler, AT_CellularDevice &device) : AT_CellularNetwork(atHandler, device)
{
}

QUECTEL_EC2X_CellularNetwork::~QUECTEL_EC2X_CellularNetwork()
{
}

nsapi_error_t QUECTEL_EC2X_CellularNetwork::set_access_technology_scan_sequence(RadioAccessTechnology *ops, size_t const size)
{
    //nwscanseq
    // 00 -> Automatic (LTE/WCDMA/TD-SCDMA/GSM)
    // 01 GSM
    // 02 TD-SCDMA (UMTS)
    // 03 WCDMA (UMTS)
    // 04 LTE
    // 05 CDMA (UMTS)
    if (ops == nullptr || size == 0) {
        _at.lock();
        _at.at_cmd_discard("+QCFG", "=\"nwscanseq\",00");
        return _at.unlock_return_error();
    }
    size_t extraLength = 0;
    for (size_t i = 0; i < size; i++) {
        switch (ops[i]) {
            case RAT_GSM:
            case RAT_GSM_COMPACT:
            case RAT_EGPRS:
                extraLength += 2;
                break;

            case RAT_HSDPA:
            case RAT_UTRAN:
            case RAT_HSUPA:
            case RAT_HSDPA_HSUPA:
                extraLength += 4;
                break;

            case RAT_E_UTRAN:
                extraLength += 2;
                break;

            default:
                return NSAPI_ERROR_UNSUPPORTED;
        }
    }


    char *str = new char[13 + extraLength + 1];
    strcpy(str, "=\"nwscanseq\",");
    char *sequenceStr = str + 13;
    for (size_t i = 0; i < size; i++) {

        switch (ops[i]) {
            case RAT_GSM:
            case RAT_GSM_COMPACT:
            case RAT_EGPRS:
                sequenceStr[0] = '0';
                sequenceStr[1] = '1';
                sequenceStr = sequenceStr + 2;
                break;

            case RAT_HSDPA:
            case RAT_UTRAN:
            case RAT_HSUPA:
            case RAT_HSDPA_HSUPA:
                sequenceStr[0] = '0';
                sequenceStr[1] = '3';
                sequenceStr[2] = '0';
                sequenceStr[3] = '2';
                sequenceStr = sequenceStr + 4;
                break;

            case RAT_E_UTRAN:
                sequenceStr[0] = '0';
                sequenceStr[1] = '4';
                sequenceStr = sequenceStr + 2;
                break;
            default: // do nothing as this is filtered before
                break;
        }

    }
    sequenceStr[0] = 0;
    _at.lock();
    _at.at_cmd_discard("+QCFG", str);
    delete[] str;
    return _at.unlock_return_error();
}

nsapi_error_t QUECTEL_EC2X_CellularNetwork::set_access_technology_impl(RadioAccessTechnology opsAct)
{

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
            // UMTS
            _at.at_cmd_discard("+QCFG", "=\"nwscanmode\",5,1");
            break;
        case RAT_E_UTRAN:
            // LTE
            _at.at_cmd_discard("+QCFG", "=\"nwscanmode\",3,1");
            break;

        case RAT_UNKNOWN:
            _at.at_cmd_discard("+QCFG", "=\"nwscanmode\",0,1");

            _at.unlock();

            break;
        default:
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

