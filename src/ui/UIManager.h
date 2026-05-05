#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <lvgl.h>
#include <stdint.h>
#include <vector>
#include "system/SystemContext.h"
#include "drivers/PinDefinition.h"
#include "ui/I_Command_Bus.h"

// Font declarations
LV_FONT_DECLARE(ui_font_chs_16);
extern const lv_font_t lv_font_montserrat_48;
extern const lv_font_t lv_font_montserrat_16;
extern const lv_font_t lv_font_montserrat_26;
extern const lv_font_t lv_font_montserrat_12;

class UIManager {
public:
    UIManager();
    void init();
    void updateDashboard(const SystemContext* ctx);
    void setCommandBus(ICommandBus* bus) { _bus = bus; }
    ICommandBus* getBus() const { return _bus; }
    lv_obj_t* getDiagTv() const { return diag_tv; }
    const UISnapshot& getSnapshot() const { return _lastSnapshot; }

private:
    void buildDashboardView(lv_obj_t* parent);
    void buildConfigView(lv_obj_t* parent);
    void buildDiagView(lv_obj_t* parent);
    void buildAboutView(lv_obj_t* parent);
    
    // --- Tabs ---
    lv_obj_t* tabview = nullptr;
    lv_obj_t* dashboard_tab = nullptr;
    lv_obj_t* config_tab = nullptr;
    lv_obj_t* diag_tab = nullptr; // Top level Diagnostic/Maintenance tab
    lv_obj_t* about_tab = nullptr;

    // --- Components ---
    lv_obj_t* status_label = nullptr;
    
    // --- Dashboard Items ---
    lv_obj_t* comm_led = nullptr;
    lv_obj_t* label_speed_sec = nullptr;
    lv_obj_t* label_yield = nullptr;
    lv_obj_t* label_capacity = nullptr;
    lv_obj_t* label_diameter = nullptr;
    lv_obj_t* label_frame_counter = nullptr;
    
    // --- Nested Diagnostic View (Inside Admin Tab) ---
    lv_obj_t* diag_tv = nullptr;       
    
    // --- Diagnostic Sub-pages UI Cache ---
    // 1. Encoder Diag
    struct {
        lv_obj_t* label_raw;
        lv_obj_t* label_corrected;
        lv_obj_t* label_logic;
        lv_obj_t* label_zero;
        lv_obj_t* label_status;
    } diag_encoder_ui;

    // 2. Laser Diag
    struct {
        lv_obj_t* leds[4];
        lv_obj_t* chart; // Using chart for history
        lv_chart_series_t* series[4];
    } diag_laser_ui;

    // 3. Outlet Diag
    lv_obj_t* diag_outlet_leds[8];

    // 4. Communication Log Diag
    lv_obj_t* diag_comm_hex_label;
    lv_obj_t* diag_comm_ascii_label;

    // Outlet Config UI Cache
    struct OutletUI {
        lv_obj_t* label_min;
        lv_obj_t* label_max;
        lv_obj_t* cb_s;
        lv_obj_t* cb_m;
        lv_obj_t* cb_l;
    } config_outlet_ui[8];

    ICommandBus* _bus = nullptr;

    // --- Performance Optimization (Dirty Check) ---
    UISnapshot _lastSnapshot;
    bool _isFirstUpdate = true;
};

#endif // UI_MANAGER_H
