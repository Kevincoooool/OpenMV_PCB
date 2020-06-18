/* Includes ------------------------------------------------------------------*/
#include "bsp_st7735r.h"

/*
    Gamma string format:
    VRF0P VOS0P PK0P PK1P PK2P PK3P PK4P PK5P PK6P PK7P PK8P PK9P SELV0P SELV1P SELV62P SELV63P
    VRF0N VOS0N PK0N PK1N PK2N PK3N PK4N PK5N PK6N PK7N PK8N PK9N SELV0N SELV1N SELV62N SELV63N
*/
static int8_t default_gamma[] = {
    0x0F, 0x1A, 0x0F, 0x18, 0x2F, 0x28, 0x20, 0x22, 0x1F, 0x1B, 0x23, 0x37, 0x00, 0x07, 0x02, 0x10,\
    0x0F, 0x1B, 0x0F, 0x17, 0x33, 0x2C, 0x29, 0x2E, 0x30, 0x30, 0x39, 0x3F, 0x00, 0x07, 0x03, 0x10    
}; 

static int8_t default_init_sequence[] = {
    /* SWRESET - Software reset */
    -1, 0x01,                                
    -2, 150,                               /* delay */

    /* SLPOUT - Sleep out & booster on */
    -1, 0x11,                          
    -2, 500,                               /* delay */

    /* FRMCTR1 - frame rate control: normal mode
         frame rate = fosc / (1 x 2 + 40) * (LINE + 2C + 2D) */
    -1, 0xB1, 0x01, 0x2C, 0x2D, 

    /* FRMCTR2 - frame rate control: idle mode
         frame rate = fosc / (1 x 2 + 40) * (LINE + 2C + 2D) */
    -1, 0xB2, 0x01, 0x2C, 0x2D, 

    /* FRMCTR3 - frame rate control - partial mode
         dot inversion mode, line inversion mode */
    -1, 0xB3, 0x01, 0x2C, 0x2D, 0x01, 0x2C, 0x2D,

    /* INVCTR - display inversion control
         no inversion */
    -1, 0xB4, 0x07,

    /* PWCTR1 - Power Control
         -4.6V, AUTO mode */
    -1, 0xC0, 0xA2, 0x02, 0x84,

    /* PWCTR2 - Power Control
         VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD */
    -1, 0xC1, 0xC5,

    /* PWCTR3 - Power Control
         Opamp current small, Boost frequency */
    -1, 0xC2, 0x0A, 0x00,

    /* PWCTR4 - Power Control
         BCLK/2, Opamp current small & Medium low */
    -1, 0xC3,0x8A,0x2A,

    /* PWCTR5 - Power Control */
    -1, 0xC4, 0x8A, 0xEE,

    /* VMCTR1 - Power Control */
    -1, 0xC5, 0x0E,

    /* INVOFF - Display inversion off */
    -1, 0x20,

    /* COLMOD - Interface pixel format 65k mode */
    -1, 0x3A, 0x05,
    
    /* DISPON - Display On */
    -1, 0x29,
    -2, 100,                               /* delay */

    /* NORON - Partial off (Normal) */
    -1, 0x13,
    -2, 10,                               /* delay */

    /* end marker */
    -3                                  
};

void ST7735R_WriteReg(uint8_t u8Data)
{
    ST7735R_DC_Clr();//写命令
    delay_us(10);
    HAL_SPI_Transmit(&hspi2,&u8Data,1, 1000);
    ST7735R_DC_Set();//写数据
    delay_us(10);    
}

void ST7735R_SendData(uint8_t *pu8Data , uint16_t u16len)
{
    HAL_SPI_Transmit(&hspi2, pu8Data, u16len, 1000);   
}


void ST7735R_Init(void)
{
    //初始化SPI2
    SPI2_Configuration();
    //硬件复位
    ST7735R_RES_Clr();
    delay_ms(100);
    ST7735R_RES_Set();
    delay_ms(100);
    //打开背光灯
    ST7735R_BLK_Set();
    delay_ms(100);

    for(uint8_t n = 0 ;n < sizeof(default_init_sequence) ; n++)
    {
        if(default_init_sequence[n] == -3) 
        {
            break;
        }else if(default_init_sequence[n] == -2)
        {
            delay_ms(default_init_sequence[++n]);
        }else if(default_init_sequence[n] == -1)
        {
            ST7735R_WriteReg(default_init_sequence[++n]);
            while(default_init_sequence[++n] >= 0)
            {                    
                ST7735R_SendData(&default_init_sequence[n],1);
            }
        }else
        {
            break;
        }
    }
    ST7735R_SetVar(0);
    ST7735R_SetGamma(default_gamma, 32);
    //清屏
    ST7735R_FillRect(0, 0, ST7735R_WIDTH, ST7735R_HEIGHT, 0xffff);
    
}

#define MY (1 << 7)
#define MX (1 << 6)
#define MV (1 << 5)
#define ML (1 << 4)
/* MADCTL - Memory data access control
 RGB/BGR:
 1. Mode selection pin SRGB
    RGB H/W pin for color filter setting: 0=RGB, 1=BGR
 2. MADCTL RGB bit
    RGB-BGR ORDER color filter panel: 0=RGB, 1=BGR */
void ST7735R_SetVar(uint8_t mode)
{ 
    uint8_t u8data = 0;
    
    switch (mode) 
    {
    case 0:
        u8data = MX | MY | (0 << 3);
        break;
    case 90:
        u8data = MX | MV | (0 << 3);
        break;
    case 180:
        u8data = (0 << 3);
        break;
    case 270:
        u8data = MY | MV | (0 << 3);
        break;
        
    }
    u8data = 0xcd;
    ST7735R_WriteReg(0x36);
    ST7735R_SendData(&u8data,1);
    
}


void ST7735R_SetGamma(uint8_t *pu8Data , uint8_t u8len)
{
    for(uint8_t n = 0 ; n < u8len/16; n++)
    {
        ST7735R_WriteReg(0xe0+n);
        ST7735R_SendData(&pu8Data[16*n],16);
    }
}

void ST7735R_SetWin(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye)
{
    uint8_t u8data[8] = {0};
    u8data[0] = xs >> 8; u8data[1] = xs & 0xff;
    u8data[2] = xe >> 8; u8data[3] = xe & 0xff;
    u8data[4] = ys >> 8; u8data[5] = ys & 0xff;
    u8data[6] = ye >> 8; u8data[7] = ye & 0xff;    
    /* Column address */
    ST7735R_WriteReg(0x2a);
    ST7735R_SendData(&u8data[0],4);
    /* Row adress */
    ST7735R_WriteReg(0x2b);
    ST7735R_SendData(&u8data[4],4);
    /* Memory write */
    ST7735R_WriteReg(0x2c);
}

void ST7735R_DrawPoint(uint16_t x, uint16_t y, uint16_t u16Color)
{
    uint8_t u8data[2] = {0}; 
    ST7735R_SetWin(x, y, x+1, y+1);
    u8data[0] = u16Color >> 8;
    u8data[1] = u16Color & 0xff;
    ST7735R_SendData(&u8data[0],2);
}

void ST7735R_FillRect(uint16_t x0, uint16_t y0,  uint16_t x1, uint16_t y1, uint16_t u16Color)
{
    uint8_t u8data[2] = {0};
    uint32_t u32len = 0;
    
    ST7735R_SetWin(x0, y0, x1, y1);
    u8data[0] = u16Color >> 8;
    u8data[1] = u16Color & 0xff;
    
    u32len = (x1 - x0 + 1) * (y1 - y0 + 1);
    for (uint32_t n=0; n < u32len; n++)
    {
        ST7735R_SendData(&u8data[0],2);
    }
}

//UG_RESULT ST7735R_DrawLine( UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR c )
//{
//    ST7735R_BeginRect(x1, y1,x2, y2);
//     //HAL_SPI_Transmit(&SPI2_Handler,TxData,size, 1000);
//    return 0;
//}

//UG_RESULT ST7735R_FillFrame( UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR c )
//{
//    ST7735R_BeginRect(x1, y1,x2, y2);
//     //HAL_SPI_Transmit(&SPI2_Handler,TxData,size, 1000);
//    return 0;
//}

//UG_RESULT ST7735R_DrawBitLine16BPP(image_t *image)
//{
//	ST7735R_BeginRect(0,0,image->w-1,image->h-1);	
//    HAL_GPIO_WritePin(GPIOD,GPIO_PIN_13,GPIO_PIN_SET);
//    HAL_SPI_Transmit(&hspi2,image->data,(image->w)*(image->h)*2,1000);
//}