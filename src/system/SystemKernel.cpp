#include "system/SystemKernel.h"
#include <lvgl.h>
#include "system/SystemContext.h"

SystemKernel::SystemKernel(SystemContext* ctx, UIManager* ui)
    : _ctx(ctx), _ui(ui) {
    _mutexCtx = xSemaphoreCreateMutex();
}

void SystemKernel::registerApp(IApp* app) {
    _apps.push_back(app);
}

void SystemKernel::begin(OperationMode initialMode) {
    _ui->setCommandBus(this);
    
    // 初始化系统上下文
    xSemaphoreTake(_mutexCtx, portMAX_DELAY);
    _ctx->prog.sysStatus = SYS_READY;
    strncpy(_ctx->prog.statusText, "系统就绪", 32); 
    _ctx->ui.curMode = initialMode;
    xSemaphoreGive(_mutexCtx);

    _comm.begin(_ctx);

    // 启动 FreeRTOS 任务
    xTaskCreatePinnedToCore(controlTaskEntry, "ControlTask", 8192, this, 10, NULL, 1);
    xTaskCreatePinnedToCore(uiTaskEntry,      "UITask",      8192, this,  5, NULL, 0);

    delay(100);
    updateOperationMode(initialMode);
}

// =============================================================================
// ICommandBus 实现 (路由到具体 App)
// =============================================================================

void SystemKernel::cmdGlobalTare() {
    _comm.pushEvent("tare");
}
void SystemKernel::cmdGlobalServo(bool open) {}
void SystemKernel::cmdStartScan() {}
void SystemKernel::cmdCancelScan() {}
void SystemKernel::cmdToggleDiagnosis(bool active) {}
void SystemKernel::cmdServoTest(int id, bool open) {}
void SystemKernel::cmdBeltTest(int beltIndex, int distanceMm) {}
void SystemKernel::cmdBeltRun(int beltIndex, bool run) {}
void SystemKernel::cmdTriggerBeltScan() {}
void SystemKernel::cmdSerialSendHex(const char* hexStr) {}
void SystemKernel::cmdSerialToggleAuto(bool enable) {}
void SystemKernel::cmdSetDiagSubMode(int mode) {}
void SystemKernel::cmdSetDiagTarget(int id) {}
void SystemKernel::cmdDiagAction(int actionId) {}
void SystemKernel::cmdUpdateTargetBase(float delta) {}
void SystemKernel::cmdUpdateTargetOffset(float delta) {}
void SystemKernel::cmdUpdateTargets(float dMin, float dMax) {}

// =============================================================================
// 模式管理
// =============================================================================

void SystemKernel::updateOperationMode(OperationMode newMode) {
    if (_pendingMode == newMode && _currentMode == newMode) return;
    Serial.printf("[Kernel] Mode Switch REQUESTED: %s\n", modeToStr(newMode));
    _pendingMode = newMode;
}

void SystemKernel::executeModeSwitch() {
    if (_pendingMode == _currentMode) return;

    Serial.printf("[Kernel] ATOMIC SWITCH: %s -> %s\n",
                  modeToStr(_currentMode),
                  modeToStr(_pendingMode));

    if (_currentApp) _currentApp->onExit();

    _currentMode = _pendingMode;
    
    xSemaphoreTake(_mutexCtx, portMAX_DELAY);
    _ctx->ui.curMode = _currentMode;
    _ctx->prog.dirtyFlags |= DF_OP_MODE; // 设置运行模式脏标记
    xSemaphoreGive(_mutexCtx);

    _currentApp = findApp(_currentMode);
    if (_currentApp) _currentApp->onEnter();
}

bool SystemKernel::canSwitchMode() const {
    return true; // 无 Modbus 阻塞
}

IApp* SystemKernel::findApp(OperationMode mode) {
    for (auto app : _apps) {
        if (app->getMode() == mode) return app;
    }
    return nullptr;
}

// =============================================================================
// 任务循环
// =============================================================================

void SystemKernel::controlTaskEntry(void* self) {
    static_cast<SystemKernel*>(self)->controlLoop();
}

void SystemKernel::controlLoop() {
    Serial.println("[Kernel] Control Task Started on Core 1");
    while (true) {
        if (_pendingMode != _currentMode && canSwitchMode()) {
            executeModeSwitch();
        }

        if (_currentApp) {
            _currentApp->onLoop();
            if (_currentMode != MODE_PRODUCTION && _currentApp->isFinished()) {
                updateOperationMode(MODE_PRODUCTION);
            }
        }
        
        _comm.loop();
        
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void SystemKernel::uiTaskEntry(void* self) {
    static_cast<SystemKernel*>(self)->uiLoop();
}

void SystemKernel::uiLoop() {
    Serial.println("[Kernel] UI Task Started on Core 0");
    while (true) {
        xSemaphoreTake(_mutexCtx, portMAX_DELAY);
        
        // 捕获并在 UI 层累加脏标记
        _ctx->ui.dirtyFlags |= _ctx->prog.dirtyFlags;
        _ctx->prog.dirtyFlags = DF_NONE; // 重置逻辑层脏标记

        // 总是保持实时数据的脏标记
        _ctx->ui.dirtyFlags |= DF_LIVE_DATA; 

        xSemaphoreGive(_mutexCtx);

        // UI 渲染 (传入带脏标记的上下文)
        _ui->updateDashboard(_ctx);
        
        _ctx->ui.dirtyFlags = DF_NONE;

        lv_tick_inc(UI_REFRESH_INTERVAL_MS);
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(UI_REFRESH_INTERVAL_MS));
    }
}

const char* SystemKernel::modeToStr(OperationMode m) {
    switch (m) {
        case MODE_PRODUCTION:      return "PRODUCTION";
        case MODE_CONFIGURATION:   return "CONFIGURATION";
        case MODE_ABOUT:           return "ABOUT";
        default:                   return "UNKNOWN";
    }
}
