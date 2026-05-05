# 芦笋分拣系统 - 主从通讯协议 (V1.0)

## 1. 物理层
*   **接口**: RS485 (半双工)
*   **参数**: 115200 Baud, 8N2

## 2. 链路层帧格式
所有数据包均采用 ASCII 格式，外层封装起始符与校验位：
`$ <JSON_PAYLOAD> * <CRC8_HEX> \n`

| 标识符 | 含义 | 长度 | 备注 |
| :--- | :--- | :--- | :--- |
| `$` | 帧起始符 | 1 Byte | |
| `JSON_PAYLOAD` | 业务数据 | 变长 | 标准 JSON 格式 |
| `*` | 校验分隔符 | 1 Byte | |
| `CRC8_HEX` | 校验和 | 2 Bytes | 基于 JSON_PAYLOAD 的 CRC8 (16进制大写) |
| `\n` | 帧结束符 | 1 Byte | |

---

## 3. 业务数据项定义

### 3.1 主机 -> 从机 (周期性上报 / 状态同步)
**Type**: `dashboard` (用于刷新监控页面)

| 字段 (JSON Key) | 类型 | 物理含义 | 备注 |
| :--- | :--- | :--- | :--- |
| `type` | string | 数据包类型 | 固定为 "dashboard" |
| `data.frame_counter` | uint32 | 帧序列计数 | 每次发送自增，反映通讯活跃度与帧率 |
| `data.speed` | float | 当前分拣速度 | 单位：根/秒 |
| `data.yield` | int | 累计产量 | 当次生产循环的识别总数 |
| `data.capacity` | int | 实时产能 | 单位：根/小时 |
| `data.diameter` | float | 最新物料直径 | 单位：mm |

**示例**: `{"type":"dashboard","data":{"up_counter":123,"speed":2.5,"yield":5000,"capacity":9000,"diameter":18.5}}`

---

### 3.2 从机 -> 主机 (交互反馈 / 事件上报)
**Type**: `ack` (在收到主机数据后的反馈包中携带)

| 字段 (JSON Key) | 类型 | 物理含义 | 备注 |
| :--- | :--- | :--- | :--- |
| `type` | string | 数据包类型 | 固定为 "ack" |
| `current_page` | string | 当前显示页面 | 如 "dashboard", "admin", "config" |
| `events` | array | 待处理事件队列 | 包含用户在屏幕上的点击动作 |
| `events[].cmd` | string | 指令名称 | 如 "tare" (置零), "start" (启动) |
| `events[].params` | int | 指令参数 | 如设置的具体数值或索引 |

**示例**: `{"type":"ack","current_page":"dashboard","events":[{"cmd":"tare","params":0}]}`

---

## 4. 故障处理机制
1.  **校验失败**: 若 CRC 校验不通过，接收端直接丢弃该包，不产生 `ack`。
2.  **通讯超时**:
    *   从机若 1s 内未收到任何 `$ ... \n` 帧，UI 通讯 LED 转为红色。
    *   主机若在发送后 100ms 内未收到 `ack`，标记该次通讯失败，重试或记录日志。
