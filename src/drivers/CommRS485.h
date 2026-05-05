#ifndef COMM_RS485_H
#define COMM_RS485_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include "system/SystemContext.h"
#include "PinDefinition.h"

struct UIEvent {
    String cmd;
    int index;
    int state; // -1 if not applicable
};

class CommRS485 {
public:
    CommRS485();
    void begin(SystemContext* ctx);
    void loop(); // Call frequently to process RX and send TX
    void pushEvent(const String& cmd, int index = -1, int state = -1);
    const std::vector<String>& getLogsHex() const { return _logBufferHex; }
    const std::vector<String>& getLogsAscii() const { return _logBufferAscii; }

private:
    SystemContext* _ctx;
    std::vector<UIEvent> _eventQueue;
    String _rxBuffer;
    HardwareSerial* _serial;
    
    void processLine(const String& line);
    void handleDashboard(JsonObject data);
    void handleDiagEncoder(JsonObject data);
    void handleDiagLaser(JsonObject data);
    void handleDiagOutlets(JsonArray data);
    
    void sendResponse();
    uint8_t calculateCRC8(const char* data, size_t len);
    
    
    // Internal cache for the current tab page to report back to Master
    String _currentPage;

    // Diagnosis Logging
    std::vector<String> _logBufferHex;
    std::vector<String> _logBufferAscii;
    void addLog(const String& line, bool isTX);
};

#endif // COMM_RS485_H
