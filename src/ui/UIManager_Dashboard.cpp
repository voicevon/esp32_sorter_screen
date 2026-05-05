#include "UIManager.h"
#include <Arduino.h>

void UIManager::buildDashboardView(lv_obj_t* parent) {
    lv_obj_set_style_bg_color(parent, lv_color_hex(0x0F172A), 0);
    lv_obj_set_style_border_width(parent, 0, 0);
    lv_obj_set_style_pad_all(parent, 10, 0);
    
    // 允许父容器滚动链，确保侧滑能穿透到 tabview
    lv_obj_add_flag(parent, LV_OBJ_FLAG_SCROLL_CHAIN);

    // --- Top Bar ---
    lv_obj_t* top_bar = lv_obj_create(parent);
    lv_obj_set_size(top_bar, 760, 60);
    lv_obj_align(top_bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(top_bar, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_border_width(top_bar, 0, 0);
    lv_obj_set_style_radius(top_bar, 12, 0);
    // 隐藏滚动条但保留滚动属性，以允许手势冒泡
    lv_obj_set_scrollbar_mode(top_bar, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(top_bar, LV_OBJ_FLAG_EVENT_BUBBLE);

    lv_obj_t* title_lbl = lv_label_create(top_bar);
    lv_obj_set_style_text_font(title_lbl, &ui_font_chs_16, 0);
    lv_obj_set_style_text_color(title_lbl, lv_color_white(), 0);
    lv_label_set_text(title_lbl, "芦笋分拣系统 - 运行监控");
    lv_obj_align(title_lbl, LV_ALIGN_LEFT_MID, 20, 0);

    // Frame Counter (Top Left)
    label_frame_counter = lv_label_create(top_bar);
    lv_obj_set_style_text_font(label_frame_counter, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(label_frame_counter, lv_color_hex(0x64748B), 0);
    lv_label_set_text(label_frame_counter, "FC: 0");
    lv_obj_align(label_frame_counter, LV_ALIGN_LEFT_MID, 320, 0);

    // Communication status LED and Label
    lv_obj_t* comm_label = lv_label_create(top_bar);
    lv_obj_set_style_text_font(comm_label, &ui_font_chs_16, 0);
    lv_obj_set_style_text_color(comm_label, lv_color_white(), 0);
    lv_label_set_text(comm_label, "通讯状态");
    lv_obj_align(comm_label, LV_ALIGN_RIGHT_MID, -50, 0);

    comm_led = lv_obj_create(top_bar);
    lv_obj_set_size(comm_led, 20, 20);
    lv_obj_align(comm_led, LV_ALIGN_RIGHT_MID, -20, 0);
    lv_obj_set_style_radius(comm_led, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(comm_led, 2, 0);
    lv_obj_set_style_border_color(comm_led, lv_color_hex(0x0F172A), 0);
    lv_obj_set_style_bg_color(comm_led, lv_color_hex(0xEF4444), 0); 

    // --- Data Container ---
    lv_obj_t* container = lv_obj_create(parent);
    lv_obj_set_size(container, 760, 320);
    lv_obj_align(container, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_color(container, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_radius(container, 12, 0);
    // 核心修复：允许滚动链冒泡，隐藏内部滚动条
    lv_obj_set_scrollbar_mode(container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(container, LV_OBJ_FLAG_SCROLL_CHAIN);
    lv_obj_add_flag(container, LV_OBJ_FLAG_EVENT_BUBBLE);
    
    static lv_coord_t col_dsc[] = {340, 340, LV_COORD_MAX};
    static lv_coord_t row_dsc[] = {120, 120, LV_COORD_MAX};
    lv_obj_set_layout(container, LV_LAYOUT_GRID);
    lv_obj_set_grid_dsc_array(container, col_dsc, row_dsc);
    lv_obj_set_style_pad_row(container, 20, 0);
    lv_obj_set_style_pad_column(container, 30, 0);
    lv_obj_set_grid_align(container, LV_GRID_ALIGN_CENTER, LV_GRID_ALIGN_CENTER);

    const char* labels[] = {
        "直径: 0 mm", 
        "速度: 0 根/秒", 
        "产量: 0 根", 
        "产能: 0 根/小时"
    };
    
    lv_obj_t** vars[] = {
        &label_diameter, 
        &label_speed_sec, 
        &label_yield, 
        &label_capacity
    };

    for (int i = 0; i < 4; i++) {
        lv_obj_t* panel = lv_obj_create(container);
        lv_obj_set_style_bg_color(panel, lv_color_hex(0x334155), 0);
        lv_obj_set_style_border_width(panel, 0, 0);
        lv_obj_set_style_radius(panel, 8, 0);
        // 允许事件穿透
        lv_obj_set_scrollbar_mode(panel, LV_SCROLLBAR_MODE_OFF);
        lv_obj_add_flag(panel, LV_OBJ_FLAG_EVENT_BUBBLE);
        
        lv_obj_set_grid_cell(panel, LV_GRID_ALIGN_STRETCH, i % 2, 1,
                                    LV_GRID_ALIGN_STRETCH, i / 2, 1);

        *vars[i] = lv_label_create(panel);
        lv_obj_set_style_text_font(*vars[i], &ui_font_chs_16, 0);
        lv_obj_set_style_text_color(*vars[i], lv_color_white(), 0);
        lv_label_set_text(*vars[i], labels[i]);
        lv_obj_center(*vars[i]);
    }
}
