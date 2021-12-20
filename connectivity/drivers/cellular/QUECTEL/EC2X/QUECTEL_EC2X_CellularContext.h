#ifndef QUECTEL_EC2X_CELLULARCONTEXT_H_
#define QUECTEL_EC2X_CELLULARCONTEXT_H_
#include <string>
#include "AT_CellularContext.h"
#include "QUECTEL_EC2X_CellularNetwork.h"

namespace mbed {

class QUECTEL_EC2X_CellularContext: public mbed::AT_CellularContext {
public:
    QUECTEL_EC2X_CellularContext(mbed::ATHandler &at, mbed::CellularDevice *device, const char *apn, bool cp_req = false, bool nonip_req = false);
    QUECTEL_EC2X_CellularContext &operator=(const QUECTEL_EC2X_CellularContext &) = delete;
    QUECTEL_EC2X_CellularContext &operator=(const QUECTEL_EC2X_CellularContext &&) = delete;
    QUECTEL_EC2X_CellularContext &operator=(QUECTEL_EC2X_CellularContext &&) = delete;
    QUECTEL_EC2X_CellularContext(const QUECTEL_EC2X_CellularContext &) = delete;
    QUECTEL_EC2X_CellularContext(const QUECTEL_EC2X_CellularContext &&) = delete;
    QUECTEL_EC2X_CellularContext(QUECTEL_EC2X_CellularContext &&) = delete;
    virtual ~QUECTEL_EC2X_CellularContext();

    CellularNetwork::RadioAccessTechnology getAccessTechnology() const;

    QUECTEL_EC2X_CellularNetwork *getCellularNetwork() const;
    char const *getApn() const;
protected:
    virtual void do_connect() override;


private:
    QUECTEL_EC2X_CellularNetwork *_network = nullptr;
    CellularNetwork::RadioAccessTechnology _rat;
};

} // namespace mbed

#endif
