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
    // 基础容器设置：禁用原有垂直布局与滚动，由嵌套 TabView 接管
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_all(parent, 0, 0);

    // 1. 创建嵌套 TabView (内部二级导航)
    admin_tv = lv_tabview_create(parent, LV_DIR_TOP, 40);
    lv_obj_set_style_bg_color(admin_tv, lv_color_hex(0x0F172A), 0);
    lv_obj_add_event_cb(admin_tv, admin_tab_change_event_cb, LV_EVENT_VALUE_CHANGED, this);
    
    lv_obj_t* sub_btns = lv_tabview_get_tab_btns(admin_tv);
    lv_obj_set_style_bg_color(sub_btns, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_text_color(sub_btns, lv_color_hex(0x94A3B8), 0);
    lv_obj_set_style_text_font(sub_btns, &ui_font_chs_16, 0);
    lv_obj_set_style_bg_color(sub_btns, lv_color_hex(0x38BDF8), LV_PART_INDICATOR);
    lv_obj_set_style_text_color(sub_btns, lv_color_white(), LV_STATE_CHECKED);

    // 2. 添加功能 Tab (目前主要展示通讯监控)
    lv_obj_t* t_encoder = lv_tabview_add_tab(admin_tv, "编码器");
    lv_obj_t* t_laser   = lv_tabview_add_tab(admin_tv, "激光扫描仪");
    lv_obj_t* t_cutter  = lv_tabview_add_tab(admin_tv, "旋转切割刀");
    lv_obj_t* t_comm    = lv_tabview_add_tab(admin_tv, "通讯端口");

    // 统一设置各 Tab 样式
    lv_obj_t* sub_tabs[] = {t_encoder, t_laser, t_cutter, t_comm};
    for(auto t : sub_tabs) {
        lv_obj_set_style_pad_all(t, 15, 0);
        lv_obj_set_scrollbar_mode(t, LV_SCROLLBAR_MODE_OFF); 
    }

    // --- 通讯端口监控页面布局 ---
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
    lv_obj_set_flex_align(log_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    // HEX 列 (左侧)
    lv_obj_t* hex_label = lv_label_create(log_cont);
    lv_obj_set_width(hex_label, 300);
    lv_obj_set_style_text_color(hex_label, lv_color_hex(0x38BDF8), 0); 
    lv_obj_set_style_text_font(hex_label, &lv_font_montserrat_12, 0);
    lv_label_set_text(hex_label, "HEX RAW");
    
    lv_obj_t* line = lv_obj_create(log_cont);
    lv_obj_set_size(line, 2, lv_pct(100));
    lv_obj_set_style_bg_color(line, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_border_width(line, 0, 0);

    // ASCII 列 (右侧)
    lv_obj_t* ascii_label = lv_label_create(log_cont);
    lv_obj_set_flex_grow(ascii_label, 1);
    lv_obj_set_style_text_color(ascii_label, lv_color_hex(0x10B981), 0); 
    lv_obj_set_style_text_font(ascii_label, &lv_font_montserrat_12, 0);
    lv_label_set_text(ascii_label, "ASCII STRING");
    
    this->admin_comm_hex_label = hex_label;
    this->admin_comm_ascii_label = ascii_label;
}
