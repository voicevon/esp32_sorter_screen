#include "DiagOutletView.h"
#include <Arduino.h>

extern const lv_font_t lv_font_montserrat_12;

void DiagOutletView::build(lv_obj_t* parent, ICommandBus* bus) {
    _bus = bus;

    lv_obj_t* outlet_cont = lv_obj_create(parent);
    lv_obj_set_size(outlet_cont, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_opa(outlet_cont, 0, 0);
    lv_obj_set_style_border_width(outlet_cont, 0, 0);
    lv_obj_set_flex_flow(outlet_cont, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(outlet_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_gap(outlet_cont, 15, 0);

    auto diag_cb = [](lv_event_t* e) {
        DiagOutletView* view = (DiagOutletView*)lv_event_get_user_data(e);
        lv_obj_t* obj = lv_event_get_target(e);
        int idx = (int)(long)lv_obj_get_user_data(obj);
        
        if (view && view->_bus) {
            bool currentState = view->_lastOutletStates[idx];
            view->_bus->onOutletDiag(idx, !currentState);
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

        leds[i] = lv_led_create(item);
        lv_obj_set_size(leds[i], 16, 16);
        lv_led_set_color(leds[i], lv_color_hex(0x38BDF8));
        lv_led_off(leds[i]);

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
}

void DiagOutletView::update(const SystemContext* ctx) {
    if (!ctx) return;

    for (int i = 0; i < 8; i++) {
        _lastOutletStates[i] = ctx->ui.outlets[i].state;
        if (leds[i]) {
            if (ctx->ui.outlets[i].state) {
                lv_led_on(leds[i]);
                lv_led_set_color(leds[i], lv_color_hex(0x38BDF8)); 
            } else {
                lv_led_off(leds[i]);
            }
        }
    }
}
