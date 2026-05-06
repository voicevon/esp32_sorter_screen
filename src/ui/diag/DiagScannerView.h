#ifndef DIAG_SCANNER_VIEW_H
#define DIAG_SCANNER_VIEW_H

#include <lvgl.h>
#include "system/SystemContext.h"
#include "ui/I_Command_Bus.h"

class DiagScannerView {
public:
    void build(lv_obj_t* parent, ICommandBus* bus);
    void update(const SystemContext* ctx);
private:
    lv_obj_t* leds[NUM_SCAN_POINTS];
    lv_obj_t* chart = nullptr;
    lv_chart_series_t* series[NUM_SCAN_POINTS];
    lv_chart_cursor_t* cursor = nullptr;
    lv_obj_t* label_sample_count = nullptr;
    ICommandBus* _bus = nullptr;
};

#endif // DIAG_SCANNER_VIEW_H
