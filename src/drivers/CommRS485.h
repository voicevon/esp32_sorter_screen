#ifndef COMM_RS485_H
#define COMM_RS485_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include "system/SystemContext.h"
#include "PinDefinition.h"

struct UIEvent {
    String cmd;
    int params; // -1 if not applicable
};

class CommRS485 {
public:
    CommRS485();
    void begin(SystemContext* ctx);
    void loop(); // Call frequently to process RX and send TX
    void pushEvent(const String& cmd, int params = -1);

private:
    SystemContext* _ctx;
    std::vector<UIEvent> _eventQueue;
    String _rxBuffer;
    HardwareSerial* _serial;
    
    void processLine(const String& line);
    void handleDashboard(JsonObject data);
    void handleAdminEncoder(JsonObject data);
    void handleAdminLaser(JsonObject data);
    void handleAdminCutter(JsonObject data);
    
    void sendResponse();
    uint8_t calculateCRC8(const char* data, size_t len);
    
    // Internal cache for the current tab page to report back to Master
    String _currentPage;
};

#endif // COMM_RS485_H
