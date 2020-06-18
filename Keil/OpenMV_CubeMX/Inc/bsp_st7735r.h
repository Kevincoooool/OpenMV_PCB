/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_ST7735R_H
#define __BSP_ST7735R_H

/* Includes ------------------------------------------------------------------*/
#include "bsp_mcu.h"

/* Private define ------------------------------------------------------------*/
#define ST7735R_WIDTH  128
#define ST7735R_HEIGHT 160

//LCD¶Ë¿Ú¶¨Òå
#define ST7735R_RES_Clr()  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,GPIO_PIN_RESET)//RES
#define ST7735R_RES_Set()  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,GPIO_PIN_SET)

#define ST7735R_DC_Clr()   HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13,GPIO_PIN_RESET)//DC
#define ST7735R_DC_Set()   HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13,GPIO_PIN_SET)

#define ST7735R_BLK_Clr()  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET)//BLK
#define ST7735R_BLK_Set()  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET)

void ST7735R_WriteReg(uint8_t u8Data);
void ST7735R_SendData(uint8_t *pu8Data , uint16_t u16len);
void ST7735R_Init(void);
void ST7735R_SetVar(uint8_t mode);
void ST7735R_SetGamma(uint8_t *pu8Data , uint8_t u8len);
void ST7735R_SetWin(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye);
void ST7735R_DrawPoint(uint16_t x, uint16_t y, uint16_t u16Color);
void ST7735R_FillRect(uint16_t x0, uint16_t y0,  uint16_t x1, uint16_t y1, uint16_t u16Color);
    
    

#endif /* __BSP_ST7735R_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
