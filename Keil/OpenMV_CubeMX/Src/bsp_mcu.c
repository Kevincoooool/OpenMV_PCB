/* Includes ------------------------------------------------------------------*/
#include "bsp_mcu.h"
#include "usbd_desc.h"
#include "usbd_cdc.h" 
#include "usbd_cdc_if.h"

/* Private define ------------------------------------------------------------*/
UART_HandleTypeDef huart3 = {0};
SPI_HandleTypeDef  hspi2  = {0};
SD_HandleTypeDef   hsd1 = {0};
I2C_HandleTypeDef  hi2c1 = {0};
DCMI_HandleTypeDef hdcmi = {0};
USBD_HandleTypeDef hUsbDeviceFS = {0};

DMA_HandleTypeDef  hdma1_stream0 = {0};
DMA_HandleTypeDef  hdma2_stream1 = {0};

/*****************************************************************************/
void systick_init(void)
{
    time.msPeriod = 0;
    time.ticksPerUs = SystemCoreClock / 1e6;
    time.ticksPerMs = SystemCoreClock / 1e3;
    time.msPerPeriod = 1;
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/(1000/time.msPerPeriod));
    NVIC_SetPriority (SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 12UL); //https://www.cnblogs.com/firege/p/5805734.html
}
void delay_us(uint32_t nus)
{
    uint64_t target = time_nowus() + nus - 2;
    while(time_nowus() <= target) {;}
}
void delay_ms(uint16_t nms)
{
    delay_us(nms * 1000);
}
void HAL_SYSTICK_Callback(void)
{
    time.msPeriod += time.msPerPeriod;
}
uint64_t time_nowus(void)
{
    return time.msPeriod * (uint64_t)1000 + (SysTick->LOAD - SysTick->VAL) / time.ticksPerUs;
}
uint32_t time_nowms(void)
{
    return time.msPeriod + (SysTick->LOAD - SysTick->VAL) / time.ticksPerMs;
}

void systick_sleep(uint16_t nms)
{
    delay_ms(nms);
}
/**********************************************************************************/

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 400000000
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    /** Supply configuration update enable 
    */
    MODIFY_REG(PWR->CR3, PWR_CR3_SCUEN, 0);
    /** Configure the main internal regulator output voltage 
    */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    while ((PWR->D3CR & (PWR_D3CR_VOSRDY)) != PWR_D3CR_VOSRDY) 
    {

    }
    /** Initializes the CPU, AHB and APB busses clocks 
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 4;
    RCC_OscInitStruct.PLL.PLLN = 50;
    RCC_OscInitStruct.PLL.PLLP = 2;
    RCC_OscInitStruct.PLL.PLLQ = 10;
    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
    RCC_OscInitStruct.PLL.PLLFRACN = 0;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }
    /** Initializes the CPU, AHB and APB busses clocks 
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }

    //时钟源配置
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI2 | RCC_PERIPHCLK_SDMMC|\
                                               RCC_PERIPHCLK_USB | RCC_PERIPHCLK_I2C1;
    PeriphClkInitStruct.PLL2.PLL2M = 8;
    PeriphClkInitStruct.PLL2.PLL2N = 48;
    PeriphClkInitStruct.PLL2.PLL2P = 8;
    PeriphClkInitStruct.PLL2.PLL2Q = 2;
    PeriphClkInitStruct.PLL2.PLL2R = 8;
    PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
    PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
    PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
    
    PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL;
    PeriphClkInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL2;
    PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
    PeriphClkInitStruct.I2c123ClockSelection = RCC_I2C123CLKSOURCE_D2PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }
}

void NVIC_Configuration(void)
{
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_2);
    NVIC_SetPriority (SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 12UL);

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();

    __HAL_RCC_RNG_CLK_ENABLE();
    __HAL_RCC_USART3_CLK_ENABLE();
    __HAL_RCC_SPI2_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();
    __HAL_RCC_SDMMC1_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();
    __HAL_RCC_DCMI_CLK_ENABLE();
    __HAL_RCC_USB_OTG_FS_CLK_ENABLE();
    __HAL_RCC_SYSCFG_CLK_ENABLE();
	HAL_PWREx_EnableUSBVoltageDetector();

    /* USART3 interrupt Init */
    HAL_NVIC_SetPriority(USART3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);


    /************* 后面中断比滴答定时器中断优先等级低************************/
    HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
    HAL_NVIC_SetPriority(SPI2_IRQn, 1, 1);
    HAL_NVIC_EnableIRQ(SPI2_IRQn);
    
    HAL_NVIC_SetPriority(DCMI_IRQn, 1, 2);
    HAL_NVIC_EnableIRQ(DCMI_IRQn);
    HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 1, 3);
    HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);
    
    /* Set USBFS Interrupt priority */
    HAL_NVIC_SetPriority(OTG_FS_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(OTG_FS_IRQn);

}

void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    //RGB LED灯引脚初始化，初始状态为高电平
    /*Configure GPIO pins : PC0-->LED_R PC1-->LED_G PC2-->LED_B*/
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2, GPIO_PIN_SET);
    
    //USART3引脚初始化
    /*Configure GPIO pins : PB10-->USART3_TX   PB11-->USART3_RX */
    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    //LCD液晶屏引脚初始化
    /*Configure GPIO pins : PD12-->LCD_RST PD13-->LCD_RS  */
    GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    /*Configure GPIO pins : PB12-->LCD_BLK  */
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    /*SPI2 GPIO Configuration : PB13--> SPI2_SCK  PB15--> SPI2_MOSI */
    GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    //SD卡引脚初始化
    /*SDMMC1 GPIO Configuration ：PC8--> SDMMC1_D0 PC9--> SDMMC1_D1  PC10--> SDMMC1_D2 PC11--> SDMMC1_D3 
                                  PC12--> SDMMC1_CK  PD2--> SDMMC1_CMD (PD0-->SD_CD)*/
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11 |GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_2;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC1;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    
    //摄像头模块引脚配置1:DCMI接口
    /*DCMI GPIO Configuration : PA4 --> DCMI_HSYNC  PA6 --> DCMI_PIXCLK
                                PB6 --> DCMI_D5     PB7 --> DCMI_VSYNC
                                PC6 --> DCMI_D0     PC7 --> DCMI_D1
                                PE0 --> DCMI_D2     PE1 --> DCMI_D3         PE4 --> DCMI_D4     PE5 --> DCMI_D6     PE6 --> DCMI_D7*/
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF13_DCMI;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_6;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    //摄像头模块引脚配置2：IIC引脚
    /*I2C1 GPIO Configuration : PB8--> I2C1_SCL PB9--> I2C1_SDA*/
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    //摄像头模块引脚配置3：其他引脚
    /*Configure GPIO pin : DCMI_XCLK-->PA8 DCMI_RST-->PA10 DCMI_PWDN-->PD7*/
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_7;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    
    //USB虚拟你串口引脚初始化
    /*Configure GPIO pin :  PA9--> USB_VBUS     PA11--> USB_DM      PA12--> USB_DP  */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF10_OTG1_FS;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

//RGB LED灯控制 
void LED_Power_Ctrl(uint16_t led, uint8_t state)
{
    if(state == 0)
    {
        HAL_GPIO_WritePin(GPIOC, led, GPIO_PIN_RESET );
    }else if(state == 1)
    {
        HAL_GPIO_WritePin(GPIOC, led, GPIO_PIN_SET);
    }else if(state == 2)
    {
        HAL_GPIO_TogglePin(GPIOC,led);
    }
}
//USART3初始化配置
void USART3_Configuration(uint32_t BaudRate)
{
    UART_HandleTypeDef *huartx = &huart3;
    
    huartx->Instance = USART3;
    huartx->Init.BaudRate = BaudRate;
    huartx->Init.WordLength = UART_WORDLENGTH_8B;
    huartx->Init.StopBits = UART_STOPBITS_1;
    huartx->Init.Parity = UART_PARITY_NONE;
    huartx->Init.Mode = UART_MODE_TX_RX;
    huartx->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huartx->Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(huartx) != HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }

    __HAL_UART_CLEAR_FLAG( huartx, UART_FLAG_RXNE );    //接受完成中断
    __HAL_UART_ENABLE_IT(huartx, UART_IT_RXNE);
    __HAL_UART_CLEAR_IDLEFLAG( huartx );    //帧中断
    __HAL_UART_ENABLE_IT(huartx, UART_IT_IDLE);
}

/** @brief    SPIx初始化配置 */
void SPI2_Configuration(void)
{
    SPI_HandleTypeDef *SPIx = &hspi2;
    
    SPIx->Instance=SPI2;                        
    SPIx->Init.Mode=SPI_MODE_MASTER;             
    SPIx->Init.Direction=SPI_DIRECTION_2LINES;  
    SPIx->Init.DataSize=SPI_DATASIZE_8BIT;      
    SPIx->Init.CLKPolarity=SPI_POLARITY_HIGH;    
    SPIx->Init.CLKPhase=SPI_PHASE_1EDGE;        
    SPIx->Init.NSS=SPI_NSS_SOFT;
    SPIx->Init.BaudRatePrescaler=SPI_BAUDRATEPRESCALER_2;
    SPIx->Init.FirstBit=SPI_FIRSTBIT_MSB;        
    SPIx->Init.TIMode=SPI_TIMODE_DISABLE;       
    SPIx->Init.CRCCalculation=SPI_CRCCALCULATION_DISABLE;
    SPIx->Init.CRCPolynomial=7;
    SPIx->Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
    SPIx->Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
    SPIx->Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
    SPIx->Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    SPIx->Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    SPIx->Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
    SPIx->Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
    SPIx->Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
    SPIx->Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
    SPIx->Init.IOSwap = SPI_IO_SWAP_DISABLE;
    
    if ( HAL_SPI_Init(SPIx) != HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }
    __HAL_SPI_ENABLE(SPIx);                    //使能SPI

}

void DMA1_Stream0_Init(void)
{
    hdma1_stream0.Instance = DMA1_Stream0;
    hdma1_stream0.Init.Request = DMA_REQUEST_SPI2_TX;
    hdma1_stream0.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma1_stream0.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma1_stream0.Init.MemInc = DMA_MINC_ENABLE;
    hdma1_stream0.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma1_stream0.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma1_stream0.Init.Mode = DMA_NORMAL;
    hdma1_stream0.Init.Priority = DMA_PRIORITY_HIGH;
    hdma1_stream0.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma1_stream0) != HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(&hspi2,hdmatx,hdma1_stream0);
}

//SDMMC1初始化
void SDMMC1_SD_Init(void)
{
    hsd1.Instance = SDMMC1;
    hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
    hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
    hsd1.Init.BusWide = SDMMC_BUS_WIDE_4B;
    hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
    hsd1.Init.ClockDiv = 0;   
}

void I2C1_Init(void)
{
    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x10C0ECFF; 
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }

    /**Configure Analogue filter 
    */
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }

    /**Configure Digital filter 
    */
    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }  
}


void DMA2_Stream1_Init(void)
{
    hdma2_stream1.Instance = DMA2_Stream1;
    hdma2_stream1.Init.Request = DMA_REQUEST_DCMI;
    hdma2_stream1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma2_stream1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma2_stream1.Init.MemInc = DMA_MINC_ENABLE;
    hdma2_stream1.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma2_stream1.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma2_stream1.Init.Mode = DMA_NORMAL;
    hdma2_stream1.Init.Priority             = DMA_PRIORITY_HIGH;        /* Priority level : high            */
    hdma2_stream1.Init.FIFOMode             = DMA_FIFOMODE_ENABLE;      /* FIFO mode enabled                */
    hdma2_stream1.Init.FIFOThreshold        = DMA_FIFO_THRESHOLD_FULL;  /* FIFO threshold full              */
    hdma2_stream1.Init.MemBurst             = DMA_MBURST_INC4;          /* Memory burst                     */
    hdma2_stream1.Init.PeriphBurst          = DMA_PBURST_SINGLE;        /* Peripheral burst                 */
    if (HAL_DMA_Init(&hdma2_stream1) != HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }

    //__HAL_LINKDMA(&hdcmi,DMA_Handle,hdma2_stream1);
}

// DCMI configuration
void DCMI_Init(uint32_t jpeg_mode,uint32_t u32VSPolarity ,uint32_t u32HSPolarity, uint32_t u32PCKPolarity) 
{ 
    // DCMI configuration
    hdcmi.Instance         = DCMI;
    // VSYNC clock polarity
    hdcmi.Init.VSPolarity  = u32VSPolarity ?DCMI_VSPOLARITY_HIGH : DCMI_VSPOLARITY_LOW;
    // HSYNC clock polarity
    hdcmi.Init.HSPolarity  = u32HSPolarity ?DCMI_HSPOLARITY_HIGH : DCMI_HSPOLARITY_LOW;
    // PXCLK clock polarity
    hdcmi.Init.PCKPolarity = u32PCKPolarity ?DCMI_PCKPOLARITY_RISING : DCMI_PCKPOLARITY_FALLING;

    hdcmi.Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;    // Enable Hardware synchronization
    hdcmi.Init.CaptureRate = DCMI_CR_ALL_FRAME;        // Capture rate all frames
    hdcmi.Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B; // Capture 8 bits on every pixel clock
    hdcmi.Init.JPEGMode = jpeg_mode;                   // Set JPEG Mode
    hdcmi.Init.ByteSelectMode  = DCMI_BSM_ALL;         // Capture all received bytes
    hdcmi.Init.ByteSelectStart = DCMI_OEBS_ODD;        // Ignored
    hdcmi.Init.LineSelectMode  = DCMI_LSM_ALL;         // Capture all received lines
    hdcmi.Init.LineSelectStart = DCMI_OELS_ODD;        // Ignored

    // Associate the DMA handle to the DCMI handle
    __HAL_LINKDMA(&hdcmi, DMA_Handle, hdma2_stream1);

   // Initialize the DCMI
    HAL_DCMI_DeInit(&hdcmi);
    if (HAL_DCMI_Init(&hdcmi) != HAL_OK) {
        // Initialization Error
        _Error_Handler(__FILE__, __LINE__);
    }
}

//USB使能连接/断线 1,允许连接  0,断开   
#define RegBase  (0x40005C00L)  /* USB_IP Peripheral Registers base address */
#define _SetCNTR(wRegValue)  (*CNTR   = (uint16_t)wRegValue)
#define _GetCNTR()   ((uint16_t) *CNTR)
#define CNTR    ((__IO unsigned *)(RegBase + 0x40))
void USB_Port_Set(uint8_t onoff)
{   
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(onoff)
    {
        _SetCNTR(_GetCNTR()&(~(1<<1)));//退出断电模式
        GPIO_InitStruct.Pin = (GPIO_PIN_11 | GPIO_PIN_12);
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF10_OTG1_FS;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); 
    }
    else
    {   
        _SetCNTR(_GetCNTR()|(1<<1));  // 断电模式
        GPIO_InitStruct.Pin = (GPIO_PIN_11 | GPIO_PIN_12);
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); 
        HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_RESET );    
    }
}

void USB_DEVICE_Init(void)
{
//    delay_ms(100);
//    USB_Port_Set(0);    //USB先断开
//    delay_ms(1000);
//    USB_Port_Set(1);    //USB再次连接
//    delay_ms(700);
    
    if (USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS) != USBD_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }
    if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC) != USBD_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }
    if (USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS) != USBD_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }
    if (USBD_Start(&hUsbDeviceFS) != USBD_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }

}


/******************* (C) COPYRIGHT 2016 lincongcong *****END OF FILE************/

