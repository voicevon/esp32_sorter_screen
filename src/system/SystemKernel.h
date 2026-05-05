#ifndef SYSTEM_KERNEL_H
#define SYSTEM_KERNEL_H

#include <Arduino.h>
#include <vector>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "system/SystemContext.h"
#include "system/SystemTypes.h"
#include "apps/IApp.h"
#include "ui/UIManager.h"
#include "drivers/CommRS485.h"

class SystemKernel : public ICommandBus {
public:
    SystemKernel(SystemContext* ctx, UIManager* ui);

    void registerApp(IApp* app);
    void begin(OperationMode initialMode = MODE_PRODUCTION);

    // ICommandBus 接口实现
    void cmdToggleDiagnosis(bool active) override {} // Default empty
    void updateOperationMode(OperationMode mode) override;
    void updateAdminPage(uint8_t pageId) override;
    void onOutletEdit(int index, int action) override;
    void onOutletDiag(int index, bool state) override;
    void pushEvent(const String& cmd, int index, int params) override;

private:
    SystemContext*        _ctx;
    UIManager*            _ui;
    CommRS485             _comm;

    SemaphoreHandle_t     _mutexCtx;
    TaskHandle_t          _uiTaskHandle;
    TaskHandle_t          _commTaskHandle;

    // 任务入口
    static void uiTaskEntry(void* self);
    static void commTaskEntry(void* self);

    // 任务循环
    void runUILoop();
    void runCommLoop();
};

#endif
