#ifndef SYSTEM_TYPES_H
#define SYSTEM_TYPES_H

#include <Arduino.h>

/**
 * @brief 系统全局运行模式
 */
enum OperationMode {
    MODE_PRODUCTION,    // 生产模式 (Dashboard)
    MODE_CONFIGURATION, // 配置模式 (Admin)
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
    
    // --- 生产数据 (来自 Master RS485 JSON) ---
    float         dashboard_speed;
    float         dashboard_yield;
    float         dashboard_capacity;
    float         dashboard_diameter;   // 芦笋直径 (毫米)

    // --- 诊断数据 (来自 Master RS485 JSON) ---
    int32_t       admin_encoder_pulses;
    float         admin_encoder_velocity;
    int8_t        admin_encoder_status;

    float         admin_laser_distance;
    float         admin_laser_intensity;
    int8_t        admin_laser_status;

    int32_t       admin_cutter_rpm;
    float         admin_cutter_current;
    int8_t        admin_cutter_status;

    // --- 状态与进度 ---
    uint8_t       admin_comm_log_count;
    char          admin_comm_log_hex[10][128]; 
    char          admin_comm_log_ascii[10][64];

    uint32_t      last_comm_time;       // 最后一次成功通讯的时间戳
    uint32_t      dirtyFlags;           // 脏标记位掩码
};

#endif
