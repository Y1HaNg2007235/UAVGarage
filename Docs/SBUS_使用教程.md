# S.BUS 接线、遥控设置与电机操作

本工程采用 MC02、GM6020 与 M2006/C610；以用户提供的能正常转动的 UAVGarage 为基线。此次没有修改原电机 ID、CAN、PID、供电或系统时钟。原工程两台电机 ID 都是 1，以这份工程实际设置为准。

## 1. 接收机接线

| 接收机端 | MC02 三针 S.BUS 口 |
|---|---|
| S.BUS 信号输出 | S |
| 接收机电源正极 | 5VS（手册电路网络为 VCC_5VS） |
| 接收机 GND | GND |

按针脚标识识别，不能只凭线色或插头方向。三针口的 S 经板上 Q1 反相器后连接 UART5_RX / PD2，已经将 S.BUS 反相波形转换为普通 UART 极性。**不要再打开软件 RXINV。**

手册同页表格把 S 写为 PC02，但电路图和最后一页管脚图均标 UART5_RX(PD02)，本次依据后两者配置。接收机已使用此三针口时，无需改接 UART4，也无需另接 UART5 TX。

接收机型号现已确认是 FS-A8S；当前输出模式还需实物确认。组合接口标有 i-BUS/S.BUS，并不代表接收机当前就在 S.BUS 模式。

## 2. 遥控器和接收机设置

1. 完成 i6 与接收机对频，确认接收机已连接。
2. 将接收机组合输出明确设置为 **S.BUS**，不要保留在 i-BUS 或 PPM。切换方法因接收机型号/固件不同，依对应说明书；部分设备在接收机端切换，并不一定在 i6 上有统一菜单。
3. 默认按照常见 Mode 2：左杆前后是 CH3，右杆左右是 CH1。用 i6 通道显示界面核对，或按第 4 节在 Watch 查看原始值。关闭不需要的通道混控，相关通道微调置中。
4. 左杆前后若是油门杆，不会松手自动回中。**将它手动放到中间，不能把最低位置当成停止。** 最低位置对应反向转动请求。
5. 配置接收机失控行为：至少使这两个通道回到中位，并检查 S.BUS failsafe 标志是否正确。接收机若在无线失联后持续输出最后的通道且不置失控标志，单靠串口有帧无法区分这种状态。

接收设置为 100000 baud / 8E2。代码中使用 UART_WORDLENGTH_9B + 偶校验，因为 STM32 的字长包含校验位。

## 3. 编译、下载和操作

1. 解压整个工程，打开 MDK-ARM/UAVGarage.uvprojx，确认打开的是本 S.BUS 副本。
2. 使用原来的 Arm Compiler 5.06 update 7，点击 Rebuild。正常结果为 0 Error、0 Warning。
3. 使用你原来能成功下载的调试器设置，点击 Download。下载或重启前让两根相关摇杆置中，并留出电机可转动空间。
4. 给开发板动力输入供电，打开已对频遥控器并确认置中，运行程序。进入 Keil 调试时点击 Run（F5）。
5. 首次有效 S.BUS 和电机反馈到达后，遥控层从当前角度接管目标。前推/后拉左杆，GM6020 向相反方向连续转动；右推/左推右杆，M2006 向相反方向连续转动。实际正方向取决于电机安装方向，可按第 5 节反转。
6. 回到中位死区时，锁定回中时采到的位置作为目标，现有位置 PID 将电机稳定在该位置附近。机械惯性和原 PID 决定实际停稳过程，不是瞬时机械刹车。

本版不需要旧工程中的 g_command.commit，也不默认执行 +90°。现在的对象是 robot 和 remoteControl。

## 4. Watch 观察和排障

添加以下变量，开启运行中窗口更新，并保持 CPU Run：

```text
remoteControl
remoteControl.channels[2]
remoteControl.channels[0]
remoteControl.receiver.rxBytes
remoteControl.receiver.uartErrors
remoteControl.receiver.lastUartError
robot.battRotate.targetAngleDeg
robot.battPush.targetAngleDeg
robot.battRotate.currentAngleDeg
robot.battPush.currentAngleDeg
robot.battRotate.motor.feedbackValid
robot.battPush.motor.feedbackValid
```

| 变量 | 意义与正常表现 |
|---|---|
| remoteControl.enabled | 默认 1，遥控层接管目标；设 0 后停止遥控写目标，可以恢复原 Watch/USART6 操作 |
| remoteControl.initialized | 串口接收处于就绪状态时为 1；不表示已经对频或取得有效帧 |
| remoteControl.frames | 结构有效的 S.BUS 帧数持续增加；包含带失控标志的帧 |
| remoteControl.online | 已有新鲜有效帧，且 failsafe/frameLost 均未置位时为 1 |
| remoteControl.channels[2] | CH3，默认左杆前后；中位约 992，端点通常约 172～1811 |
| remoteControl.channels[0] | CH1，默认右杆左右；上述范围可因设备而略有不同 |
| remoteControl.gmDemand / m2006Demand | 经过死区和反向处理后的请求，范围 -1～+1；中间应为 0 |
| remoteControl.gmTargetDeg / m2006TargetDeg | 遥控层计算的输出轴角度目标，范围 0～360°，连续转动时正常回绕 |
| remoteControl.frameLost / failsafe | 最近一帧的丢帧/失控标志，正常为 0 |
| remoteControl.receiver.rxBytes | UART5 收到字节的计数 |
| remoteControl.receiver.uartErrors | 串口校验、帧格式等错误次数；正常通信不应持续增长 |
| remoteControl.receiver.queueOverflows | 接收任务来不及消费字节的次数，正常为 0 |
| remoteControl.taskMonitor.loopCount | 遥控任务是否在运行 |

先看左杆前后是否只改变预期的 CH3、右杆左右是否改变 CH1。通道不符时修改集中配置，避免误判为电机故障。

常见情况：

- rxBytes=0：检查接收机供电、对频、信号端口、S/GND、程序是否 Run。
- 旧版 rxBytes=1、uartErrors=1、restarts=0 且都不增长：更新到 RXFix 版，见 [首字节接收修复](SBUS_首字节接收修复.md)。这次修复不更改引脚。
- rxBytes 增长、frames=0 或 uartErrors 持续增加：检查实际输出是不是 S.BUS、线序是否正确。本三针口已硬件反相，不应软件重复反相。
- frames 增长、online=0：检查 frameLost/failsafe、帧是否仍持续更新及接收机设置。
- online=1，但 gmDemand/m2006Demand 不变化：检查通道映射和死区；摇杆位于中位时为 0 是正常的。
- 请求变化、目标不变化：检查 remoteControl.enabled、对应 robot 任务 enabled、CAN 实际反馈是否新鲜。原电机被手动卸载后，遥控层不会强行重新使能它。
- 目标变化、电机不转：使用原工程的 CAN 与 PID 观察变量排查；电机驱动实现沿用原版。

## 5. 调整通道、方向与快慢

集中配置在 [RemoteConfig.h](../Application/Tasks/RemoteConfig.h)：

```cpp
const unsigned GM6020_CHANNEL = 3; // CH3，配置使用从 1 开始的通道号
const unsigned M2006_CHANNEL = 1;  // CH1

// low, center, high, deadband, reversed, maxTargetRateDegS, maxLeadDeg
const StickSettings GM6020_STICK = {172, 992, 1811, 40, false, 45.0f, 15.0f};
const StickSettings M2006_STICK = {172, 992, 1811, 40, false, 60.0f, 15.0f};
```

- 方向反了：只把相应 false 改为 true。
- 想快一点或慢一点：调整 45.0f / 60.0f，单位是输出轴目标度/秒。
- 中位轻微漂移：先检查遥控微调，必要时修改 center 或 deadband。
- 修改配置后 Rebuild 并重新下载。

原代码使用位置 PID，目标每 20ms 推进一次。实际转速不保证严格等于目标推进速度；本次没有新增速度 PID。领先角限制默认 15°，避免卡住时目标积累到半圈以上，触发原有最短路径角度算法的反向追踪。此限制不切断供电。

## 6. 保留原有控制方式

遥控工作时会持续维护目标，直接在 Watch 修改 targetAngleDeg 或发送 USART6 角度命令可能随后被遥控覆盖。

需要使用原方式时，先把 remoteControl.enabled 设为 0，再按照原 README 修改 robot.battRotate.targetAngleDeg / robot.battPush.targetAngleDeg 或发送 USART6 命令。remoteControl.enabled=0 只是交出目标控制权，不等于卸载电机；电机继续保持最后的角度目标。

需要卸载某台电机时使用原变量 robot.battRotate.enabled=0 或 robot.battPush.enabled=0。遥控层不会将它强制设回 1。重新开启遥控时让摇杆置中，避免重新接管后立即转动。

## 7. 无线失联与测试边界

本版收到失控/丢帧标志，或超过 100ms 没有新有效帧时停止目标推进，在下一次目标更新时保持采样位置；不重启 MCU、不切断 XT30。保持依赖原电机反馈和位置控制正常工作，不能替代机械制动。恢复有效信号后按当前摇杆值重新工作。

软件已完成电脑行为测试和完整 Keil 编译，未连接实板。首次需实测通道/中位/正方向、回中保持以及关闭遥控器时接收机的实际输出行为。

## 8. 后续 CubeMX 再生成

IOC 已加入 UART5/PD2、100000、9 位字长、偶校验、2 停止位、RX-only 和 UART5 中断。已交付源文件可直接编译，不需要先重新生成。

以后再生成时保留 USER CODE。CubeMX 可能重写 Keil 分组、生成 UART5 初始化及中断；检查 RemoteControl-SBUS 分组仍包含新增源码，rcProtocolTask 的 RemoteControl_Init/Update 调用仍在，HAL 接收完成回调仍分发到 SbusUartReceiver。新增中断使用常规生成区域，避免同一个 UART5_IRQHandler 被生成两次。
