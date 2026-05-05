#include "UIManager.h"
#include <Arduino.h>

void UIManager::buildConfigView(lv_obj_t* parent) {
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_style_pad_all(parent, 10, 0);

    lv_obj_t* outlet_cont = lv_obj_create(parent);
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

        create_adjust("Min", &config_outlet_ui[i].label_min, 0, 1);
        lv_obj_t* div = lv_label_create(row); lv_label_set_text(div, "~");
        create_adjust("Max", &config_outlet_ui[i].label_max, 2, 3);

        const char* ln[] = {"S", "M", "L"};
        lv_obj_t** lp[] = {&config_outlet_ui[i].cb_s, &config_outlet_ui[i].cb_m, &config_outlet_ui[i].cb_l};
        for(int j=0; j<3; j++) {
            *lp[j] = lv_checkbox_create(row);
            lv_checkbox_set_text(*lp[j], ln[j]);
            lv_obj_set_style_text_color(*lp[j], lv_color_white(), 0);
            lv_obj_set_user_data(*lp[j], (void*)(long)((i<<4)|(4+j)));
            lv_obj_add_event_cb(*lp[j], edit_cb, LV_EVENT_VALUE_CHANGED, this);
        }
    }
}
