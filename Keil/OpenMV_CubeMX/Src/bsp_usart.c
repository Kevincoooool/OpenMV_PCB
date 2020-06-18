/* Includes ------------------------------------------------------------------*/
#include "bsp_usart.h"
#include "set.h"

/* Private define ------------------------------------------------------------*/
uint8_t u8PrintfBuffer[DEBUG_SIZE] = {0};

#ifdef USE_USART3 
    uint8_t u8Usart3RecvBuffer[USART3_RECV_SIZE] = {0};
    ring_buffer_t g_tUsart3RecvFiFo = {0,0, 0, 0, USART3_RECV_SIZE, u8Usart3RecvBuffer};
#endif





#ifdef USE_USART3 
void USART3_IRQHandler(void)
{
    UsartIsr(&huart3, &g_tUsart3RecvFiFo);           
    HAL_UART_IRQHandler(&huart3);
}
#endif

/** @brief    供中断服务程序调用，通用串口中断处理函数 */
void UsartIsr(UART_HandleTypeDef *huartx,ring_buffer_t *RecvFiFo)
{
    uint8_t ch;
    extern uint8_t u8LEDflag;
    //接收中断
    if (__HAL_UART_GET_FLAG(huartx, UART_FLAG_RXNE) != RESET)    /* 处理接收中断  */
    {
        ch =huartx->Instance->RDR & 0xff;          
        ringbuffer_putchar(RecvFiFo, ch);        /* 从串口接收数据寄存器读取数据存放到接收FIFO */
        __HAL_UART_CLEAR_FLAG( huartx, UART_FLAG_RXNE );
    }
    if(__HAL_UART_GET_FLAG(huartx, UART_FLAG_IDLE) != RESET)
    {       
        /* read a data for clear receive idle interrupt flag */
        ch = huartx->Instance->RDR & 0xff;
        __HAL_UART_CLEAR_IDLEFLAG( huartx );
        
        if(huartx->Instance == USART3)
        {
            PdaRecvCount1 = ringbuffer_data_len(&g_tUsart3RecvFiFo);
            if(PdaRecvCount1 != 0 )
            {
                ringbuffer_get(&g_tUsart3RecvFiFo,&PdaRecvBuffer1[0],PdaRecvCount1);
                AT_Cmd_Process(1);
            }
        }                          
    }
}

/** @brief    串口,printf 函数 */
void UsartPrintf(UART_HandleTypeDef *huartx,const char *fmt, ...)
{
    int16_t size =0;

    va_list va_params;
    va_start(va_params,fmt);
    size =vsnprintf((char *)u8PrintfBuffer,(uint16_t)DEBUG_SIZE,fmt,va_params);
    va_end(va_params);

    if(size != -1 )
    {
        HAL_UART_Transmit(huartx, u8PrintfBuffer, size, 10000 );
    }
}
/*
void UsartPutString(UART_HandleTypeDef *huartx , uint8_t *p_string ,uint16_t length)
{
    extern uint8_t u8LEDflag;
    
    u8LEDflag += 2;
    if( globalSettingParam.u16_485_232 == ST_RS485)   
    {
        Rs485_Comm_Ctrl(0);
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        //delay_us(10);
        HAL_UART_Transmit(huartx, p_string, length, 10000);
        Rs485_Comm_Ctrl(1);
    }
    else
    {
        HAL_UART_Transmit(huartx, p_string, length, 10000);
    } 
}
*/




/******************* (C) COPYRIGHT 2016 LinCongCong*****END OF FILE************/


