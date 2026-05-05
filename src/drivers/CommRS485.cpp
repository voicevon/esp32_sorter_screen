#include "CommRS485.h"

CommRS485::CommRS485() {
    _ctx = nullptr;
    _serial = &Serial1; // Using Serial1 for RS485
    _currentPage = "dashboard";
}

void CommRS485::begin(SystemContext* ctx) {
    _ctx = ctx;
    // RS485_BAUD is defined in PinDefinition.h
    _serial->begin(RS485_BAUD, SERIAL_8N2, PIN_RS485_RX, PIN_RS485_TX);
    _rxBuffer.reserve(512);
    Serial.println("[CommRS485] Initialized as Slave.");
}

uint8_t CommRS485::calculateCRC8(const char* data, size_t len) {
    uint8_t crc = 0x00;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x07;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

void CommRS485::pushEvent(const String& cmd, int params) {
    _eventQueue.push_back({cmd, params});
}

void CommRS485::loop() {
    while (_serial->available()) {
        char c = _serial->read();
        if (c == '\n') {
            addLog(_rxBuffer, false); // Log raw RX line
            processLine(_rxBuffer);
            _rxBuffer = "";
        } else {
            _rxBuffer += c;
            // Prevent buffer overflow in case of missing newline
            if (_rxBuffer.length() > 500) {
                _rxBuffer = "";
            }
        }
    }
}

void CommRS485::processLine(const String& line) {
    // Format expected: $<JSON>*<CRC8>\r or without \r
    String cleanLine = line;
    cleanLine.trim(); // remove \r if any
    
    if (!cleanLine.startsWith("$")) return;
    
    int starIdx = cleanLine.lastIndexOf('*');
    if (starIdx < 1) return;
    
    String jsonStr = cleanLine.substring(1, starIdx);
    String crcHexStr = cleanLine.substring(starIdx + 1);
    
    // Calculate CRC
    uint8_t calcCrc = calculateCRC8(jsonStr.c_str(), jsonStr.length());
    uint8_t recvCrc = (uint8_t) strtol(crcHexStr.c_str(), NULL, 16);
    
    if (calcCrc != recvCrc) {
        Serial.printf("[CommRS485] CRC mismatch! Calc:%02X Recv:%02X\n", calcCrc, recvCrc);
        return;
    }
    
    // Parse JSON
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, jsonStr);
    
    if (error) {
        Serial.printf("[CommRS485] JSON parse failed: %s\n", error.c_str());
        return;
    }
    
    String type = doc["type"] | "unknown";
    JsonObject data = doc["data"];
    
    if (type == "dashboard") {
        handleDashboard(data);
    } else if (type == "admin_encoder") {
        handleAdminEncoder(data);
    } else if (type == "admin_laser") {
        handleAdminLaser(data);
    } else if (type == "admin_cutter") {
        handleAdminCutter(data);
    }
    
    if (_ctx) {
        _ctx->ui.last_comm_time = millis();
    }
    
    // Since we are Slave, we must reply immediately upon receiving a valid request
    sendResponse();
}

void CommRS485::handleDashboard(JsonObject data) {
    if (!_ctx) return;
    _ctx->ui.frame_counter = data["frame_counter"] | 0;
    _ctx->ui.dashboard_speed = data["speed"] | 0.0f;
    _ctx->ui.dashboard_yield = data["yield"] | 0.0f;
    _ctx->ui.dashboard_capacity = data["capacity"] | 0.0f;
    _ctx->ui.dashboard_diameter = data["diameter"] | 0.0f;
    _ctx->ui.dirtyFlags |= DF_LIVE_DATA; // Trigger UI update
}

void CommRS485::handleAdminEncoder(JsonObject data) {
    if (!_ctx) return;
    _ctx->ui.admin_encoder_pulses = data["pulse_count"] | 0;
    _ctx->ui.admin_encoder_velocity = data["velocity"] | 0.0f;
    _ctx->ui.admin_encoder_status = data["status"] | 0;
    _ctx->ui.dirtyFlags |= DF_DIAG;
}

void CommRS485::handleAdminLaser(JsonObject data) {
    if (!_ctx) return;
    _ctx->ui.admin_laser_distance = data["distance"] | 0.0f;
    _ctx->ui.admin_laser_intensity = data["intensity"] | 0.0f;
    _ctx->ui.admin_laser_status = data["status"] | 0;
    _ctx->ui.dirtyFlags |= DF_DIAG;
}

void CommRS485::handleAdminCutter(JsonObject data) {
    if (!_ctx) return;
    _ctx->ui.admin_cutter_rpm = data["rpm"] | 0;
    _ctx->ui.admin_cutter_current = data["current"] | 0.0f;
    _ctx->ui.admin_cutter_status = data["status"] | 0;
    _ctx->ui.dirtyFlags |= DF_DIAG;
}

void CommRS485::sendResponse() {
    StaticJsonDocument<512> doc;
    
    // Convert current UI mode to page string
    if (_ctx) {
        if (_ctx->ui.curMode == MODE_PRODUCTION) {
            _currentPage = "dashboard";
        } else if (_ctx->ui.curMode == MODE_CONFIGURATION) {
            // Mapping these roughly, Admin tabs have their own submodes or we can assume it based on current tab
            // In a real scenario, UIManager should update _currentPage more explicitly.
            _currentPage = "admin_encoder"; // fallback/placeholder
        }
    }
    
    doc["current_page"] = _currentPage;
    
    JsonArray events = doc.createNestedArray("events");
    for (const auto& ev : _eventQueue) {
        JsonObject obj = events.createNestedObject();
        obj["cmd"] = ev.cmd;
        if (ev.params != -1) {
            obj["params"] = ev.params;
        }
    }
    _eventQueue.clear(); // Consume events
    
    String jsonStr;
    serializeJson(doc, jsonStr);
    
    uint8_t crc = calculateCRC8(jsonStr.c_str(), jsonStr.length());
    
    String outStr = "$" + jsonStr + "*";
    if (crc < 0x10) outStr += "0"; // Pad zero
    outStr += String(crc, HEX);
    outStr += "\n";
    outStr.toUpperCase(); // Optional, but usually CRC hex is uppercase
    
    
    _serial->print(outStr);
    addLog(outStr, true); // Log raw TX line
}

void CommRS485::addLog(const String& line, bool isTX) {
    String prefix = isTX ? "> " : "< ";
    
    // 1. ASCII Column
    String cleanLine = line;
    cleanLine.trim();
    String asciiEntry = prefix + cleanLine;
    
    _logBufferAscii.push_back(asciiEntry);
    if (_logBufferAscii.size() > 10) _logBufferAscii.erase(_logBufferAscii.begin());

    // 2. HEX Column
    String hexEntry = prefix;
    for (size_t i = 0; i < line.length() && i < 16; i++) {
        char buf[4];
        sprintf(buf, "%02X ", (uint8_t)line[i]);
        hexEntry += buf;
    }
    if (line.length() > 16) hexEntry += "..";
    hexEntry.trim();

    _logBufferHex.push_back(hexEntry);
    if (_logBufferHex.size() > 10) _logBufferHex.erase(_logBufferHex.begin());
}
