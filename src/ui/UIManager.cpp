#include "UIManager.h"
#include <Arduino.h>
#include <stdio.h>

UIManager::UIManager() {
    tabview = nullptr;
    dashboard_tab = nullptr;
    admin_tab = nullptr;
    status_label = nullptr;
    _bus = nullptr;
}


static void tab_change_event_cb(lv_event_t * e) {
    lv_obj_t * tv = lv_event_get_target(e);
    uint16_t tab_id = lv_tabview_get_tab_act(tv);
    UIManager* ui = (UIManager*)lv_event_get_user_data(e);
    
    if (ui && ui->getBus()) {
        if (tab_id == 0) {
            ui->getBus()->updateOperationMode(MODE_PRODUCTION);
        } else if (tab_id == 1) {
            // 系统维护 Tab
            ui->getBus()->updateOperationMode(MODE_CONFIGURATION);
        } else {
            ui->getBus()->updateOperationMode(MODE_ABOUT);
        }
    }
}






















void UIManager::init() {
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x0F172A), 0);

    tabview = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 0);
    lv_obj_add_event_cb(tabview, tab_change_event_cb, LV_EVENT_VALUE_CHANGED, this);
    
    lv_obj_set_style_bg_color(tabview, lv_color_hex(0x0F172A), 0);
    lv_obj_t* tab_btns = lv_tabview_get_tab_btns(tabview);
    lv_obj_set_style_bg_color(tab_btns, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_text_color(tab_btns, lv_color_white(), 0);
    lv_obj_set_style_text_font(tab_btns, &ui_font_chs_16, 0);
    lv_obj_set_style_bg_color(tab_btns, lv_color_hex(0x38BDF8), LV_PART_INDICATOR);

    lv_obj_set_style_border_width(tabview, 0, 0);
    lv_obj_set_size(tabview, 800, 480);

    dashboard_tab = lv_tabview_add_tab(tabview, "生产主界面");
    admin_tab = lv_tabview_add_tab(tabview, "系统维护");
    about_tab = lv_tabview_add_tab(tabview, "关于我们");

    lv_obj_set_style_pad_all(dashboard_tab, 0, 0);
    lv_obj_set_style_border_width(dashboard_tab, 0, 0);
    lv_obj_set_scrollbar_mode(dashboard_tab, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scrollbar_mode(admin_tab, LV_SCROLLBAR_MODE_OFF); // 调整为 OFF，由嵌套接管
    lv_obj_set_scrollbar_mode(about_tab, LV_SCROLLBAR_MODE_OFF);

    buildDashboardView(dashboard_tab);
    buildAdminView(admin_tab);
    buildAboutView(about_tab);

    Serial.println("[UI] Tri-tab UI initialized with Dashboard.");
}

void UIManager::updateDashboard(const SystemContext* ctx) {
    if (!ctx) return;
    
    // 2. Data from Master
    if (_isFirstUpdate || (ctx->ui.dirtyFlags & DF_LIVE_DATA)) {
        float speed_sec = ctx->ui.dashboard_speed;
        
        if (label_speed_sec) {
            lv_label_set_text_fmt(label_speed_sec, "速度: %.1f 根/秒", speed_sec);
        }
        if (label_diameter) {
            lv_label_set_text_fmt(label_diameter, "直径: %.1f mm", ctx->ui.dashboard_diameter);
        }
        if (label_yield) {
            lv_label_set_text_fmt(label_yield, "产量: %.0f 根", ctx->ui.dashboard_yield);
        }
        if (label_capacity) {
            lv_label_set_text_fmt(label_capacity, "产能: %.0f 根/小时", ctx->ui.dashboard_capacity);
        }
        if (label_frame_counter) {
            lv_label_set_text_fmt(label_frame_counter, "FC: %u", ctx->ui.frame_counter);
        }
    }
    
    // 3. Comm Watchdog LED
    if (comm_led) {
        bool isOk = (millis() - ctx->ui.last_comm_time) <= COMM_WATCHDOG_TIMEOUT_MS;
        lv_obj_set_style_bg_color(comm_led, isOk ? lv_color_hex(0x22C55E) : lv_color_hex(0xEF4444), 0);
    }
    
    // 4. Comm Log Monitor (Dual Columns)
    if (admin_comm_hex_label && admin_comm_ascii_label && ctx->ui.curMode == MODE_CONFIGURATION) {
        String fullHex = "";
        String fullAscii = "";
        
        if (ctx->ui.admin_comm_log_count == 0) {
            fullHex = "Listen...";
            fullAscii = "Wait Data...";
        } else {
            for (int i = 0; i < ctx->ui.admin_comm_log_count; i++) {
                fullHex += String(ctx->ui.admin_comm_log_hex[i]) + "\n";
                fullAscii += String(ctx->ui.admin_comm_log_ascii[i]) + "\n";
            }
        }
        lv_label_set_text(admin_comm_hex_label, fullHex.c_str());
        lv_label_set_text(admin_comm_ascii_label, fullAscii.c_str());
    }

    _lastSnapshot = ctx->ui;
    _isFirstUpdate = false;
}

// =============================================================================
// About Section & 80s Apple II Rain Animation
// =============================================================================


