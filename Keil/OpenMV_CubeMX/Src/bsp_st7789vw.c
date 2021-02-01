/* Includes ------------------------------------------------------------------*/
#include "bsp_st7789vw.h"
#include "ugui.h"

/* Private define ------------------------------------------------------------*/
static int16_t default_init_sequence[] = {
    /* SWRESET - Software reset */
    -1, 0x01,                                
    -2, 300,                               /* delay */

    /* SLPOUT - Sleep out & booster on */
    -1, 0x11,                          
    -2, 500,                               /* delay */
    
    -1, 0x36, 0x00,
    
    -1, 0x3A, 0x05,
    -1, 0xB2, 0x0C, 0x0C, 0x00, 0x33, 0x33,  
    -1, 0xB7, 0x35,
    -1, 0xBB, 0x19,
    -1, 0xC0, 0x2C,
    -1, 0xC2, 0x01,
    -1, 0xC3, 0x12,
    -1, 0xC4, 0x20,
    -1, 0xC6, 0x0F,
    -1, 0xD0, 0xA4, 0xA1,
    
    -1, 0xE0, 0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, 0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23,
    -1, 0xE1, 0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, 0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23,
    
    -1, 0x21,
    -1, 0x29,
    -2, 100,                               /* delay */
    -3
};

void ST7789VW_WriteReg(uint8_t u8Data)
{
    ST7789VW_DC_Clr();//写命令
    delay_us(20);
    HAL_SPI_Transmit(&hspi2,&u8Data,1, 1000);
    ST7789VW_DC_Set();//写数据
    delay_us(20);
}

void ST7789VW_SendData(uint8_t *pu8Data , uint16_t u16len)
{
    HAL_SPI_Transmit(&hspi2, pu8Data, u16len, 1000);   
}

void ST7789VW_SendData_DMA(uint8_t *pu8Data , uint16_t u16len)
{   
    HAL_SPI_Transmit_DMA(&hspi2, pu8Data, u16len);
    while(HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY){;}    
}
void ST7789VW_SendData_MYDMA(uint8_t *pu8Data , uint16_t u16len)
{   
    HAL_SPI_Transmit_DMA(&hspi2, pu8Data, u16len);
    while(HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY){;}    
}

void ST7789VW_Init(void)
{   
    //SPI2初始化
    DMA1_Stream0_Init();
    SPI2_Configuration();

    ST7789VW_RES_Clr();//硬件复位
    delay_ms(200);
    ST7789VW_RES_Set();
    delay_ms(200);

    ST7789VW_BLK_Set();//打开背光
    delay_ms(100);
    
    for(uint8_t n =0 ;;n++)
    {
        if(default_init_sequence[n] == -3) 
        {
            break;
        }else if(default_init_sequence[n] == -2)
        {
            delay_ms(default_init_sequence[++n]);
        }else if(default_init_sequence[n] == -1)
        {
            ST7789VW_WriteReg((uint8_t)default_init_sequence[++n]);
            while(default_init_sequence[++n] >= 0)
            {                    
                ST7789VW_SendData((uint8_t *)&default_init_sequence[n],1);
            }
            n--;
        }else{
            break;
        }
    }
    //清屏
    ST7789VW_FillRect(0, 0, ST7789VW_WIDTH, ST7789VW_HEIGHT, C_WHITE);
}



void ST7789VW_SetWin(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye)
{
    uint8_t u8data[8] = {0};
    u8data[0] = xs >> 8; u8data[1] = xs & 0xff;
    u8data[2] = xe >> 8; u8data[3] = xe & 0xff;
    u8data[4] = ys >> 8; u8data[5] = ys & 0xff;
    u8data[6] = ye >> 8; u8data[7] = ye & 0xff;    
    /* Column address */
    ST7789VW_WriteReg(0x2a);
    ST7789VW_SendData(&u8data[0],4);
    /* Row adress */
    ST7789VW_WriteReg(0x2b);
    ST7789VW_SendData(&u8data[4],4);
    /* Memory write */
    ST7789VW_WriteReg(0x2c);
}

void ST7789VW_DrawPoint(uint16_t x, uint16_t y, uint16_t u16Color)
{
    uint8_t u8data[2] = {0}; 
    ST7789VW_SetWin(x, y, x+1, y+1);
    u8data[0] = u16Color >> 8;
    u8data[1] = u16Color & 0xff;
    ST7789VW_SendData(&u8data[0],2);
}
void ST7789VW_DrawLine(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t color)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance;
	int incx,incy,uRow,uCol;
	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1;
	uRow=x1;//画线起点坐标
	uCol=y1;
	if(delta_x>0)incx=1; //设置单步方向 
	else if (delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;}
	if(delta_y>0)incy=1;
	else if (delta_y==0)incy=0;//水平线 
	else {incy=-1;delta_y=-delta_x;}
	if(delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y;
	for(t=0;t<distance+1;t++)
	{
		ST7789VW_DrawPoint(uRow,uCol,color);//画点
		xerr+=delta_x;
		yerr+=delta_y;
		if(xerr>distance)
		{
			xerr-=distance;
			uRow+=incx;
		}
		if(yerr>distance)
		{
			yerr-=distance;
			uCol+=incy;
		}
	}

}
void ST7789VW_FillRect(uint16_t x0, uint16_t y0,  uint16_t x1, uint16_t y1, uint16_t u16Color)
{
    uint8_t u8data[2] = {0};
    uint32_t u32len = 0;
    
    ST7789VW_SetWin(x0, y0, x1, y1);
    u8data[0] = u16Color >> 8;
    u8data[1] = u16Color & 0xff;
    
    u32len = (x1 - x0 + 1) * (y1 - y0 + 1);
    for (uint32_t n=0; n < u32len; n++)
    {
        ST7789VW_SendData(&u8data[0],2);
    }
}

void ST7789VW_DrawBitLine16BPP(image_t *image)
{
    
    ST7789VW_SetWin(0,80,image->w-1,image->h-1+80);
    if((image->w)*(image->h)*2 <= 65535)
    {
        ST7789VW_SendData_DMA(image->data,(image->w)*(image->h)*2);
    }
    else
    {
        ST7789VW_SendData_DMA(&image->data[0],65535);
        ST7789VW_SendData_DMA(&image->data[65535],(image->w)*(image->h)*2-65535);
    }
    
}
void ST7789VW_DrawGrayscale(image_t *image) 
{
  	int i,x,y; 
	unsigned char picH,picL;

	uint16_t temp = 0;

	ST7789VW_SetWin(0,50,image->w-1,image->h-1);
//	for(y=0;y<image->h;y++)
//	{
//		for(x=0;x<image->w;x++)
//		{

	ST7789VW_SendData_DMA(&image->data[0],65535);
		ST7789VW_SendData_DMA(&image->data[65535],(image->w)*(image->h)*2-65535);		
			
//		}
//	}

	//delay_ms(500);
}

/******************* (C) COPYRIGHT 2016 LinCongCong*****END OF FILE************/


