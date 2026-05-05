#include "UIManager.h"
#include <Arduino.h>
#include <stdio.h>

UIManager::UIManager() {
    tabview = nullptr;
    dashboard_tab = nullptr;
    config_tab = nullptr;
    diag_tab = nullptr;
    about_tab = nullptr;
    status_label = nullptr;
    _bus = nullptr;
}


static void tab_change_event_cb(lv_event_t * e) {
    lv_obj_t * tv = lv_event_get_current_target(e);
    uint16_t tab_id = lv_tabview_get_tab_act(tv);
    UIManager* ui = (UIManager*)lv_event_get_user_data(e);
    
    if (ui && ui->getBus()) {
        Serial.printf("[UI] Tab Change Event! Target Tab: %d\n", tab_id);
        if (tab_id == 0) {
            ui->getBus()->updateOperationMode(MODE_PRODUCTION);
        } else if (tab_id == 1) {
            ui->getBus()->updateOperationMode(MODE_OUTLET_CONFIG);
        } else if (tab_id == 2) {
            ui->getBus()->updateOperationMode(MODE_DIAGNOSTICS);
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
    config_tab = lv_tabview_add_tab(tabview, "参数配置");
    diag_tab = lv_tabview_add_tab(tabview, "系统维护");
    about_tab = lv_tabview_add_tab(tabview, "关于我们");

    lv_obj_set_style_pad_all(dashboard_tab, 0, 0);
    lv_obj_set_style_border_width(dashboard_tab, 0, 0);
    lv_obj_set_scrollbar_mode(dashboard_tab, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scrollbar_mode(config_tab, LV_SCROLLBAR_MODE_OFF); 
    lv_obj_set_scrollbar_mode(diag_tab, LV_SCROLLBAR_MODE_OFF); 
    lv_obj_set_scrollbar_mode(about_tab, LV_SCROLLBAR_MODE_OFF);

    buildDashboardView(dashboard_tab);
    buildConfigView(config_tab);
    buildDiagView(diag_tab);
    buildAboutView(about_tab);

    Serial.println("[UI] Multi-tab UI initialized with Diag support.");
}

void UIManager::updateDashboard(const SystemContext* ctx) {
    if (!ctx) return;
    
    // 2. Data from Master
    if (_isFirstUpdate || (ctx->ui.dirtyFlags & DF_LIVE_DATA)) {
        if (label_speed_sec) {
            lv_label_set_text_fmt(label_speed_sec, "速度: %.1f 根/秒", ctx->ui.dashboard_speed);
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
    if (diag_comm_hex_label && diag_comm_ascii_label && ctx->ui.curMode == MODE_DIAGNOSTICS) {
        String fullHex = "";
        String fullAscii = "";
        
        if (ctx->ui.diag_comm_log_count == 0) {
            fullHex = "Listen...";
            fullAscii = "Wait Data...";
        } else {
            for (int i = 0; i < ctx->ui.diag_comm_log_count; i++) {
                fullHex += String(ctx->ui.diag_comm_log_hex[i]) + "\n";
                fullAscii += String(ctx->ui.diag_comm_log_ascii[i]) + "\n";
            }
        }
        lv_label_set_text(diag_comm_hex_label, fullHex.c_str());
        lv_label_set_text(diag_comm_ascii_label, fullAscii.c_str());
    }
    
    // 5. Diag Encoder Data
    if (ctx->ui.curMode == MODE_DIAGNOSTICS && ctx->ui.diag_page_id == 0) {
        if (diag_encoder_ui.label_raw) {
            lv_label_set_text_fmt(diag_encoder_ui.label_raw, "原始值 (Raw): %d", ctx->ui.diag_encoder_raw);
        }
        if (diag_encoder_ui.label_corrected) {
            lv_label_set_text_fmt(diag_encoder_ui.label_corrected, "修正值 (Corrected): %d", ctx->ui.diag_encoder_corrected);
        }
        if (diag_encoder_ui.label_logic) {
            lv_label_set_text_fmt(diag_encoder_ui.label_logic, "逻辑值 (Logic): %d", ctx->ui.diag_encoder_logic);
        }
        if (diag_encoder_ui.label_zero) {
            lv_label_set_text_fmt(diag_encoder_ui.label_zero, "经历 Z 信号次数: %d", ctx->ui.diag_encoder_zero_count);
        }
        if (diag_encoder_ui.label_status) {
            lv_label_set_text_fmt(diag_encoder_ui.label_status, "Zero 统计 (正确/总计): %d / %d", 
                                  ctx->ui.diag_encoder_zero_correct, ctx->ui.diag_encoder_zero_total);
        }
    }

    // 6. Diag Laser Data
    if (ctx->ui.curMode == MODE_DIAGNOSTICS && ctx->ui.diag_page_id == 1) {
        for (int i = 0; i < NUM_SCAN_POINTS; i++) {
            if (diag_laser_ui.leds[i]) {
                bool isOn = (ctx->ui.diag_laser_states >> i) & 0x01;
                if (isOn) {
                    lv_led_set_color(diag_laser_ui.leds[i], lv_color_hex(0xF43F5E)); // 红: 遮挡
                } else {
                    lv_led_set_color(diag_laser_ui.leds[i], lv_color_hex(0x10B981)); // 绿: 畅通
                }
                lv_led_on(diag_laser_ui.leds[i]);
            }
        }

        if (diag_laser_ui.chart) {
            for (int i = 0; i < NUM_SCAN_POINTS; i++) {
                if (diag_laser_ui.series[i]) {
                    for (int j = 0; j < 200; j++) {
                        int byteIdx = j / 8;
                        int bitIdx = 7 - (j % 8); 
                        bool val = (ctx->ui.diag_laser_history[i][byteIdx] >> bitIdx) & 0x01;
                        lv_chart_set_value_by_id(diag_laser_ui.chart, diag_laser_ui.series[i], j, i + (val ? 0.8 : 0));
                    }
                }
            }
            lv_chart_refresh(diag_laser_ui.chart);
        }
    }

    // 7. Outlet Config (Standalone Config Mode)
    if (ctx->ui.curMode == MODE_OUTLET_CONFIG) {
        for (int i = 0; i < 8; i++) {
            if (config_outlet_ui[i].label_min) {
                lv_label_set_text_fmt(config_outlet_ui[i].label_min, "%.1f", ctx->ui.outlets[i].minDiameter);
            }
            if (config_outlet_ui[i].label_max) {
                lv_label_set_text_fmt(config_outlet_ui[i].label_max, "%.1f", ctx->ui.outlets[i].maxDiameter);
            }
            
            auto update_cb = [](lv_obj_t* cb, bool target) {
                if (!cb) return;
                bool current = lv_obj_has_state(cb, LV_STATE_CHECKED);
                if (current != target) {
                    if (target) lv_obj_add_state(cb, LV_STATE_CHECKED);
                    else lv_obj_clear_state(cb, LV_STATE_CHECKED);
                }
            };
            
            update_cb(config_outlet_ui[i].cb_s, (ctx->ui.outlets[i].lengthMask & 0x01));
            update_cb(config_outlet_ui[i].cb_m, (ctx->ui.outlets[i].lengthMask & 0x02));
            update_cb(config_outlet_ui[i].cb_l, (ctx->ui.outlets[i].lengthMask & 0x04));
        }
    }

    // 8. Diag Outlet Data
    if (ctx->ui.curMode == MODE_DIAGNOSTICS && ctx->ui.diag_page_id == 2) {
        for (int i = 0; i < 8; i++) {
            if (diag_outlet_leds[i]) {
                if (ctx->ui.outlets[i].state) {
                    lv_led_on(diag_outlet_leds[i]);
                    lv_led_set_color(diag_outlet_leds[i], lv_color_hex(0x38BDF8)); 
                } else {
                    lv_led_off(diag_outlet_leds[i]);
                }
            }
        }
    }

    _lastSnapshot = ctx->ui;
    _isFirstUpdate = false;
}
