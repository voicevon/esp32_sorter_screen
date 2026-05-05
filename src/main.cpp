/**
 * @file main.cpp
 * @brief 系统启动入口（ESP32 Sorter Screen）
 */

#include <Arduino.h>
#include "drivers/TouchScreen.h"
#include "drivers/PinDefinition.h"

// Apps & System
#include "system/SystemKernel.h"
#include "apps/IApp.h"

// --- 全局共享状态 ---
SystemContext      sysCtx;

// --- 驱动层 ---
TouchScreen        hw;
UIManager          ui;

// --- 核心调度器 (系统内核) ---
SystemKernel      kernel(&sysCtx, &ui);

void setup() {
    Serial.begin(115200);
    delay(500); 
    Serial.println("\n[SYSTEM] Starting ESP32 Sorter Screen...");

    if (hw.begin()) {
        hw.lvglInit();
        ui.init();
        Serial.println("[SYSTEM] Hardware & UI Init OK.");
    } else {
        Serial.println("[SYSTEM] CRITICAL: HW Init Failed");
        while (1) delay(100);
    }

    // 这里可以后续注册 AppSelfTest 等
    // kernel.registerApp(&appSelfTest);

    // 启动调度器
    kernel.begin(MODE_PRODUCTION); 
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
