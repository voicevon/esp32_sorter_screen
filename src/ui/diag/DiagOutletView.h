#ifndef DIAG_OUTLET_VIEW_H
#define DIAG_OUTLET_VIEW_H

#include <lvgl.h>
#include "system/SystemContext.h"
#include "ui/I_Command_Bus.h"

class DiagOutletView {
public:
    void build(lv_obj_t* parent, ICommandBus* bus);
    void update(const SystemContext* ctx);
private:
    lv_obj_t* leds[8];
    bool _lastOutletStates[8] = {false};
    ICommandBus* _bus = nullptr;
};

#endif // DIAG_OUTLET_VIEW_H
