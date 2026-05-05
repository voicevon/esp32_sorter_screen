#ifndef I_COMMAND_BUS_H
#define I_COMMAND_BUS_H

#include <Arduino.h>
#include "system/SystemTypes.h"

class ICommandBus {
public:
    virtual ~ICommandBus() = default;

    virtual void cmdToggleDiagnosis(bool active) = 0;
    virtual void updateOperationMode(OperationMode mode) = 0;
    virtual void updateAdminPage(uint8_t pageId) = 0;
    virtual void onOutletEdit(int index, int action) = 0;
    virtual void onOutletDiag(int index, bool state) = 0;
};

#endif
