#include "DiagScannerView.h"
#include <Arduino.h>

extern const lv_font_t lv_font_montserrat_12;
extern const lv_font_t ui_font_chs_16;

void DiagScannerView::build(lv_obj_t* parent, ICommandBus* bus) {
    _bus = bus;

    lv_obj_t* laser_cont = lv_obj_create(parent);
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
        
        leds[i] = lv_led_create(wrapper);
        lv_obj_set_size(leds[i], 16, 16);
        lv_led_set_color(leds[i], lv_color_hex(0x10B981));
        lv_led_off(leds[i]);
        
        lv_obj_t* label = lv_label_create(wrapper);
        lv_label_set_text_fmt(label, "P%d", i+1);
        lv_obj_set_style_text_color(label, lv_color_hex(0x94A3B8), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);
    }

    // 5. 显示 sample_count 采样时刻的 Label
    lv_obj_t* sample_wrapper = lv_obj_create(indicator_cont);
    lv_obj_set_size(sample_wrapper, 160, 50);
    lv_obj_set_style_pad_all(sample_wrapper, 0, 0);
    lv_obj_set_style_bg_opa(sample_wrapper, 0, 0);
    lv_obj_set_style_border_width(sample_wrapper, 0, 0);
    lv_obj_set_flex_flow(sample_wrapper, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(sample_wrapper, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    label_sample_count = lv_label_create(sample_wrapper);
    lv_label_set_text(label_sample_count, "Samples: --");
    lv_obj_set_style_text_color(label_sample_count, lv_color_hex(0xFBBF24), 0); // 暖橙色
    lv_obj_set_style_text_font(label_sample_count, &ui_font_chs_16, 0); // 统一大方的高级字库

    chart = lv_chart_create(laser_cont);
    lv_obj_set_size(chart, lv_pct(100), lv_pct(70));
    lv_obj_align(chart, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(chart, 200);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, 50);
    lv_chart_set_div_line_count(chart, 0, 0); // 彻底删除横纵辅助网格线
    lv_obj_set_style_bg_color(chart, lv_color_hex(0x020617), 0);
    lv_obj_set_style_border_color(chart, lv_color_hex(0x1E293B), 0);
    lv_color_t colors[] = {lv_color_hex(0x38BDF8), lv_color_hex(0x10B981), lv_color_hex(0xF43F5E), lv_color_hex(0xFBBF24)};
    for(int i=0; i<NUM_SCAN_POINTS; i++) {
        series[i] = lv_chart_add_series(chart, colors[i], LV_CHART_AXIS_PRIMARY_Y);
    }
    
    // 添加橙黄色竖线游标 (Vertical Line Cursor)
    cursor = lv_chart_add_cursor(chart, lv_color_hex(0xFBBF24), LV_DIR_VER);
}

void DiagScannerView::update(const SystemContext* ctx) {
    if (!ctx) return;

    for (int i = 0; i < NUM_SCAN_POINTS; i++) {
        if (leds[i]) {
            bool isOn = (ctx->ui.diag_laser_states >> i) & 0x01;
            if (isOn) {
                lv_led_set_color(leds[i], lv_color_hex(0xF43F5E)); // 红: 遮挡
            } else {
                lv_led_set_color(leds[i], lv_color_hex(0x10B981)); // 绿: 畅通
            }
            lv_led_on(leds[i]);
        }
    }

    if (label_sample_count) {
        lv_label_set_text_fmt(label_sample_count, "Samples: %d", ctx->ui.diag_sample_count);
    }

    if (chart) {
        for (int i = 0; i < NUM_SCAN_POINTS; i++) {
            if (series[i]) {
                for (int j = 0; j < 200; j++) {
                    int byteIdx = j / 8;
                    int bitIdx = 7 - (j % 8); 
                    bool val = (ctx->ui.diag_laser_history[i][byteIdx] >> bitIdx) & 0x01;
                    lv_chart_set_value_by_id(chart, series[i], j, i * 10 + (val ? 8 : 0));
                }
            }
        }
        if (cursor && series[0]) {
            int index = ctx->ui.diag_sample_count;
            if (index < 0) index = 0;
            if (index > 199) index = 199;
            lv_chart_set_cursor_point(chart, cursor, series[0], index);
        }
        lv_chart_refresh(chart);
    }
}
