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

    // 1. 创建诊断嵌套 TabView
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

    // 3. 模块化构建各个子页面
    if (_diagEncoderView) _diagEncoderView->build(t_encoder, _bus);
    if (_diagScannerView) _diagScannerView->build(t_laser, _bus);
    if (_diagOutletView)  _diagOutletView->build(t_outlets, _bus);
    if (_diagCommView)    _diagCommView->build(t_comm, _bus);
}
