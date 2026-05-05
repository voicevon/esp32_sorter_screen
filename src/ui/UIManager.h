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
    lv_obj_t* getAdminTv() const { return admin_tv; }

private:
    void buildDashboardView(lv_obj_t* parent);
    
    // --- Tabs ---
    lv_obj_t* tabview = nullptr;
    lv_obj_t* dashboard_tab = nullptr;
    lv_obj_t* admin_tab = nullptr;
    lv_obj_t* about_tab = nullptr;

    // --- About Section ---
    void buildAboutView(lv_obj_t* parent);

    // --- Components ---
    lv_obj_t* status_label = nullptr;
    
    // --- Dashboard Items ---
    lv_obj_t* comm_led = nullptr;
    lv_obj_t* label_speed_sec = nullptr;
    lv_obj_t* label_yield = nullptr;
    lv_obj_t* label_capacity = nullptr;
    lv_obj_t* label_diameter = nullptr;
    lv_obj_t* label_frame_counter = nullptr;
    
    // --- Admin / Maintenance Section ---
    void buildAdminView(lv_obj_t* parent);
    lv_obj_t* admin_tv = nullptr;       // 维护页面的嵌套 TabView
    lv_obj_t* admin_comm_hex_label = nullptr; 
    lv_obj_t* admin_comm_ascii_label = nullptr;

    ICommandBus* _bus = nullptr;


    // --- Performance Optimization (Dirty Check) ---
    UISnapshot _lastSnapshot;
    bool _isFirstUpdate = true;
};

#endif // UI_MANAGER_H
