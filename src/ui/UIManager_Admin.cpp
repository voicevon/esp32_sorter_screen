#include "UIManager.h"
#include <Arduino.h>

static void admin_tab_change_event_cb(lv_event_t * e) {
    // 占位
}

static void btn_scan_event_cb(lv_event_t * e) {
    UIManager* ui = (UIManager*)lv_event_get_user_data(e);
    if (ui && ui->getBus()) ui->getBus()->cmdStartScan();
}

static void diag_switch_event_cb(lv_event_t * e) {
    // 仅保留业务指令，不再触发行销模式切换
    lv_obj_t * obj = lv_event_get_target(e);
    bool active = lv_obj_has_state(obj, LV_STATE_CHECKED);
    UIManager* ui = (UIManager*)lv_event_get_user_data(e);
    if (ui && ui->getBus()) ui->getBus()->cmdToggleDiagnosis(active);
}

static void belt_diag_switch_event_cb(lv_event_t * e) {
    // 仅用于 UI 状态控制或内部标志，不再干预全局运行模式
}

static void belt_scan_event_cb(lv_event_t * e) {
    UIManager* ui = (UIManager*)lv_event_get_user_data(e);
    if (ui && ui->getBus()) {
        ui->getBus()->cmdTriggerBeltScan();
    }
}

static void serial_send_preset_cb(lv_event_t * e) {
    UIManager* ui = (UIManager*)lv_event_get_user_data(e);
    lv_obj_t* btn = lv_event_get_target(e);
    lv_obj_t* lbl = lv_obj_get_child(btn, 0);
    const char* preset = (const char*)lv_obj_get_user_data(btn);
    if (ui && ui->getBus() && preset) {
        ui->getBus()->cmdSerialSendHex(preset);
    }
}

static void serial_auto_switch_cb(lv_event_t * e) {
    UIManager* ui = (UIManager*)lv_event_get_user_data(e);
    lv_obj_t* obj = lv_event_get_target(e);
    bool active = lv_obj_has_state(obj, LV_STATE_CHECKED);
    if (ui && ui->getBus()) ui->getBus()->cmdSerialToggleAuto(active);
}

static void servo_test_event_cb(lv_event_t * e) {
    // 占位: 芦笋分拣机暂无舵机测试
}

static void btn_global_open_cb(lv_event_t * e) {
    UIManager* ui = (UIManager*)lv_event_get_user_data(e);
    if (ui && ui->getBus()) ui->getBus()->cmdGlobalServo(true);
}

static void btn_global_close_cb(lv_event_t * e) {
    UIManager* ui = (UIManager*)lv_event_get_user_data(e);
    if (ui && ui->getBus()) ui->getBus()->cmdGlobalServo(false);
}

static void btn_belt1_test_cb(lv_event_t * e) {
    UIManager* ui = (UIManager*)lv_event_get_user_data(e);
    lv_obj_t * btn = lv_event_get_target(e);
    lv_obj_t * label = lv_obj_get_child(btn, 0);
    const char* text = lv_label_get_text(label);
    int dist = atoi(text);
    if (dist > 0 && ui && ui->getBus()) {
        ui->getBus()->cmdBeltTest(0, dist); // 使用逻辑索引 0 (一级皮带)
    }
}

static void btn_belt2_test_cb(lv_event_t * e) {
    UIManager* ui = (UIManager*)lv_event_get_user_data(e);
    lv_obj_t * btn = lv_event_get_target(e);
    lv_obj_t * label = lv_obj_get_child(btn, 0);
    const char* text = lv_label_get_text(label);
    int dist = atoi(text);
    if (dist > 0 && ui && ui->getBus()) {
        ui->getBus()->cmdBeltTest(1, dist); // 使用逻辑索引 1 (二级皮带)
    }
}

static void btn_belt2_start_cb(lv_event_t * e) {
    UIManager* ui = (UIManager*)lv_event_get_user_data(e);
    if (ui && ui->getBus()) {
        ui->getBus()->cmdBeltRun(1, true);
    }
}

static void btn_belt2_stop_cb(lv_event_t * e) {
    UIManager* ui = (UIManager*)lv_event_get_user_data(e);
    if (ui && ui->getBus()) {
        ui->getBus()->cmdBeltRun(1, false);
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

    // 2. 添加三个功能 Tab
    lv_obj_t* t_encoder = lv_tabview_add_tab(admin_tv, "编码器");
    lv_obj_t* t_laser = lv_tabview_add_tab(admin_tv, "激光扫描仪");
    lv_obj_t* t_cutter = lv_tabview_add_tab(admin_tv, "旋转切割刀");

    // 统一设置各 Tab 样式
    lv_obj_t* sub_tabs[] = {t_encoder, t_laser, t_cutter};
    for(auto t : sub_tabs) {
        lv_obj_set_style_pad_all(t, 15, 0);
        lv_obj_set_scrollbar_mode(t, LV_SCROLLBAR_MODE_OFF); 
    }
}
