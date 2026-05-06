#include "DiagCommView.h"
#include "drivers/PinDefinition.h"
#include <Arduino.h>

LV_FONT_DECLARE(ui_font_chs_16);
extern const lv_font_t lv_font_montserrat_12;

void DiagCommView::build(lv_obj_t* parent, ICommandBus* bus) {
    _bus = bus;

    lv_obj_t* comm_info = lv_label_create(parent);
    lv_obj_set_style_text_color(comm_info, lv_color_hex(0x94A3B8), 0);
    lv_obj_set_style_text_font(comm_info, &ui_font_chs_16, 0);
    lv_label_set_text_fmt(comm_info, "串口: Serial1 (RX:%d TX:%d) | 波特率: %d", PIN_RS485_RX, PIN_RS485_TX, RS485_BAUD);
    
    lv_obj_t* log_cont = lv_obj_create(parent);
    lv_obj_set_size(log_cont, lv_pct(100), lv_pct(80));
    lv_obj_align(log_cont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(log_cont, lv_color_hex(0x020617), 0);
    lv_obj_set_style_border_width(log_cont, 1, 0);
    lv_obj_set_style_border_color(log_cont, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_pad_all(log_cont, 2, 0);
    lv_obj_set_scrollbar_mode(log_cont, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_flex_flow(log_cont, LV_FLEX_FLOW_ROW);
    
    hex_label = lv_label_create(log_cont);
    lv_obj_set_width(hex_label, 300);
    lv_obj_set_style_text_color(hex_label, lv_color_hex(0x38BDF8), 0); 
    lv_obj_set_style_text_font(hex_label, &lv_font_montserrat_12, 0);
    
    lv_obj_t* line = lv_obj_create(log_cont);
    lv_obj_set_size(line, 2, lv_pct(100));
    lv_obj_set_style_bg_color(line, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_border_width(line, 0, 0);

    ascii_label = lv_label_create(log_cont);
    lv_obj_set_flex_grow(ascii_label, 1);
    lv_obj_set_style_text_color(ascii_label, lv_color_hex(0x10B981), 0); 
    lv_obj_set_style_text_font(ascii_label, &lv_font_montserrat_12, 0);
}

void DiagCommView::update(const SystemContext* ctx) {
    if (!ctx) return;

    if (hex_label && ascii_label) {
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
        lv_label_set_text(hex_label, fullHex.c_str());
        lv_label_set_text(ascii_label, fullAscii.c_str());
    }
}
