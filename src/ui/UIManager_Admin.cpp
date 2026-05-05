#include "UIManager.h"
#include <Arduino.h>

static void admin_tab_change_event_cb(lv_event_t * e) {
    lv_obj_t * tv = lv_event_get_current_target(e);
    uint16_t tab_id = lv_tabview_get_tab_act(tv);
    UIManager* ui = (UIManager*)lv_event_get_user_data(e);
    if (ui && ui->getBus()) {
        ui->getBus()->updateAdminPage(tab_id);
        Serial.printf("[UI] Admin Tab Changed to: %d\n", tab_id);
    }
}

void UIManager::buildAdminView(lv_obj_t* parent) {
    // 基础容器设置
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_all(parent, 0, 0);

    // 1. 创建嵌套 TabView
    admin_tv = lv_tabview_create(parent, LV_DIR_TOP, 40);
    lv_obj_set_style_bg_color(admin_tv, lv_color_hex(0x0F172A), 0);
    lv_obj_add_event_cb(admin_tv, admin_tab_change_event_cb, LV_EVENT_VALUE_CHANGED, this);
    
    lv_obj_t* sub_btns = lv_tabview_get_tab_btns(admin_tv);
    lv_obj_set_style_bg_color(sub_btns, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_text_color(sub_btns, lv_color_hex(0x94A3B8), 0);
    lv_obj_set_style_text_font(sub_btns, &ui_font_chs_16, 0);
    lv_obj_set_style_bg_color(sub_btns, lv_color_hex(0x38BDF8), LV_PART_INDICATOR);
    lv_obj_set_style_text_color(sub_btns, lv_color_white(), LV_STATE_CHECKED);

    // 2. 添加功能 Tab
    lv_obj_t* t_encoder = lv_tabview_add_tab(admin_tv, "编码器");
    lv_obj_t* t_laser   = lv_tabview_add_tab(admin_tv, "激光扫描仪");
    lv_obj_t* t_outlets = lv_tabview_add_tab(admin_tv, "下料口");
    lv_obj_t* t_comm    = lv_tabview_add_tab(admin_tv, "通讯端口");

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
    
    this->label_admin_encoder_raw = lv_label_create(enc_cont);
    this->label_admin_encoder_corrected = lv_label_create(enc_cont);
    this->label_admin_encoder_logic = lv_label_create(enc_cont);
    this->label_admin_encoder_zero_count = lv_label_create(enc_cont);
    this->label_admin_encoder_zero_stats = lv_label_create(enc_cont);
    
    lv_obj_t* enc_labels[] = {label_admin_encoder_raw, label_admin_encoder_corrected, label_admin_encoder_logic, label_admin_encoder_zero_count, label_admin_encoder_zero_stats};
    for(auto l : enc_labels) {
        lv_obj_set_style_text_color(l, lv_color_white(), 0);
        lv_obj_set_style_text_font(l, &ui_font_chs_16, 0);
        lv_label_set_text(l, "---");
    }

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
        admin_laser_leds[i] = lv_led_create(wrapper);
        lv_obj_set_size(admin_laser_leds[i], 16, 16);
        lv_led_set_color(admin_laser_leds[i], lv_color_hex(0x10B981));
        lv_led_off(admin_laser_leds[i]);
        lv_obj_t* label = lv_label_create(wrapper);
        lv_label_set_text_fmt(label, "P%d", i+1);
        lv_obj_set_style_text_color(label, lv_color_hex(0x94A3B8), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);
    }

    admin_laser_chart = lv_chart_create(laser_cont);
    lv_obj_set_size(admin_laser_chart, lv_pct(100), lv_pct(70));
    lv_obj_align(admin_laser_chart, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_chart_set_type(admin_laser_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(admin_laser_chart, 200);
    lv_chart_set_range(admin_laser_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 5);
    lv_obj_set_style_bg_color(admin_laser_chart, lv_color_hex(0x020617), 0);
    lv_obj_set_style_border_color(admin_laser_chart, lv_color_hex(0x1E293B), 0);
    lv_color_t colors[] = {lv_color_hex(0x38BDF8), lv_color_hex(0x10B981), lv_color_hex(0xF43F5E), lv_color_hex(0xFBBF24)};
    for(int i=0; i<NUM_SCAN_POINTS; i++) admin_laser_series[i] = lv_chart_add_series(admin_laser_chart, colors[i], LV_CHART_AXIS_PRIMARY_Y);

    // --- 3. Outlets Tab Layout ---
    lv_obj_t* outlet_cont = lv_obj_create(t_outlets);
    lv_obj_set_size(outlet_cont, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(outlet_cont, 0, 0);
    lv_obj_set_style_border_width(outlet_cont, 0, 0);
    lv_obj_set_flex_flow(outlet_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_gap(outlet_cont, 5, 0);

    auto edit_cb = [](lv_event_t* e) {
        UIManager* ui = (UIManager*)lv_event_get_user_data(e);
        lv_obj_t* obj = lv_event_get_target(e);
        long encoded = (long)lv_obj_get_user_data(obj);
        int index = encoded >> 4;
        int action = encoded & 0x0F;
        if (ui && ui->getBus()) ui->getBus()->onOutletEdit(index, action);
    };

    for(int i=0; i<8; i++) {
        lv_obj_t* row = lv_obj_create(outlet_cont);
        lv_obj_set_size(row, lv_pct(100), 50);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_bg_color(row, lv_color_hex(0x1E293B), 0);
        lv_obj_set_style_border_width(row, 0, 0);
        lv_obj_set_style_pad_all(row, 5, 0);

        lv_obj_t* l_idx = lv_label_create(row);
        lv_label_set_text_fmt(l_idx, "#%d", i+1);
        lv_obj_set_style_text_color(l_idx, lv_color_hex(0x38BDF8), 0);

        auto create_adjust = [&](const char* label, lv_obj_t** val_label, int action_m, int action_p) {
            lv_obj_t* btn_m = lv_btn_create(row); lv_obj_set_size(btn_m, 28, 28);
            lv_obj_t* lm = lv_label_create(btn_m); lv_label_set_text(lm, "-"); lv_obj_center(lm);
            lv_obj_set_user_data(btn_m, (void*)(long)((i<<4)|action_m));
            lv_obj_add_event_cb(btn_m, edit_cb, LV_EVENT_CLICKED, this);

            *val_label = lv_label_create(row);
            lv_obj_set_width(*val_label, 35);
            lv_label_set_text(*val_label, "0.0");
            lv_obj_set_style_text_color(*val_label, lv_color_white(), 0);

            lv_obj_t* btn_p = lv_btn_create(row); lv_obj_set_size(btn_p, 28, 28);
            lv_obj_t* lp = lv_label_create(btn_p); lv_label_set_text(lp, "+"); lv_obj_center(lp);
            lv_obj_set_user_data(btn_p, (void*)(long)((i<<4)|action_p));
            lv_obj_add_event_cb(btn_p, edit_cb, LV_EVENT_CLICKED, this);
        };

        create_adjust("Min", &admin_outlet_ui[i].label_min, 0, 1);
        lv_obj_t* div = lv_label_create(row); lv_label_set_text(div, "~");
        create_adjust("Max", &admin_outlet_ui[i].label_max, 2, 3);

        const char* ln[] = {"S", "M", "L"};
        lv_obj_t** lp[] = {&admin_outlet_ui[i].cb_s, &admin_outlet_ui[i].cb_m, &admin_outlet_ui[i].cb_l};
        for(int j=0; j<3; j++) {
            *lp[j] = lv_checkbox_create(row);
            lv_checkbox_set_text(*lp[j], ln[j]);
            lv_obj_set_style_text_color(*lp[j], lv_color_white(), 0);
            lv_obj_set_user_data(*lp[j], (void*)(long)((i<<4)|(4+j)));
            lv_obj_add_event_cb(*lp[j], edit_cb, LV_EVENT_VALUE_CHANGED, this);
        }
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
    
    this->admin_comm_hex_label = lv_label_create(log_cont);
    lv_obj_set_width(admin_comm_hex_label, 300);
    lv_obj_set_style_text_color(admin_comm_hex_label, lv_color_hex(0x38BDF8), 0); 
    lv_obj_set_style_text_font(admin_comm_hex_label, &lv_font_montserrat_12, 0);
    
    lv_obj_t* line = lv_obj_create(log_cont);
    lv_obj_set_size(line, 2, lv_pct(100));
    lv_obj_set_style_bg_color(line, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_border_width(line, 0, 0);

    this->admin_comm_ascii_label = lv_label_create(log_cont);
    lv_obj_set_flex_grow(admin_comm_ascii_label, 1);
    lv_obj_set_style_text_color(admin_comm_ascii_label, lv_color_hex(0x10B981), 0); 
    lv_obj_set_style_text_font(admin_comm_ascii_label, &lv_font_montserrat_12, 0);
}
