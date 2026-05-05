#ifndef I_COMMAND_BUS_H
#define I_COMMAND_BUS_H

#include "system/SystemTypes.h"

class ICommandBus {
public:
    virtual ~ICommandBus() = default;

    virtual void cmdToggleDiagnosis(bool active) = 0;
    virtual void updateOperationMode(OperationMode mode) = 0;
    virtual void updateAdminPage(uint8_t pageId) = 0;
    virtual void onOutletEdit(uint8_t index, uint8_t action) = 0;
};

#endif // I_COMMAND_BUS_H
