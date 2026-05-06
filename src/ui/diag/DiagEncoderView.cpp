#include "DiagEncoderView.h"
#include <Arduino.h>

LV_FONT_DECLARE(ui_font_chs_16);
extern const lv_font_t lv_font_montserrat_26;

void DiagEncoderView::build(lv_obj_t* parent, ICommandBus* bus) {
    _bus = bus;

    lv_obj_t* enc_cont = lv_obj_create(parent);
    lv_obj_set_size(enc_cont, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(enc_cont, 0, 0);
    lv_obj_set_style_border_width(enc_cont, 0, 0);
    lv_obj_set_flex_flow(enc_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_gap(enc_cont, 10, 0);
    
    label_raw = lv_label_create(enc_cont);
    label_corrected = lv_label_create(enc_cont);
    label_logic = lv_label_create(enc_cont);
    label_zero = lv_label_create(enc_cont);
    label_status = lv_label_create(enc_cont);
    
    lv_obj_t* enc_labels[] = {
        label_raw, 
        label_corrected, 
        label_logic, 
        label_zero, 
        label_status
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
        DiagEncoderView* view = (DiagEncoderView*)lv_event_get_user_data(e);
        lv_obj_t* btn = lv_event_get_target(e);
        int delta = (int)(long)lv_obj_get_user_data(btn);
        
        if (view && view->_bus) {
            int current = view->_lastOffset;
            int target = current + delta;
            if (target < 0) target = 0;
            if (target > 199) target = 199;
            view->_bus->pushEvent("set_offset", 0, target);
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

    label_offset = lv_label_create(offset_cont);
    lv_obj_set_style_text_color(label_offset, lv_color_hex(0x38BDF8), 0);
    lv_obj_set_style_text_font(label_offset, &lv_font_montserrat_26, 0);
    lv_obj_set_width(label_offset, 60);
    lv_obj_set_style_text_align(label_offset, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(label_offset, "0");

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
}

void DiagEncoderView::update(const SystemContext* ctx) {
    if (!ctx) return;
    
    _lastOffset = ctx->ui.diag_encoder_offset;

    if (label_raw) {
        lv_label_set_text_fmt(label_raw, "原始值 (Raw): %d", ctx->ui.diag_encoder_raw);
    }
    if (label_corrected) {
        lv_label_set_text_fmt(label_corrected, "修正值 (Corrected): %d", ctx->ui.diag_encoder_corrected);
    }
    if (label_logic) {
        lv_label_set_text_fmt(label_logic, "逻辑值 (Logic): %d", ctx->ui.diag_encoder_logic);
    }
    if (label_zero) {
        lv_label_set_text_fmt(label_zero, "经历 Z 信号次数: %d", ctx->ui.diag_encoder_zero_count);
    }
    if (label_status) {
        lv_label_set_text_fmt(label_status, "Zero 统计 (正确/总计): %d / %d", 
                              ctx->ui.diag_encoder_zero_correct, ctx->ui.diag_encoder_zero_total);
    }
    if (label_offset) {
        lv_label_set_text_fmt(label_offset, "%d", ctx->ui.diag_encoder_offset);
    }
}
