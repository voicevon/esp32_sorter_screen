#ifndef DIAG_COMM_VIEW_H
#define DIAG_COMM_VIEW_H

#include <lvgl.h>
#include "system/SystemContext.h"
#include "ui/I_Command_Bus.h"

class DiagCommView {
public:
    void build(lv_obj_t* parent, ICommandBus* bus);
    void update(const SystemContext* ctx);
private:
    lv_obj_t* hex_label = nullptr;
    lv_obj_t* ascii_label = nullptr;
    ICommandBus* _bus = nullptr;
};

#endif // DIAG_COMM_VIEW_H
