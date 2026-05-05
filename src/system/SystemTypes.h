#ifndef SYSTEM_TYPES_H
#define SYSTEM_TYPES_H

#include <Arduino.h>

/**
 * @brief 系统全局运行模式
 */
enum OperationMode {
    MODE_PRODUCTION,    // 生产模式 (Dashboard)
    MODE_OUTLET_CONFIG, // 参数配置 (Config)
    MODE_DIAGNOSTICS,   // 系统维护 (Admin/Diag)
    MODE_ABOUT          // 关于界面
};

/**
 * @brief 系统业务状态
 */
enum SystemStatus {
    SYS_INIT,          // 初始化中
    SYS_READY,         // 准备就绪
    SYS_ERROR          // 故障状态
};

/**
 * @brief UI 同步脏标记位掩码
 */
enum DirtyFlag : uint32_t {
    DF_NONE          = 0x00000000,
    DF_SYS_STATUS    = 0x00000001, // 系统业务状态
    DF_OP_MODE       = 0x00000002, // 运行模式切换
    DF_LIVE_DATA     = 0x00000010, // 实时生产数据 (速度, 产量等)
    DF_DIAG          = 0x00000100, // 诊断信息 (编码器, 激光等)
    DF_PROGRESS      = 0x00000040, // 动作进度
    DF_ALL           = 0xFFFFFFFF
};

/**
 * @brief 核心业务状态 (逻辑层)
 */
struct SystemState {
    SystemStatus sysStatus;       
    char         statusText[32];  
    uint32_t     dirtyFlags;      
};


/**
 * @brief UI 渲染快照 (由 uiLoop 定期从系统上下文同步)
 */
struct UISnapshot {
    OperationMode curMode;
    SystemStatus  sysStatus;
    char          statusText[32];
    uint32_t      frame_counter;        // 帧序列计数 (代表通讯活跃度)
    
    // --- 生产数据 (来自 Master RS485 JSON) ---
    float         dashboard_speed;
    float         dashboard_yield;
    float         dashboard_capacity;
    float         dashboard_diameter;   // 芦笋直径 (毫米)

    // --- 诊断数据 (来自 Master RS485 JSON) ---
    int32_t       diag_encoder_pulses;
    float         diag_encoder_velocity;
    int8_t        diag_encoder_status;
    int32_t       diag_encoder_raw;       // 原始值
    int32_t       diag_encoder_corrected; // 修正值
    int32_t       diag_encoder_logic;     // 逻辑值
    int32_t       diag_encoder_zero_count; // 归零次数
    int32_t       diag_encoder_zero_correct; // 正确次数
    int32_t       diag_encoder_zero_total;   // 总次数

    float         diag_laser_distance;
    float         diag_laser_intensity;
    int8_t        diag_laser_status;
    #define NUM_SCAN_POINTS 4
    uint8_t       diag_laser_states;      // 激光当前状态位掩码
    uint8_t       diag_laser_history[NUM_SCAN_POINTS][25]; // 激光历史数据 (200 bits = 25 bytes)
    
    // --- 下料口配置 (8个出口) ---
    struct OutletConfig {
        float minDiameter;
        float maxDiameter;
        uint8_t lengthMask; // Bit 0:S, 1:M, 2:L
        uint8_t state;      // 0: Closed, 1: Open
    } outlets[8];

    // --- 状态与进度 ---
    uint8_t       diag_comm_log_count;
    char          diag_comm_log_hex[10][128]; 
    char          diag_comm_log_ascii[10][64];

    uint8_t       diag_page_id;        // 0=encoder, 1=laser, 2=outlets, 3=comm
    
    uint32_t      last_comm_time;       // 最后一次成功通讯的时间戳
    uint32_t      dirtyFlags;           // 脏标记位掩码
};

#endif
