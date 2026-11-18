# S.BUS 接入修改与验证记录

交付日期：2026-11-15，当前版本 RXFix。基线为用户提供的 `UAVGarage-main (1)/UAVGarage-main`，交付在独立副本 `UAVGarage_SBUS` 中。原始目录未写入。

RXFix 只修正新增的 SbusUartReceiver 初始化时序，并扩展新增 UART 测试及更新交付记录；下面列出的 9 个原有文件与首版接入版保持一致。问题复现和更新步骤见 [首字节接收修复](SBUS_首字节接收修复.md)。

## 1. 授权范围与逐文件记录

根据用户对《S.BUS接入修改清单_待确认.md》的确认，仅修改以下 9 个原有文件，其余原有源码与配置保持原样。

| 原有文件 | 必要接入改动 |
|---|---|
| Core/Src/usart.c | 新增 UART5 接收初始化、PD2 复用及中断开关；保留 UART4/USART6 配置 |
| Core/Inc/usart.h | 声明 huart5 和 MX_UART5_Init |
| Core/Src/main.c | 在原串口初始化后调用 MX_UART5_Init |
| Core/Src/stm32h7xx_it.c | 引用 huart5，新增 UART5_IRQHandler 并调用 HAL 处理函数 |
| Core/Inc/stm32h7xx_it.h | 声明 UART5_IRQHandler |
| Core/Src/freertos.c | 在已有 rcProtocolTask 中接入 RemoteControl_Init/Update；不新增或调整原任务优先级 |
| Libraries/Serial/UartLineReceiver.cpp | 已有接收完成回调按 UART5 分发，其他串口保持原处理路径 |
| UAVGarage.ioc | 加入 UART5、PD2 RX、串口格式与中断配置 |
| MDK-ARM/UAVGarage.uvprojx | 新增 RemoteControl-SBUS 编译分组与文件引用 |

逐行差异见 [SBUS_改动.patch](SBUS_改动.patch)，原文件与副本的 SHA256 比对见 [SBUS_文件核对.json](SBUS_文件核对.json)。其中 1710 个原始保留文件均进行核对，副本改动应恰好为上述 9 项；原始目录中这 1710 个文件仍与基线一致。复制时未带入旧编译输出、`.git` 与个人 IDE 布局文件。

电机 ID、CAN、PID 参数和实现、XT30 供电、时钟、原 README 均未更改。本基线中两台电机的 ID 都为 1，不沿用更早独立工程中的 ID=2 设置。

## 2. 新增模块的分工

| 模块 | 职责 |
|---|---|
| Libraries/Remote/SbusDecoder | 独立 C++ 类，解析 S.BUS 25 字节帧、16 个模拟通道与状态位，不依赖 HAL |
| Libraries/Remote/SbusUartReceiver | UART5 中断收字节、环形队列、接收异常恢复，任务中完成解码 |
| Libraries/Remote/StickTarget | 独立 C++ 类，将摇杆偏转转为持续推进的角度目标，处理中位保持、角度回绕 |
| Application/Tasks/RemoteConfig.h | 集中设置通道、中位、死区、反向、推进速度与超时 |
| Application/Tasks/RemoteControl | 接入现有遥控任务，只通过原有公开目标角度字段控制两台电机 |
| Tests 与 Tools/test-sbus.ps1 | 电脑端协议、目标映射和 UART 接收行为测试，不加入嵌入式固件 |

数据路径：S.BUS 三针口 → 板载反相器 → UART5/PD2 → 接收队列与解码 → 摇杆目标换算 → 原有目标角度 → 原有电机任务与 CAN。

延续原工程的模块分层、C++ 类封装、任务分工和集中配置方式。共享串口回调按实例分发；新增遥控层不直接发送 CAN，也不修改 PID 内部状态。

## 3. 新增行为

- 默认 CH3 控制 GM6020，CH1 控制 M2006；需通过 Watch 确认实际遥控通道。
- 偏转越大，角度目标推进越快；回中时保存当时测得的位置，随后保持该目标。左侧油门杆需要手动回中。
- 本次沿用基线的位置 PID，没有另加速度环；实际速度由原 PID 与负载决定。
- 收到失控/丢帧标志或有效帧超时后停止推进目标。已有新鲜电机反馈时，从运动状态切换为当前角度保持；不重启板卡或断开电机电源。
- 不主动重新使能被用户卸载的电机；没有首次有效遥控数据时，不接管原电机目标。
- `remoteControl.enabled=0` 交出目标控制权，恢复原 Watch/USART6 使用方式，不表示卸载电机。
- 目标领先当前角度不超过默认 15°，避免堵转后目标跨过半圈而使原最短路径算法反向追踪。

## 4. 接口依据

依据 MC02 V1.1 手册第 8 页电路图与第 17 页管脚图：三针 S.BUS 的信号经 Q1 反相后到 UART5_RX/PD02。手册表格中的 PC02 与电路图不一致，本版本采用 PD2。

UART5：100000 baud、8 个数据位、偶校验、2 个停止位、仅接收。STM32 HAL 字长配置为 `UART_WORDLENGTH_9B`（包含校验位），PD2 使用 AF8，关闭软件 RXINV；没有配置 UART5 TX 或新 DMA。

## 5. 已完成验证

| 检查 | 结果 |
|---|---|
| 完整 Keil Rebuild，Arm Compiler 5.06 update 7 build 960 | 0 错误、0 警告，生成 AXF/HEX |
| 解码与摇杆目标测试 | 34858 次断言通过，包含系统性通道位枚举，不代表同等数量的独立场景 |
| UART 接收行为测试 | 589 次断言通过，使用模拟 HAL，覆盖接收、队列溢出、错误恢复、回调顺序及初始化期间抢占 |
| 原始文件与副本 SHA256 核对 | 原始目录未变；副本恰好修改授权的 9 个原文件 |
| 接入配置核对 | UART5 引脚与格式、单一中断/回调、IOC 枚举、Keil 文件引用核对通过 |

行为测试覆盖通道提取、半帧与错帧重同步、状态位、超时与计时回绕、死区、方向、速度请求、回中保持、失联保持和角度回绕。解码/目标模块未在 RXFix 中更改，保留此前通过的测试记录；UART 测试在修复后重新运行。测试源码在 `Tests/`；安装 Zig 后可用 `Tools/test-sbus.ps1 -ZigPath <zig.exe 路径>` 重跑。测试运行需要允许本地 PowerShell 脚本执行。

编译、测试和配置核对原始记录在 [Validation](Validation/)；本次编译的固件及校验值在 [Firmware](../Firmware/)。固件不包含电脑端测试。

## 6. 验证边界

没有连接实物、烧录或实测电机，也没有运行 CubeMX 再生成。IOC 参数按安装的 CubeMX UART 配置定义核对，交付源文件可以直接用 Keil 编译。

原 Keil 工程还引用了一个不存在的“新建 文本文档.txt”；它不是编译源码，不影响本次构建。此原有占位引用按约定保留，具体路径记在配置核对记录中。

需要实物确认的内容：接收机确实输出 S.BUS、CH3/CH1 对应指定摇杆、中位和方向正确、运动与回中符合机械负载、接收机断联时输出的标志或通道符合预期。详细步骤见 [使用教程](SBUS_使用教程.md)。
