#include "UIManager.h"
#include <Arduino.h>
#include <stdio.h>

static void btn_target_base_plus_cb(lv_event_t * e) {
    UIManager* ui = (UIManager*)lv_event_get_user_data(e);
    if (ui && ui->getBus()) ui->getBus()->cmdUpdateTargetBase(10.0f);
}

static void btn_target_base_minus_cb(lv_event_t * e) {
    UIManager* ui = (UIManager*)lv_event_get_user_data(e);
    if (ui && ui->getBus()) ui->getBus()->cmdUpdateTargetBase(-10.0f);
}

static void btn_target_offset_plus_cb(lv_event_t * e) {
    UIManager* ui = (UIManager*)lv_event_get_user_data(e);
    if (ui && ui->getBus()) ui->getBus()->cmdUpdateTargetOffset(1.0f);
}

static void btn_target_offset_minus_cb(lv_event_t * e) {
    UIManager* ui = (UIManager*)lv_event_get_user_data(e);
    if (ui && ui->getBus()) ui->getBus()->cmdUpdateTargetOffset(-1.0f);
}

static void target_sheet_bg_cb(lv_event_t * e) {
    UIManager* ui = (UIManager*)lv_event_get_user_data(e);
    if (ui) ui->closeTargetBottomSheet();
}

void UIManager::showTargetBottomSheet() {
    if (target_sheet) return;

    // 1. 创建全屏半透明背景 (用于点击外部自动关闭)
    target_sheet_bg = lv_obj_create(lv_scr_act());
    lv_obj_set_size(target_sheet_bg, 800, 480);
    lv_obj_set_style_bg_color(target_sheet_bg, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(target_sheet_bg, 160, 0);
    lv_obj_set_style_border_width(target_sheet_bg, 0, 0);
    lv_obj_add_event_cb(target_sheet_bg, target_sheet_bg_cb, LV_EVENT_CLICKED, this);

    // 2. [Plan A] 高精度高亮：已移除 target_label 逻辑

    // 3. 创建侧边调整面板 (1/5 宽度，增加高度以容纳 4 个按钮)
    target_sheet = lv_obj_create(lv_scr_act());
    lv_obj_set_size(target_sheet, 160, 320); // 从 180 增加到 320
    lv_obj_align(target_sheet, LV_ALIGN_TOP_RIGHT, 0, 65);
    lv_obj_set_style_bg_color(target_sheet, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_border_width(target_sheet, 1, 0);
    lv_obj_set_style_border_color(target_sheet, lv_color_hex(0x38BDF8), 0);
    lv_obj_set_style_border_side(target_sheet, LV_BORDER_SIDE_LEFT | LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_radius(target_sheet, 12, 0);
    lv_obj_set_scrollbar_mode(target_sheet, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(target_sheet, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(target_sheet, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(target_sheet, 12, 0);
    lv_obj_set_style_pad_all(target_sheet, 10, 0);

    // 4.1 基准调节按钮
    lv_obj_t* btn_base_p = lv_btn_create(target_sheet);
    lv_obj_set_size(btn_base_p, 140, 60);
    lv_obj_set_style_bg_color(btn_base_p, lv_color_hex(0x22C55E), 0);
    lv_obj_add_event_cb(btn_base_p, btn_target_base_plus_cb, LV_EVENT_CLICKED, this);
    lv_obj_t* lbl_base_p = lv_label_create(btn_base_p);
    lv_obj_set_style_text_font(lbl_base_p, &ui_font_chs_16, 0);
    lv_label_set_text(lbl_base_p, "基准 +10g");
    lv_obj_center(lbl_base_p);

    lv_obj_t* btn_base_m = lv_btn_create(target_sheet);
    lv_obj_set_size(btn_base_m, 140, 60);
    lv_obj_set_style_bg_color(btn_base_m, lv_color_hex(0x6366F1), 0); // 使用靛蓝色区分
    lv_obj_add_event_cb(btn_base_m, btn_target_base_minus_cb, LV_EVENT_CLICKED, this);
    lv_obj_t* lbl_base_m = lv_label_create(btn_base_m);
    lv_obj_set_style_text_font(lbl_base_m, &ui_font_chs_16, 0);
    lv_label_set_text(lbl_base_m, "基准 -10g");
    lv_obj_center(lbl_base_m);

    // 4.2 误差调节按钮
    lv_obj_t* btn_off_p = lv_btn_create(target_sheet);
    lv_obj_set_size(btn_off_p, 140, 60);
    lv_obj_set_style_bg_color(btn_off_p, lv_color_hex(0x10B981), 0);
    lv_obj_add_event_cb(btn_off_p, btn_target_offset_plus_cb, LV_EVENT_CLICKED, this);
    lv_obj_t* lbl_off_p = lv_label_create(btn_off_p);
    lv_obj_set_style_text_font(lbl_off_p, &ui_font_chs_16, 0);
    lv_label_set_text(lbl_off_p, "误差 +1g");
    lv_obj_center(lbl_off_p);

    lv_obj_t* btn_off_m = lv_btn_create(target_sheet);
    lv_obj_set_size(btn_off_m, 140, 60);
    lv_obj_set_style_bg_color(btn_off_m, lv_color_hex(0xEF4444), 0);
    lv_obj_add_event_cb(btn_off_m, btn_target_offset_minus_cb, LV_EVENT_CLICKED, this);
    lv_obj_t* lbl_off_m = lv_label_create(btn_off_m);
    lv_obj_set_style_text_font(lbl_off_m, &ui_font_chs_16, 0);
    lv_label_set_text(lbl_off_m, "误差 -1g");
    lv_obj_center(lbl_off_m);
}

void UIManager::closeTargetBottomSheet() {
    if (target_sheet) {
        lv_obj_del(target_sheet);
        target_sheet = nullptr;
    }
    if (target_sheet_bg) {
        lv_obj_del(target_sheet_bg);
        target_sheet_bg = nullptr;
    }

    // 关键点：已移除 target_label 回迁逻辑
}
