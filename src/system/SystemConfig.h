#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

/**
 * @file SystemConfig.h
 * @brief 系统全局业务配置常量
 */

// --- 通讯协议配置 ---
#define COMM_WATCHDOG_TIMEOUT_MS    1000  // 通讯超时判定时间 (1s)
#define COMM_PACKET_MAX_LEN         512   // JSON 数据包最大长度

// --- 界面交互配置 ---
#define UI_REFRESH_INTERVAL_MS      33    // 界面刷新周期 (约 30 FPS)
#define MSG_BOX_DURATION_MS         2000  // 提示框自动消失时间

#endif // SYSTEM_CONFIG_H
