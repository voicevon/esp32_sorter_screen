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

### 3.3 主机 -> 从机 (编码器诊断同步)
**Type**: `diag_encoder` (用于维护 -> 编码器诊断页面)

| 字段 (JSON Key) | 类型 | 物理含义 | 备注 |
| :--- | :--- | :--- | :--- |
| `type` | string | 数据包类型 | 固定为 "diag_encoder" |
| `data.pulse_count` | int | 编码器原始值 | 同 `raw_val` |
| `data.velocity` | float | 实时速度 | |
| `data.status` | int | 状态掩码 | |
| `data.raw_val` | int | 编码器原始值 | |
| `data.corrected_val` | int | 修正值 | 经过校准系数处理后的值 |
| `data.logic_val` | int | 逻辑相位值 | 0-199 循环逻辑位置 |
| `data.zero_count` | int | 归零触发次数 | 经历过多少次 Z 信号或软件归零 |
| `data.zero_correct` | int | 归零正确计数 | |
| `data.zero_total` | int | 经历的编码器信号总数 (预计) | |

**示例**: `{"type":"diag_encoder","data":{"pulse_count":12345,"velocity":20.5,"status":1,"raw_val":12345,"corrected_val":12340,"logic_val":50,"zero_count":10,"zero_correct":9,"zero_total":10}}`

### 3.4 主机 -> 从机 (激光扫描仪诊断同步)
**Type**: `diag_laser` (用于激光扫描仪诊断页面)

| 字段 (JSON Key) | 类型 | 物理含义 | 备注 |
| :--- | :--- | :--- | :--- |
| `type` | string | 数据包类型 | 固定为 "diag_laser" |
| `data.states` | int | 5 路激光当前状态 | 位掩码 (Bit 0-4), 1=遮挡/高, 0=畅通/低 |
| `data.history_p1` | string | 1 号激光历史数据 | 200 bits 压缩为 Hex 字符串 (50 chars) |
| `data.history_p2` | string | 2 号激光历史数据 | 同上 |
| `data.history_p3` | string | 3 号激光历史数据 | 同上 |
| `data.history_p4` | string | 4 号激光历史数据 | 同上 |
| `data.history_p5` | string | 5 号激光历史数据 | 同上 |

**示例**: `{"type":"diag_laser","data":{"states":1,"history_p1":"ABC...","history_p2":"...","history_p3":"...","history_p4":"...","history_p5":"..."}}`

---

### 3.5 主机 -> 从机 (下料口诊断/配置同步)
**Type**: `diag_outlets` 或 `config_outlets`

| 字段 (JSON Key) | 类型 | 物理含义 | 备注 |
| :--- | :--- | :--- | :--- |
| `type` | string | 数据包类型 | "diag_outlets" 或 "config_outlets" |
| `data[].min` | float | 最小触发直径 | |
| `data[].max` | float | 最大触发直径 | |
| `data[].mask` | int | 长度掩码 | Bit 0:S, 1:M, 2:L (1=选中, 0=未选) |
| `data[].state` | int | 实时位置状态 | 仅在 "diag_outlets" 类型中存在 |

**示例**: `{"type":"diag_outlets","data":[{"min":18.0,"max":20.0,"mask":7}, ...]}`

### 3.6 从机 -> 主机 (配置下发指令)
当用户在屏幕上修改下料口参数时，从机通过 `events` 队列上报。

**Command**: `set_outlet`

| 字段 (JSON Key) | 类型 | 物理含义 | 备注 |
| :--- | :--- | :--- | :--- |
| `cmd` | string | 指令名称 | "set_outlet" |
| `index` | int | 出口索引 | 0-7 |
| `min` | float | 新的最小直径 | |
| `max` | float | 新的最大直径 | |
| `mask` | int | 新的长度位掩码 | |

**示例**: `{"cmd":"set_outlet","index":2,"min":18.5,"max":20.5,"mask":3}`

---

## 4. 故障处理机制
1.  **校验失败**: 若 CRC 校验不通过，接收端直接丢弃该包，不产生 `ack`。
2.  **通讯超时**:
    *   从机若 1s 内未收到任何 `$ ... \n` 帧，UI 通讯 LED 转为红色。
    *   主机若在发送后 100ms 内未收到 `ack`，标记该次通讯失败，重试或记录日志。
