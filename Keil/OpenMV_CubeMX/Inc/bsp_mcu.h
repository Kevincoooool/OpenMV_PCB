/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_MCU_H
#define __BSP_MCU_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "build.h"
#include "common.h"

/* Private define ------------------------------------------------------------*/

static struct Time
{
    volatile uint32_t msPeriod;	// 整周期的时间，ms。
    uint32_t ticksPerUs;        // 每us等于的滴答次数
    uint32_t ticksPerMs;        // 每ms等于的滴答次数
    uint32_t msPerPeriod;       // 每周期的ms数
}time;

#define Loop_Begin(time_ms) {static uint32_t target = 0;if(target <= time_nowms())\
                            {target = time_nowms() + time_ms;
#define Loop_End()          }}

extern UART_HandleTypeDef huart3;
extern SPI_HandleTypeDef  hspi2; 
extern SD_HandleTypeDef   hsd1;
extern I2C_HandleTypeDef  hi2c1;
extern DCMI_HandleTypeDef hdcmi;
//extern USBD_HandleTypeDef hUsbDeviceFS;

extern DMA_HandleTypeDef hdma1_stream0;
extern DMA_HandleTypeDef  hdma2_stream1;

void systick_init(void);
void delay_us(uint32_t nus);
void delay_ms(uint16_t nms);
uint64_t time_nowus(void);
uint32_t time_nowms(void);
void systick_sleep(uint16_t nms);

extern void _Error_Handler(char *, int);
void SystemClock_Config(void);
void NVIC_Configuration(void);
void GPIO_Configuration(void);
void LED_Power_Ctrl(uint16_t led, uint8_t state);
void USART3_Configuration(uint32_t BaudRate);
void SPI2_Configuration(void);
void DMA1_Stream0_Init(void);
void SDMMC1_SD_Init(void);
void I2C1_Init(void);
void DMA2_Stream1_Init(void);
void DCMI_Init(uint32_t jpeg_mode,uint32_t u32VSPolarity ,uint32_t u32HSPolarity, uint32_t u32PCKPolarity);
void USB_DEVICE_Init(void);

/* Private define ------------------------------------------------------------*/

#endif /* __BSP_MCU_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

