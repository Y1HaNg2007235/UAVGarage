#ifndef TEST_HAL_H
#define TEST_HAL_H
#include <stdint.h>
struct UART_HandleTypeDef { uint32_t ErrorCode; };
enum HAL_StatusTypeDef { HAL_OK, HAL_ERROR, HAL_BUSY };
#define HAL_UART_ERROR_NONE 0U
HAL_StatusTypeDef HAL_UART_Receive_IT(UART_HandleTypeDef*, uint8_t*, unsigned);
HAL_StatusTypeDef HAL_UART_AbortReceive(UART_HandleTypeDef*);
uint32_t HAL_GetTick();
inline void __DMB() {}
#endif
