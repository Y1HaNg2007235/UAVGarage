#ifndef __TJCUSARTHMI_H__
#define __TJCUSARTHMI_H__

#include <stdio.h>
// 建议在此处包含主头文件，确保 UART_HandleTypeDef 类型可用
// #include "main.h" 
// #include "stm32h7xx_hal.h" 

/**
	打印到屏幕串口
*/

// 1. 修改为使用 UART7
#define TJC_UART huart7
#define TJC_UART_INS UART7

// 2. 补充中断号定义（如果你的 .c 文件中有用到中断相关配置，这行非常关键）
#define TJC_UART_IRQn UART7_IRQn 

extern UART_HandleTypeDef huart7;



// --- 下面这部分是关键 ---
#ifdef __cplusplus
extern "C" {
#endif

// 变量声明
extern volatile uint8_t RxBuffer[1];
extern volatile uint32_t msTicks;

// 函数声明
void tjc_send_string(char* str);
void tjc_send_txt(char* objname, char* attribute, char* txt);
void tjc_send_val(char* objname, char* attribute, int val);
void tjc_send_nstring(char* str, unsigned char str_length);
void initRingBuffer(void);
void write1ByteToRingBuffer(uint8_t data);
void deleteRingBuffer(uint16_t size);
uint16_t getRingBufferLength(void);
uint8_t read1ByteFromRingBuffer(uint16_t position);

#ifdef __cplusplus
}
#endif




#define RINGBUFFER_LEN	(500)     //定义最大接收字节数 500

#define usize getRingBufferLength()
#define code_c() initRingBuffer()
#define udelete(x) deleteRingBuffer(x)
#define u(x) read1ByteFromRingBuffer(x)


#endif