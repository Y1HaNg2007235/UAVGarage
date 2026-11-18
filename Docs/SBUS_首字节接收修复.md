# 首字节接收停止修复（RXFix，2026-11-15）

用户报告：接收字节数为 1、UART 错误次数为 1、恢复次数为 0，持续运行也不增长。电脑测试已在旧接收模块中复现相同组合。此处是确认的软件缺陷，实板上是否还存在接收机模式、接线等问题，需要更新后观察。

## 原因与修复

原 `SbusUartReceiver::init()` 在 `HAL_UART_Receive_IT()` 返回后才写入 `ready` 与 `_restart`。HAL 开启接收中断后、返回之前，就可能接到第一个字节或错误中断。如果该中断要求恢复接收，随后初始化中的赋值会覆盖这个请求，使状态显示 ready=1，但 UART 已经停止接收，恢复任务也没有被触发。

修正版先设置初始状态，再开启接收；HAL 返回成功时不覆盖中断已经更新的状态。若 HAL 返回失败，才标记待恢复。UART 错误后的恢复只重新开启串口接收，不重启板卡或电机。

本次改动的嵌入式源码只有新增模块 `Libraries/Remote/SbusUartReceiver.cpp`；新增测试 `Tests/sbus_uart_tests.cpp` 补充初始化中断抢占场景。UART5/PD2、反相设置、CAN、PID、电机 ID、供电和原任务结构均保持交付首版的内容。

## 复现与验证

测试用模拟 HAL，在接收初始化返回之前触发一次帧格式错误。错误码 4 是测试注入值，不代表已经读到了用户实板的错误码。

| 测试阶段 | rxBytes | uartErrors | restarts | 接收结果 |
|---|---:|---:|---:|---|
| 修复前，任务轮询后 | 1 | 1 | 0 | 接收未重新开启 |
| 修复后，任务轮询后 | 1 | 1 | 1 | 接收已重新开启 |

扩展 UART 测试的 589 次断言通过，包含恢复后完整帧仍能解码的检查。完整 Keil Rebuild 为 0 错误、0 警告。对比原始工程仍只有先前授权的 9 个原有文件变化；本次没有额外修改原有文件。未连接实物进行验证。

复现记录在 `Validation/startup-race-before.txt` 与 `Validation/startup-race-after.txt`；前者的 FAIL 是特意复现旧版问题得到的预期失败，不是本修正版的构建失败。

## 更新到板卡

1. 退出 Keil 调试，解压带 `RXFix` 的完整工程包到独立目录。
2. 打开其中 `UAVGarage_SBUS/MDK-ARM/UAVGarage.uvprojx`。不要沿用旧版 AXF 调试新固件。
3. Rebuild，确认 0 Error、0 Warning，然后 Download。
4. 两根控制摇杆置中，进入 Debug，按 F5 Run。不要在逐字节串口中断里停断点观察连续数据。
5. 在 Watch 中添加下列完整变量。也可以直接添加 `remoteControl` 并展开 `receiver`，避免名称输入错误。

```text
remoteControl.receiver.rxBytes
remoteControl.receiver.uartErrors
remoteControl.receiver.lastUartError
remoteControl.receiver.restarts
remoteControl.frames
remoteControl.online
remoteControl.channels[2]
remoteControl.channels[0]
```

`lastUartError` 确实存在于本修正版的头文件和调试信息中。若 Keil 仍显示找不到该成员，请确认已打开新工程并 Rebuild、Download 后重新进入 Debug。

正确 S.BUS 输出下，即使摇杆不动，rxBytes 和 frames 也应持续增长，online=1。出现一次启动错误后 restarts 增加一次并恢复正常，是允许的现象。

如果更新后错误次数与恢复次数不断增加、frames 仍为 0，请继续检查接收机是否实际切换到 S.BUS、输出线序与极性。常见 lastUartError 位：0x01 偶校验错误、0x02 噪声、0x04 帧格式错误、0x08 接收溢出；多个错误可同时置位。

已确认接收机型号为 FS-A8S，但尚未确认其当前输出模式。组合接口标有 i-BUS/S.BUS，并不能证明正在输出 S.BUS；按该接收机说明确认输出模式。本版保持板载三针 S.BUS 接口、UART5/PD2 和硬件反相方案，不需要因为这次初始化缺陷更改引脚。
