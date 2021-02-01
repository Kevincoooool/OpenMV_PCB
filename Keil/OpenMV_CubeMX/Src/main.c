/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "bsp_mcu.h"
#include "bsp_usart.h"
//#include "bsp_st7735r.h"
#include "bsp_st7789vw.h"

#include "ugui.h"
#include "SDFatfs.h"

#include "framebuffer.h"
#include "sensor.h"
#include "imlib.h"


void example_qrcodes(void);
void example_blob_detection(void);
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
UG_GUI  GUI_1;

#define MAX_OBJ_NUM 10
/* Window 1 */
UG_WINDOW window_1;
UG_OBJECT obj_buff_wnd_1[MAX_OBJ_NUM];
UG_BUTTON button1_1;
UG_BUTTON button1_2;

uint8_t u8statue = 0;
int8_t ret = 0;
uint32_t u32time1 = 0, u32time2 = 0, u32temp = 0;

extern PCD_HandleTypeDef hpcd_USB_OTG_FS;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
/* Callback function for the main menu */
void window_1_callback( UG_MESSAGE* msg )
{
       
        if ( msg->type == MSG_TYPE_OBJECT )
   {      
                if ( msg->id == OBJ_TYPE_BUTTON )
                {
                        switch( msg->sub_id )
                        {
                          case BTN_ID_0:
                          {
                                if(msg->event==OBJ_EVENT_PRESSED)
                                {
                                       // UG_WindowShow( &window_2 );
                                }
                               
                          break;
                          }
                        case BTN_ID_1: // Show UI info
                        {
                        break;
                        }
                          
                        }
                }
   }
}
int main(void)
{
	uint32_t tick1;
	uint8_t show_buf[8];
    /* MCU Configuration----------------------------------------------------------*/
    
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();
    SystemClock_Config();
    NVIC_Configuration();
    GPIO_Configuration();
    systick_init();
    USART3_Configuration(UART3_BAUD);
    DEBUG("OpenMV BUILD:%04d %s %s" ,BUILD_NUMBER , __DATE__, __TIME__ );
 
    ST7789VW_Init();
    UG_Init( &GUI_1, (void*)ST7789VW_DrawPoint, (UG_S16)ST7789VW_WIDTH, (UG_S16)ST7789VW_HEIGHT );
	UG_DriverRegister(DRIVER_DRAW_LINE,(void*)ST7789VW_DrawLine);
	UG_DriverRegister(DRIVER_FILL_FRAME,(void*)ST7789VW_FillRect);
	UG_DriverEnable(DRIVER_DRAW_LINE);
	UG_DriverEnable(DRIVER_FILL_FRAME);
	UG_FillScreen( C_DARK_GRAY );
	UG_SelectGUI(&GUI_1);
    UG_FontSelect(&FONT_12X16);
    UG_SetForecolor(C_BLACK  );
    UG_SetBackcolor(C_DARK_GRAY );
	/* Create Window 1 */
	UG_WindowCreate( &window_1, obj_buff_wnd_1, MAX_OBJ_NUM, window_1_callback );
	UG_WindowSetTitleText( &window_1, "OpenMV4" );
	UG_WindowSetTitleTextFont( &window_1, &FONT_12X16 );
	/* Configure Button 2 */
	UG_ButtonCreate( &window_1, &button1_1, BTN_ID_0, 20, 2, 100, 50);
	UG_ButtonCreate( &window_1, &button1_2, BTN_ID_1, 120, 2, 220, 50 );
	UG_ButtonSetFont( &window_1, BTN_ID_0, &FONT_12X16 );
	UG_ButtonSetBackColor( &window_1, BTN_ID_0, C_WHITE );
	UG_ButtonSetText( &window_1, BTN_ID_0, "Find" );
	
	UG_ButtonSetFont( &window_1, BTN_ID_1, &FONT_12X16 );
	UG_ButtonSetBackColor( &window_1, BTN_ID_1, C_WHITE );
	UG_ButtonSetText( &window_1, BTN_ID_1, "OK!" );
	UG_WindowShow( &window_1 );

	UG_Update();
    
//    UG_PutString(0,170,"Hello OpenMV4!!");
//    
    SDCard_Init();
    
    USB_DEVICE_Init();
    delay_ms(100);
    USB_printf("USB_DEVICE_Init OK!!\r\n");

    fb_alloc_init0();
    u32temp = fb_avail();
    DEBUG("fb_avail£º%d",u32temp);
	
	example_blob_detection();
//	example_face_detection();
//	example_qrcodes();


}

void DMA1_Stream0_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma1_stream0);
}

void SPI2_IRQHandler(void)
{
    HAL_SPI_IRQHandler(&hspi2);    
}

void DCMI_IRQHandler(void) 
{
    HAL_DCMI_IRQHandler(&hdcmi);
}

void DMA2_Stream1_IRQHandler(void) 
{
    HAL_DMA_IRQHandler(&hdma2_stream1);
}

void OTG_FS_IRQHandler(void)
{
    HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
