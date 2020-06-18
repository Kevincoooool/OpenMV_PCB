/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_USART_H
#define __BSP_USART_H

/* Includes ------------------------------------------------------------------*/
#include "bsp_mcu.h"
#include "ringbuffer.h"
#include "xprintf.h"
/* Private define ------------------------------------------------------------*/
#define USE_USART3
#define DEBUG_SIZE       128

#ifdef USE_USART3 
    #define UART3_BAUD              115200
    #define USART3_RECV_SIZE        128
    extern ring_buffer_t            g_tUsart3RecvFiFo;
#endif
void UsartIsr(UART_HandleTypeDef *huartx,ring_buffer_t *RecvFiFo);
void UsartPrintf(UART_HandleTypeDef *huartx ,const char *fmt, ...) ;

#endif /* __BSP_USART_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

