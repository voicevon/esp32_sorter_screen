#include "SystemKernel.h"
#include <Arduino.h>

SystemKernel::SystemKernel(SystemContext* ctx, UIManager* ui) 
    : _ctx(ctx), _ui(ui) {
    _mutexCtx = xSemaphoreCreateMutex();
}

void SystemKernel::begin(OperationMode mode) {
    _ctx->ui.curMode = mode;
    
    // 初始化通讯总线
    _comm.begin(_ctx);
    _ui->setCommandBus(this);

    // 创建核心任务
    xTaskCreatePinnedToCore(uiTaskEntry, "UI_Loop", 16384, this, 1, &_uiTaskHandle, 1);
    xTaskCreatePinnedToCore(commTaskEntry, "Comm_Task", 8192, this, 2, &_commTaskHandle, 0);

    Serial.println("[Kernel] Tasks created.");
}

void SystemKernel::registerApp(IApp* app) {
    // Currently minimal implementation
}

// --- I_Command_Bus Implementation ---
void SystemKernel::updateOperationMode(OperationMode mode) {
    xSemaphoreTake(_mutexCtx, portMAX_DELAY);
    _ctx->ui.curMode = mode;
    _ctx->ui.dirtyFlags |= DF_OP_MODE;
    xSemaphoreGive(_mutexCtx);
}

void SystemKernel::updateAdminPage(uint8_t pageId) {
    xSemaphoreTake(_mutexCtx, portMAX_DELAY);
    _ctx->ui.diag_page_id = pageId;
    _ctx->ui.dirtyFlags |= DF_OP_MODE;
    xSemaphoreGive(_mutexCtx);
}

void SystemKernel::onOutletEdit(int index, int action) {
    xSemaphoreTake(_mutexCtx, portMAX_DELAY);
    if (index >= 0 && index < 8) {
        float step = 0.5f;
        if (action == 0) _ctx->ui.outlets[index].minDiameter -= step;
        else if (action == 1) _ctx->ui.outlets[index].minDiameter += step;
        else if (action == 2) _ctx->ui.outlets[index].maxDiameter -= step;
        else if (action == 3) _ctx->ui.outlets[index].maxDiameter += step;
        else if (action == 4) _ctx->ui.outlets[index].lengthMask ^= 0x01;
        else if (action == 5) _ctx->ui.outlets[index].lengthMask ^= 0x02;
        else if (action == 6) _ctx->ui.outlets[index].lengthMask ^= 0x04;
        
        if (_ctx->ui.outlets[index].minDiameter < 0) _ctx->ui.outlets[index].minDiameter = 0;
        if (_ctx->ui.outlets[index].maxDiameter > 50) _ctx->ui.outlets[index].maxDiameter = 50;
        
        _comm.pushEvent("set_outlet", index, 0);
    }
    xSemaphoreGive(_mutexCtx);
}

void SystemKernel::onOutletDiag(int index, bool state) {
    _comm.pushEvent("diag_outlet", index, state ? 1 : 0);
}

void SystemKernel::pushEvent(const String& cmd, int index, int params) {
    _comm.pushEvent(cmd, index, params);
}

// --- Task Entry Points ---
void SystemKernel::uiTaskEntry(void* self) {
    ((SystemKernel*)self)->runUILoop();
}

void SystemKernel::commTaskEntry(void* self) {
    ((SystemKernel*)self)->runCommLoop();
}

// --- Task Loops ---
void SystemKernel::runUILoop() {
    Serial.printf("[Kernel] UI Loop Started on Core %d\n", xPortGetCoreID());
    
    uint32_t lastHb = 0;
    while (1) {
        lv_tick_inc(20); 
        lv_timer_handler();

        _ui->updateDashboard(_ctx);
        _ctx->ui.dirtyFlags = DF_NONE;

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void SystemKernel::runCommLoop() {
    Serial.printf("[Kernel] Comm Loop Started on Core %d\n", xPortGetCoreID());
    while (1) {
        _comm.loop();

        if (millis() - _ctx->ui.last_comm_time > 2000) {
            _ctx->ui.sysStatus = SYS_ERROR; // Offline
        } else {
            _ctx->ui.sysStatus = SYS_READY; // Online
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
