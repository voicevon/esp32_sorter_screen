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

void CommRS485::pushEvent(const String& cmd, int index, int state) {
    _eventQueue.push_back({cmd, index, state});
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

    /* 
    Serial.println("[CommRS485] Received signal frame from master:");
    serializeJsonPretty(doc, Serial);
    Serial.println();
    Serial.printf("[RS485] << RX: %s\n", jsonStr.c_str());
    */
    
    String type = doc["type"] | "unknown";
    JsonObject data = doc["data"];
    
    if (type == "dashboard") {
        handleDashboard(data);
    } else if (type == "diag_encoder") {
        handleAdminEncoder(data);
    } else if (type == "diag_laser") {
        handleAdminLaser(data);
    } else if (type == "diag_outlets" || type == "config_outlets") {
        handleAdminOutlets(doc["data"].as<JsonArray>());
    }
    
    if (_ctx) {
        _ctx->ui.last_comm_time = millis();
    }
    
    // Since we are Slave, we must reply immediately upon receiving a valid request.
    // Add a small delay to allow the Master to switch its RS485 transceiver to RX mode.
    delay(5); 
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
    _ctx->ui.diag_encoder_pulses = data["pulse_count"] | 0;
    _ctx->ui.diag_encoder_velocity = data["velocity"] | 0.0f;
    _ctx->ui.diag_encoder_status = data["status"] | 0;
    
    _ctx->ui.diag_encoder_raw = data["raw_val"] | 0;
    _ctx->ui.diag_encoder_corrected = data["corrected_val"] | 0;
    _ctx->ui.diag_encoder_logic = data["logic_val"] | 0;
    _ctx->ui.diag_encoder_zero_count = data["zero_count"] | 0;
    _ctx->ui.diag_encoder_zero_correct = data["zero_correct"] | 0;
    _ctx->ui.diag_encoder_zero_total = data["zero_total"] | 0;
    
    _ctx->ui.dirtyFlags |= DF_DIAG;
}

void CommRS485::handleAdminLaser(JsonObject data) {
    if (!_ctx) return;
    
    // 1. 解析当前状态位掩码
    _ctx->ui.diag_laser_states = data["states"] | 0;

    // 2. 解析历史数据
    const char* keys[] = {"history_p1", "history_p2", "history_p3", "history_p4"};
    for (int i = 0; i < NUM_SCAN_POINTS; i++) {
        const char* hex = data[keys[i]] | "";
        if (strlen(hex) == 50) { // 25 bytes * 2 chars/byte = 50 chars
            for (int j = 0; j < 25; j++) {
                char tmp[3] = { hex[j * 2], hex[j * 2 + 1], 0 };
                _ctx->ui.diag_laser_history[i][j] = (uint8_t)strtol(tmp, NULL, 16);
            }
        }
    }

    _ctx->ui.dirtyFlags |= DF_DIAG;
}

void CommRS485::handleAdminOutlets(JsonArray data) {
    if (!_ctx) return;
    int i = 0;
    for (JsonObject obj : data) {
        if (i >= 8) break;
        _ctx->ui.outlets[i].minDiameter = obj["min"] | 0.0f;
        _ctx->ui.outlets[i].maxDiameter = obj["max"] | 0.0f;
        _ctx->ui.outlets[i].lengthMask  = obj["mask"] | 0;
        _ctx->ui.outlets[i].state       = obj["state"] | 0;
        i++;
    }
    _ctx->ui.dirtyFlags |= DF_LIVE_DATA; // Refresh UI
}

void CommRS485::sendResponse() {
    StaticJsonDocument<512> doc;
    
    // Convert current UI mode to page string
    if (_ctx) {
        if (_ctx->ui.curMode == MODE_PRODUCTION) {
            _currentPage = "dashboard";
        } else if (_ctx->ui.curMode == MODE_OUTLET_CONFIG) {
            _currentPage = "config_outlets";
        } else if (_ctx->ui.curMode == MODE_DIAGNOSTICS) {
            switch (_ctx->ui.diag_page_id) {
                case 0: _currentPage = "diag_encoder"; break;
                case 1: _currentPage = "diag_laser"; break;
                case 2: _currentPage = "diag_outlets"; break; // 现在代表诊断
                case 3: _currentPage = "diag_comm"; break;
                default: _currentPage = "diag_encoder"; break;
            }
        } else if (_ctx->ui.curMode == MODE_ABOUT) {
            _currentPage = "about";
        }
    }
    
    doc["current_page"] = _currentPage;
    
    JsonArray events = doc.createNestedArray("events");
    for (const auto& ev : _eventQueue) {
        JsonObject obj = events.createNestedObject();
        obj["cmd"] = ev.cmd;
        if (ev.cmd == "set_outlet" && _ctx) {
            int index = ev.index;
            obj["index"] = index;
            obj["min"] = _ctx->ui.outlets[index].minDiameter;
            obj["max"] = _ctx->ui.outlets[index].maxDiameter;
            obj["mask"] = _ctx->ui.outlets[index].lengthMask;
        } else {
            if (ev.index != -1) obj["index"] = ev.index;
            if (ev.state != -1) obj["state"] = ev.state;
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
    
    _serial->print(outStr);
    addLog(outStr, true); // Log raw TX line
    // Serial.printf("[RS485] >> TX: %s", outStr.c_str()); 
    // Serial.println("[CommRS485] Response sent to master.");
}

void CommRS485::addLog(const String& line, bool isTX) {
    if (!_ctx) return;
    String prefix = isTX ? "> " : "< ";
    
    // 1. ASCII Column
    String cleanLine = line;
    cleanLine.trim();
    String asciiEntry = prefix + cleanLine;
    
    // 2. HEX Column (提升作用域)
    String hexEntry = prefix;
    for (size_t i = 0; i < line.length() && i < 16; i++) {
        char buf[4];
        sprintf(buf, "%02X ", (uint8_t)line[i]);
        hexEntry += buf;
    }
    if (line.length() > 16) hexEntry += "..";
    hexEntry.trim();
    
    // 存储到上下文供 UI 展示
    int count = _ctx->ui.diag_comm_log_count;
    if (count < 10) {
        strncpy(_ctx->ui.diag_comm_log_ascii[count], asciiEntry.c_str(), 63);
        _ctx->ui.diag_comm_log_ascii[count][63] = 0;
        strncpy(_ctx->ui.diag_comm_log_hex[count], hexEntry.c_str(), 127);
        _ctx->ui.diag_comm_log_hex[count][127] = 0;

        _ctx->ui.diag_comm_log_count++;
    } else {
        // 滚动逻辑
        for (int i = 0; i < 9; i++) {
            strcpy(_ctx->ui.diag_comm_log_ascii[i], _ctx->ui.diag_comm_log_ascii[i+1]);
            strcpy(_ctx->ui.diag_comm_log_hex[i], _ctx->ui.diag_comm_log_hex[i+1]);
        }
        strncpy(_ctx->ui.diag_comm_log_ascii[9], asciiEntry.c_str(), 63);
        _ctx->ui.diag_comm_log_ascii[9][63] = 0;
        strncpy(_ctx->ui.diag_comm_log_hex[9], hexEntry.c_str(), 127); 
        _ctx->ui.diag_comm_log_hex[9][127] = 0;
    }
    
    _ctx->ui.dirtyFlags |= DF_DIAG;
}
