#ifndef DIAG_ENCODER_VIEW_H
#define DIAG_ENCODER_VIEW_H

#include <lvgl.h>
#include "system/SystemContext.h"
#include "ui/I_Command_Bus.h"

class DiagEncoderView {
public:
    void build(lv_obj_t* parent, ICommandBus* bus);
    void update(const SystemContext* ctx);
private:
    lv_obj_t* label_raw = nullptr;
    lv_obj_t* label_corrected = nullptr;
    lv_obj_t* label_logic = nullptr;
    lv_obj_t* label_zero = nullptr;
    lv_obj_t* label_status = nullptr;
    lv_obj_t* label_offset = nullptr;
    int _lastOffset = 0;
    ICommandBus* _bus = nullptr;
};

#endif // DIAG_ENCODER_VIEW_H
