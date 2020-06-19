/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_ST7789VW_H
#define __BSP_ST7789VW_H

/* Includes ------------------------------------------------------------------*/
#include "bsp_mcu.h"
/* Private define ------------------------------------------------------------*/
#define ST7789VW_WIDTH  240
#define ST7789VW_HEIGHT 240

#define YUVToRGB(Y) ( ((Y>>3)<<11) + ((Y>>2)<<5) + (Y>>3) )

#include "imlib.h"
#if 0
typedef struct image {
    int w;
    int h;
    int bpp;
    union {
        uint8_t *pixels;
        uint8_t *data;
    };
} image_t;
#endif

//ST7789端口定义
#define ST7789VW_RES_Clr()  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,GPIO_PIN_RESET)//RES
#define ST7789VW_RES_Set()  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_12,GPIO_PIN_SET)

#define ST7789VW_DC_Clr()   HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13,GPIO_PIN_RESET)//DC
#define ST7789VW_DC_Set()   HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13,GPIO_PIN_SET)

#define ST7789VW_BLK_Clr()  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET)//BLK
#define ST7789VW_BLK_Set()  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET)

void ST7789VW_WriteReg(uint8_t u8Data);
void ST7789VW_SendData(uint8_t *pu8Data , uint16_t u16len);
void ST7789VW_SendData_DMA(uint8_t *pu8Data , uint16_t u16len);
void ST7789VW_Init(void);
void ST7789VW_SetVar(uint8_t mode);
void ST7789VW_SetGamma(uint8_t *pu8Data , uint8_t u8len);
void ST7789VW_SetWin(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye);
void ST7789VW_DrawPoint(uint16_t x, uint16_t y, uint16_t u16Color);
void ST7789VW_DrawLine(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t color);
void ST7789VW_FillRect(uint16_t x0, uint16_t y0,  uint16_t x1, uint16_t y1, uint16_t u16Color);
void ST7789VW_DrawBitLine16BPP(image_t *image);
void ST7789VW_DrawGrayscale(image_t *image);
#endif /* __BSP_ST7789VW_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

