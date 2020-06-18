/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_SDFATFS_H
#define __BSP_SDFATFS_H

/* Includes ------------------------------------------------------------------*/
#include "bsp_mcu.h"
#include "bsp_usart.h"
#include "ff.h"         /* FatFS文件系统模块*/
#include "ff_gen_drv.h"
#include "sd_diskio.h"


/* Private define ------------------------------------------------------------*/

void SDCard_Init(void);
uint8_t SDCard_ViewRootDir(void);
uint8_t SDCard_SaveDEBUG(const char *fmt, ...);
uint8_t SDcard_SaveModbusData(void);
uint8_t SDcard_SaveJsonData(uint8_t *pbuf);
uint8_t SDcard_GetJsonData(uint8_t (*func)(uint8_t *));
uint8_t SDcard_DeleteJsonData(void);

#endif /* __BSP_SDFATFS_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

