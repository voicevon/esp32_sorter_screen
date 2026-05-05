#include "UIManager.h"
#include <Arduino.h>

static void diag_tab_change_event_cb(lv_event_t * e) {
    lv_obj_t * tv = lv_event_get_current_target(e);
    uint16_t tab_id = lv_tabview_get_tab_act(tv);
    UIManager* ui = (UIManager*)lv_event_get_user_data(e);
    if (ui && ui->getBus()) {
        ui->getBus()->updateAdminPage(tab_id);
        Serial.printf("[UI] Diag Sub-Tab Changed to: %d\n", tab_id);
    }
}

void UIManager::buildDiagView(lv_obj_t* parent) {
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_all(parent, 0, 0);

    // 1. 创建诊断嵌套 TabView (取代原 diag_tv)
    diag_tv = lv_tabview_create(parent, LV_DIR_TOP, 40);
    lv_obj_set_style_bg_color(diag_tv, lv_color_hex(0x0F172A), 0);
    lv_obj_add_event_cb(diag_tv, diag_tab_change_event_cb, LV_EVENT_VALUE_CHANGED, this);
    
    lv_obj_t* sub_btns = lv_tabview_get_tab_btns(diag_tv);
    lv_obj_set_style_bg_color(sub_btns, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_text_color(sub_btns, lv_color_hex(0x94A3B8), 0);
    lv_obj_set_style_text_font(sub_btns, &ui_font_chs_16, 0);
    lv_obj_set_style_bg_color(sub_btns, lv_color_hex(0x38BDF8), LV_PART_INDICATOR);
    lv_obj_set_style_text_color(sub_btns, lv_color_white(), LV_STATE_CHECKED);

    // 2. 添加功能 Tab
    lv_obj_t* t_encoder = lv_tabview_add_tab(diag_tv, "编码器");
    lv_obj_t* t_laser   = lv_tabview_add_tab(diag_tv, "激光扫描仪");
    lv_obj_t* t_outlets = lv_tabview_add_tab(diag_tv, "下料口");
    lv_obj_t* t_comm    = lv_tabview_add_tab(diag_tv, "通讯端口");

    lv_obj_t* sub_tabs[] = {t_encoder, t_laser, t_outlets, t_comm};
    for(auto t : sub_tabs) {
        lv_obj_set_style_pad_all(t, 10, 0);
        lv_obj_set_scrollbar_mode(t, LV_SCROLLBAR_MODE_AUTO); 
    }
    
    // --- 1. Encoder Tab Layout ---
    lv_obj_t* enc_cont = lv_obj_create(t_encoder);
    lv_obj_set_size(enc_cont, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(enc_cont, 0, 0);
    lv_obj_set_style_border_width(enc_cont, 0, 0);
    lv_obj_set_flex_flow(enc_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_gap(enc_cont, 10, 0);
    
    diag_encoder_ui.label_raw = lv_label_create(enc_cont);
    diag_encoder_ui.label_corrected = lv_label_create(enc_cont);
    diag_encoder_ui.label_logic = lv_label_create(enc_cont);
    diag_encoder_ui.label_zero = lv_label_create(enc_cont);
    diag_encoder_ui.label_status = lv_label_create(enc_cont);
    
    lv_obj_t* enc_labels[] = {
        diag_encoder_ui.label_raw, 
        diag_encoder_ui.label_corrected, 
        diag_encoder_ui.label_logic, 
        diag_encoder_ui.label_zero, 
        diag_encoder_ui.label_status
    };
    for(auto l : enc_labels) {
        lv_obj_set_style_text_color(l, lv_color_white(), 0);
        lv_obj_set_style_text_font(l, &ui_font_chs_16, 0);
        lv_label_set_text(l, "---");
    }
    
    // --- Offset Adjustment Row ---
    lv_obj_t* offset_cont = lv_obj_create(enc_cont);
    lv_obj_set_size(offset_cont, lv_pct(100), 50);
    lv_obj_set_style_bg_opa(offset_cont, 0, 0);
    lv_obj_set_style_border_width(offset_cont, 0, 0);
    lv_obj_set_flex_flow(offset_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(offset_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(offset_cont, 0, 0);

    lv_obj_t* l_off_title = lv_label_create(offset_cont);
    lv_label_set_text(l_off_title, "零位偏移 (Offset): ");
    lv_obj_set_style_text_color(l_off_title, lv_color_hex(0x94A3B8), 0);
    lv_obj_set_style_text_font(l_off_title, &ui_font_chs_16, 0);

    auto offset_cb = [](lv_event_t* e) {
        UIManager* ui = (UIManager*)lv_event_get_user_data(e);
        lv_obj_t* btn = lv_event_get_target(e);
        int delta = (int)(long)lv_obj_get_user_data(btn);
        
        if (ui && ui->getBus()) {
            int current = ui->getSnapshot().diag_encoder_offset;
            int target = current + delta;
            if (target < 0) target = 0;
            if (target > 199) target = 199;
            ui->getBus()->pushEvent("set_offset", 0, target);
        }
    };

    lv_obj_t* btn_minus_10 = lv_btn_create(offset_cont);
    lv_obj_set_size(btn_minus_10, 45, 35);
    lv_obj_set_style_bg_color(btn_minus_10, lv_color_hex(0x1E293B), 0);
    lv_obj_t* l_minus_10 = lv_label_create(btn_minus_10);
    lv_label_set_text(l_minus_10, "-10");
    lv_obj_center(l_minus_10);
    lv_obj_set_user_data(btn_minus_10, (void*)-10);
    lv_obj_add_event_cb(btn_minus_10, offset_cb, LV_EVENT_CLICKED, this);

    lv_obj_t* btn_minus = lv_btn_create(offset_cont);
    lv_obj_set_size(btn_minus, 40, 35);
    lv_obj_set_style_bg_color(btn_minus, lv_color_hex(0x334155), 0);
    lv_obj_t* l_minus = lv_label_create(btn_minus);
    lv_label_set_text(l_minus, LV_SYMBOL_MINUS);
    lv_obj_center(l_minus);
    lv_obj_set_user_data(btn_minus, (void*)-1);
    lv_obj_add_event_cb(btn_minus, offset_cb, LV_EVENT_CLICKED, this);

    diag_encoder_ui.label_offset = lv_label_create(offset_cont);
    lv_obj_set_style_text_color(diag_encoder_ui.label_offset, lv_color_hex(0x38BDF8), 0);
    lv_obj_set_style_text_font(diag_encoder_ui.label_offset, &lv_font_montserrat_26, 0);
    lv_obj_set_width(diag_encoder_ui.label_offset, 60);
    lv_obj_set_style_text_align(diag_encoder_ui.label_offset, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(diag_encoder_ui.label_offset, "0");

    lv_obj_t* btn_plus = lv_btn_create(offset_cont);
    lv_obj_set_size(btn_plus, 40, 35);
    lv_obj_set_style_bg_color(btn_plus, lv_color_hex(0x334155), 0);
    lv_obj_t* l_plus = lv_label_create(btn_plus);
    lv_label_set_text(l_plus, LV_SYMBOL_PLUS);
    lv_obj_center(l_plus);
    lv_obj_set_user_data(btn_plus, (void*)1);
    lv_obj_add_event_cb(btn_plus, offset_cb, LV_EVENT_CLICKED, this);

    lv_obj_t* btn_plus_10 = lv_btn_create(offset_cont);
    lv_obj_set_size(btn_plus_10, 45, 35);
    lv_obj_set_style_bg_color(btn_plus_10, lv_color_hex(0x1E293B), 0);
    lv_obj_t* l_plus_10 = lv_label_create(btn_plus_10);
    lv_label_set_text(l_plus_10, "+10");
    lv_obj_center(l_plus_10);
    lv_obj_set_user_data(btn_plus_10, (void*)10);
    lv_obj_add_event_cb(btn_plus_10, offset_cb, LV_EVENT_CLICKED, this);

    // --- 2. Laser Tab Layout ---
    lv_obj_t* laser_cont = lv_obj_create(t_laser);
    lv_obj_set_size(laser_cont, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(laser_cont, 0, 0);
    lv_obj_set_style_border_width(laser_cont, 0, 0);
    
    lv_obj_t* indicator_cont = lv_obj_create(laser_cont);
    lv_obj_set_size(indicator_cont, lv_pct(100), 60);
    lv_obj_set_flex_flow(indicator_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(indicator_cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(indicator_cont, 0, 0);
    lv_obj_set_style_border_width(indicator_cont, 0, 0);
    lv_obj_set_style_pad_gap(indicator_cont, 15, 0);

    for(int i=0; i<NUM_SCAN_POINTS; i++) {
        lv_obj_t* wrapper = lv_obj_create(indicator_cont);
        lv_obj_set_size(wrapper, 50, 50);
        lv_obj_set_style_bg_opa(wrapper, 0, 0);
        lv_obj_set_style_border_width(wrapper, 0, 0);
        lv_obj_set_flex_flow(wrapper, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(wrapper, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        
        diag_laser_ui.leds[i] = lv_led_create(wrapper);
        lv_obj_set_size(diag_laser_ui.leds[i], 16, 16);
        lv_led_set_color(diag_laser_ui.leds[i], lv_color_hex(0x10B981));
        lv_led_off(diag_laser_ui.leds[i]);
        
        lv_obj_t* label = lv_label_create(wrapper);
        lv_label_set_text_fmt(label, "P%d", i+1);
        lv_obj_set_style_text_color(label, lv_color_hex(0x94A3B8), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);
    }

    diag_laser_ui.chart = lv_chart_create(laser_cont);
    lv_obj_set_size(diag_laser_ui.chart, lv_pct(100), lv_pct(70));
    lv_obj_align(diag_laser_ui.chart, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_chart_set_type(diag_laser_ui.chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(diag_laser_ui.chart, 200);
    lv_chart_set_range(diag_laser_ui.chart, LV_CHART_AXIS_PRIMARY_Y, 0, 5);
    lv_obj_set_style_bg_color(diag_laser_ui.chart, lv_color_hex(0x020617), 0);
    lv_obj_set_style_border_color(diag_laser_ui.chart, lv_color_hex(0x1E293B), 0);
    lv_color_t colors[] = {lv_color_hex(0x38BDF8), lv_color_hex(0x10B981), lv_color_hex(0xF43F5E), lv_color_hex(0xFBBF24)};
    for(int i=0; i<NUM_SCAN_POINTS; i++) {
        diag_laser_ui.series[i] = lv_chart_add_series(diag_laser_ui.chart, colors[i], LV_CHART_AXIS_PRIMARY_Y);
    }

    // --- 3. Outlets Diag Layout ---
    lv_obj_t* outlet_cont = lv_obj_create(t_outlets);
    lv_obj_set_size(outlet_cont, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(outlet_cont, 0, 0);
    lv_obj_set_style_border_width(outlet_cont, 0, 0);
    lv_obj_set_flex_flow(outlet_cont, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(outlet_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_gap(outlet_cont, 15, 0);

    auto diag_cb = [](lv_event_t* e) {
        UIManager* ui = (UIManager*)lv_event_get_user_data(e);
        lv_obj_t* obj = lv_event_get_target(e);
        int idx = (int)(long)lv_obj_get_user_data(obj);
        
        if (ui && ui->getBus()) {
            bool currentState = ui->getSnapshot().outlets[idx].state;
            ui->getBus()->onOutletDiag(idx, !currentState);
            Serial.printf("[UI] Diag Outlet #%d Toggle: %s\n", idx + 1, !currentState ? "OPEN" : "CLOSE");
        }
    };

    for(int i=0; i<8; i++) {
        lv_obj_t* item = lv_obj_create(outlet_cont);
        lv_obj_set_size(item, 85, 100);
        lv_obj_set_style_bg_color(item, lv_color_hex(0x1E293B), 0);
        lv_obj_set_style_border_width(item, 1, 0);
        lv_obj_set_style_border_color(item, lv_color_hex(0x334155), 0);
        lv_obj_set_flex_flow(item, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(item, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_all(item, 5, 0);

        lv_obj_t* l_idx = lv_label_create(item);
        lv_label_set_text_fmt(l_idx, "OUT %d", i+1);
        lv_obj_set_style_text_color(l_idx, lv_color_white(), 0);
        lv_obj_set_style_text_font(l_idx, &lv_font_montserrat_12, 0);

        diag_outlet_leds[i] = lv_led_create(item);
        lv_obj_set_size(diag_outlet_leds[i], 16, 16);
        lv_led_set_color(diag_outlet_leds[i], lv_color_hex(0x38BDF8));
        lv_led_off(diag_outlet_leds[i]);

        lv_obj_t* btn = lv_btn_create(item);
        lv_obj_set_size(btn, 60, 30);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x334155), 0);
        lv_obj_t* l_btn = lv_label_create(btn);
        lv_label_set_text(l_btn, "TEST");
        lv_obj_center(l_btn);
        lv_obj_set_style_text_font(l_btn, &lv_font_montserrat_12, 0);
        
        lv_obj_set_user_data(btn, (void*)(long)i);
        lv_obj_add_event_cb(btn, diag_cb, LV_EVENT_CLICKED, this);
    }

    // --- 4. Comm Port Tab Layout ---
    lv_obj_t* comm_info = lv_label_create(t_comm);
    lv_obj_set_style_text_color(comm_info, lv_color_hex(0x94A3B8), 0);
    lv_obj_set_style_text_font(comm_info, &ui_font_chs_16, 0);
    lv_label_set_text_fmt(comm_info, "串口: Serial1 (RX:%d TX:%d) | 波特率: %d", PIN_RS485_RX, PIN_RS485_TX, RS485_BAUD);
    
    lv_obj_t* log_cont = lv_obj_create(t_comm);
    lv_obj_set_size(log_cont, lv_pct(100), lv_pct(80));
    lv_obj_align(log_cont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(log_cont, lv_color_hex(0x020617), 0);
    lv_obj_set_style_border_width(log_cont, 1, 0);
    lv_obj_set_style_border_color(log_cont, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_pad_all(log_cont, 2, 0);
    lv_obj_set_scrollbar_mode(log_cont, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_flex_flow(log_cont, LV_FLEX_FLOW_ROW);
    
    diag_comm_hex_label = lv_label_create(log_cont);
    lv_obj_set_width(diag_comm_hex_label, 300);
    lv_obj_set_style_text_color(diag_comm_hex_label, lv_color_hex(0x38BDF8), 0); 
    lv_obj_set_style_text_font(diag_comm_hex_label, &lv_font_montserrat_12, 0);
    
    lv_obj_t* line = lv_obj_create(log_cont);
    lv_obj_set_size(line, 2, lv_pct(100));
    lv_obj_set_style_bg_color(line, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_border_width(line, 0, 0);

    diag_comm_ascii_label = lv_label_create(log_cont);
    lv_obj_set_flex_grow(diag_comm_ascii_label, 1);
    lv_obj_set_style_text_color(diag_comm_ascii_label, lv_color_hex(0x10B981), 0); 
    lv_obj_set_style_text_font(diag_comm_ascii_label, &lv_font_montserrat_12, 0);
}
