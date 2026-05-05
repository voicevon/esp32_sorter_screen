#ifndef PIN_DEFINITION_H
#define PIN_DEFINITION_H

/* 
 * ESP32-S3 Sorter Screen - 硬件引脚与寄存器统一定义
 * 本文件管理业务层的引脚常量。LCD 和 Touch 的引脚归 TouchScreen 管。
 */

// --- RS485 通信 (Serial 1/2) ---
// 根据 esp32_s3_lcd 的通信验证基础，RX:15，TX:16
#define PIN_RS485_RX     15
#define PIN_RS485_TX     16
#define PIN_RS485_TX_EN  -1    // 硬件自动收发切换，无需使能引脚
#define RS485_TX_ENABLE  HIGH 
#define RS485_RX_ENABLE  LOW  
#define RS485_BAUD       115200



#endif // PIN_DEFINITION_H
