# UAVGarage_AI 电机变量说明文档

> 适用工程：`UAVGarage_AI`（STM32H723VGT6，达妙 MC02）
> 全局对象：`robot`（`garage::Garage` 类实例，定义在 `Application/App.cpp`）
> Watch 窗口里的完整路径 = `robot` + 下面各表「完整变量名」列。

本工程有两个电机控制任务，各走一路 CAN：

| 任务 | 电机 | 电调/驱动 | CAN 总线 | 前缀 |
|---|---|---|---|---|
| BattRotate | GM6020 | 内置驱动 | CAN1（PD0/PD1） | `robot.battRotate` |
| BattPush | M2006 P36 | C610 电调 | CAN2（PB5/PB6） | `robot.battPush` |

可写性标注：

- ✅ **可改**：`volatile` 成员，设计上就是给 Watch / 其他任务在运行时修改的，改完立即生效。
- ⚠️ **不建议改**：`public` 但属于「状态反馈」，改动会被下一轮更新覆盖，或干扰闭环逻辑，只应读取。
- 🔒 **不可改**：`private` 成员，外部（Watch / 其他程序）无法访问，只能通过类方法修改。

---

## 一、电机一：GM6020（任务 BattRotate，CAN1）

### 1.1 任务层 `robot.battRotate`

| 完整变量名 | 类型 | 功能 | 可写性 |
|---|---|---|---|
| `robot.battRotate.targetAngleDeg` | `volatile float` | 目标角度（相对上电零位，0~360°）。改它电机就转到对应角度 | ✅ 可改 |
| `robot.battRotate.currentAngleDeg` | `volatile float` | 当前角度（相对上电零位，0~360°）。实时反映电机位置 | ✅ 可改（改它无意义，下一轮被反馈覆盖） |
| `robot.battRotate.enabled` | `volatile bool` | 位置闭环使能开关。`1` 使能，`0` 卸载（输出恒 0）。上电自动为 1 | ✅ 可改 |

### 1.2 电机层 `robot.battRotate.motor`（类 `Gm6020`）

| 完整变量名 | 类型 | 功能 | 可写性 |
|---|---|---|---|
| `robot.battRotate.motor.motorId` | `uint8_t` | 电机拨码 ID（1~4），决定读哪一路反馈（0x204+ID） | ⚠️ 不建议改 |
| `robot.battRotate.motor.output` | `int16_t` | 当前输出的电压给定（-25000~25000），由位置环 PID 计算 | ⚠️ 不建议改 |
| `robot.battRotate.motor.angleRaw` | `uint16_t` | 编码器原始机械角度（0~8191，对应 0~360°） | ⚠️ 不建议改 |
| `robot.battRotate.motor.angleDegValue` | `float` | 编码器**绝对**角度（度，0~360，非相对零位） | ⚠️ 不建议改 |
| `robot.battRotate.motor.speedRpm` | `int16_t` | 转速（rpm） | ⚠️ 不建议改 |
| `robot.battRotate.motor.torqueCurrent` | `int16_t` | 实际转矩电流 | ⚠️ 不建议改 |
| `robot.battRotate.motor.temperature` | `int8_t` | 电机温度（℃） | ⚠️ 不建议改 |
| `robot.battRotate.motor.feedbackValid` | `bool` | 反馈是否有效。`1` 表示 CAN 收到了电机反馈（排障关键） | ⚠️ 不建议改 |
| `robot.battRotate.motor.feedbackMs` | `uint32_t` | 最近一次收到反馈的 `HAL_GetTick()` 时刻 | ⚠️ 不建议改 |

### 1.3 CAN 层 `robot.battRotate.can`（类 `Gm6020Can`）

| 完整变量名 | 类型 | 功能 | 可写性 |
|---|---|---|---|
| `robot.battRotate.can.ready` | `bool` | CAN1 是否启动成功。`0` 说明滤波器/启动失败 | ⚠️ 不建议改 |

### 1.4 PID 层 `robot.battRotate.pid`（类 `PidController`）

| 完整变量名 | 类型 | 功能 | 可写性 |
|---|---|---|---|
| `robot.battRotate.pid.output` | `float` | PID 实时输出（限幅后），就是最终电压给定 | ⚠️ 不建议改 |

### 1.5 监控层 `robot.battRotate.taskMonitor`（类 `TaskMonitor`）

| 完整变量名 | 类型 | 功能 | 可写性 |
|---|---|---|---|
| `robot.battRotate.taskMonitor.loopCount` | `uint32_t` | 任务循环计数，看任务是否在跑 | ⚠️ 不建议改 |
| `robot.battRotate.taskMonitor.cpuPercent` | `float` | 本任务 CPU 占用率（%） | ⚠️ 不建议改 |
| `robot.battRotate.taskMonitor.minFreeStackBytes` | `uint32_t` | 任务栈历史最少剩余字节（查栈溢出） | ⚠️ 不建议改 |
| `robot.battRotate.taskMonitor.lastLoopTick` | `TickType_t` | 最近一次 `update()` 的系统节拍 | ⚠️ 不建议改 |
| `robot.battRotate.taskMonitor.cpuValid` | `bool` | 最近一次 CPU 统计是否有效 | ⚠️ 不建议改 |

---

## 二、电机二：C610 + M2006（任务 BattPush，CAN2）

### 2.1 任务层 `robot.battPush`

| 完整变量名 | 类型 | 功能 | 可写性 |
|---|---|---|---|
| `robot.battPush.targetAngleDeg` | `volatile float` | 目标角度（输出轴，相对上电零位，0~360°） | ✅ 可改 |
| `robot.battPush.currentAngleDeg` | `volatile float` | 当前输出轴角度（相对上电零位，0~360°） | ✅ 可改（改它无意义） |
| `robot.battPush.enabled` | `volatile bool` | 使能开关。`1` 使能，`0` 卸载。上电自动为 1 | ✅ 可改 |

### 2.2 电机层 `robot.battPush.motor`（类 `M2006`）

| 完整变量名 | 类型 | 功能 | 可写性 |
|---|---|---|---|
| `robot.battPush.motor.motorId` | `uint8_t` | 电调 ID（1~4，SET 键设置），决定读哪一路反馈（0x200+ID） | ⚠️ 不建议改 |
| `robot.battPush.motor.output` | `int16_t` | 当前输出的电流给定（-10000~10000，对应 -10A~10A） | ⚠️ 不建议改 |
| `robot.battPush.motor.angleRaw` | `uint16_t` | 转子**单圈**角度（0~8191），转子转一圈回绕一次，会反复跳 | ⚠️ 不建议改 |
| `robot.battPush.motor.angleDegValue` | `float` | 输出轴角度（度，0~360，按 36:1 减速多圈累计换算，相对零位） | ⚠️ 不建议改 |
| `robot.battPush.motor.speedRpm` | `int16_t` | 转子转速（rpm） | ⚠️ 不建议改 |
| `robot.battPush.motor.torqueCurrent` | `int16_t` | 实际输出转矩 | ⚠️ 不建议改 |
| `robot.battPush.motor.feedbackValid` | `bool` | 反馈是否有效。`1` 表示 CAN2 收到电调反馈 | ⚠️ 不建议改 |
| `robot.battPush.motor.feedbackMs` | `uint32_t` | 最近一次收到反馈的时刻 | ⚠️ 不建议改 |

> 注：M2006/C610 反馈帧里没有温度，故 BattPush 无 `temperature` 变量。

### 2.3 CAN 层 `robot.battPush.can`（类 `C610Can`）

| 完整变量名 | 类型 | 功能 | 可写性 |
|---|---|---|---|
| `robot.battPush.can.ready` | `bool` | CAN2 是否启动成功 | ⚠️ 不建议改 |

### 2.4 PID 层 `robot.battPush.pid`

| 完整变量名 | 类型 | 功能 | 可写性 |
|---|---|---|---|
| `robot.battPush.pid.output` | `float` | PID 实时输出（限幅后），就是最终电流给定 | ⚠️ 不建议改 |

### 2.5 监控层 `robot.battPush.taskMonitor`

与 `robot.battRotate.taskMonitor` 完全相同的 5 个成员：`loopCount` / `cpuPercent` / `minFreeStackBytes` / `lastLoopTick` / `cpuValid`。

---

## 三、🔒 不可访问的 private 成员（说明）

以下成员是 `private`，Watch 和其他程序**无法直接读写**，只能通过类方法间接影响，列出来供理解内部实现：

| 类 | private 成员 | 作用 |
|---|---|---|
| `PidController` | `_kp / _ki / _kd` | PID 增益，通过 `setGains()` 设置 |
| `PidController` | `_outputLimit / _integralLimit` | 输出/积分限幅，通过 `setOutputLimit()/setIntegralLimit()` 设置 |
| `PidController` | `_integral / _lastError / _first` | PID 内部状态 |
| `BattRotateTask` | `_zeroAngleRaw / _zeroed` | 上电零位标定状态 |
| `BattRotateTask` | `_appliedAngle` | 记录已应用目标，检测 Watch 是否改了角度 |
| `BattRotateTask` | `_serial` | 串口指令接收器（USART6） |
| `BattPushTask` | `_appliedAngle / _lastUpdateMs` | 同上 |
| `M2006` | `_rotorAccum / _lastRotorRaw / _first` | 多圈累计状态 |
| `Gm6020Can` / `C610Can` | `_output[] / _feedback[]` | 各电机给定值 / 反馈缓存 |
| `Gm6020Can` / `C610Can` | `_hfdcan / _lastSendMs` | CAN 句柄 / 发送节流时间 |

---

## 四、实战速查

| 想干什么 | 用哪个变量 |
|---|---|
| 让电机转到 90° | `robot.battRotate.targetAngleDeg = 90`（或 `robot.battPush.targetAngleDeg = 90`） |
| 停机卸载 | `robot.battRotate.enabled = 0` |
| 查 CAN 通没通 | `robot.battRotate.can.ready` + `robot.battRotate.motor.feedbackValid` |
| 查当前转到哪了 | `robot.battRotate.currentAngleDeg` |
| 查 PID 有没有输出 | `robot.battRotate.pid.output` |
| 查任务活没活 | `robot.battRotate.taskMonitor.loopCount` |
