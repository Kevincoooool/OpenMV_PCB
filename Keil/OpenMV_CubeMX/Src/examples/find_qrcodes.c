/*
 * This file is part of the Micro Vision Device MVD project.
 * Copyright (c) 2017 Micro Vision Device [http://www.mvdevice.com]
 *
 * by yuanjun<yuanjun2006@outlook.com>
 *
 * qrcodes example.
 *
 */

#include <stdlib.h>
#include "stm32h7xx_hal.h"
#include "imlib.h"
#include "bsp_mcu.h"
#include "sensor.h"
#include "omv_boardconfig.h"

/* yuanjun <yuanjun2006@outlook.com> 
 * 
 * 二维码解码例子
 */

void example_qrcodes(void)
{
	int tick1;
	
	int i;
	int cnt;
	rectangle_t *r;
	list_t lst;
	
	int ret;
	image_t img;
	rectangle_t roi;
	array_t *objects_array;
	cascade_t face_cascade;
	ret = sensor_init();
    if (ret < 0) {
        DEBUG("sensor_init return %d\n", ret);
        while(1);
    }

    ret = sensor_reset();
    if (ret < 0) {
        DEBUG("sensor_reset return %d\n", ret);
        while(1);
    }
//	sensor_set_contrast(0);
//	sensor_set_gainceiling(GAINCEILING_16X);

	ret = sensor_set_pixformat(PIXFORMAT_RGB565);
	if (ret < 0) {
		DEBUG("sensor_set_pixformat return %d\n", ret);
		goto err;
	}
	
	ret = sensor_set_framesize(FRAMESIZE_HQVGA);
	if (ret < 0) {
		DEBUG("sensor_set_framesize return %d\n", ret);
		goto err;
	}

	 ret = sensor_set_auto_gain(0, 0,0);
	if (ret < 0) {
		DEBUG("sensor_set_framesize return %d\n", ret);
		goto err;
	}
	 
	DEBUG("Running example_qrcodes\n");

	delay_ms(1000);

	while (1) {
		tick1 = HAL_GetTick();
		ret = sensor_snapshot_simple(&img);
		
		if (ret != 0) {
			continue;
		}

		/*镜头失真校正，可以根据情况调整第二个参数(1.5) */
		imlib_lens_corr(&img, 0.5, 1.0);
		
		roi.x = 0;
		roi.y = 0;
		roi.w = img.w;
		roi.h = img.h;

		imlib_find_qrcodes(&lst, &img, &roi);
//		USB_printf("-------------------------------------------------\n");
		/* 打印二维码解码信息 */
		while (lst.size) {
			
//			USB_printf("-------------------------------------------------\n");
			find_qrcodes_list_lnk_data_t lnk_data;
			list_pop_back(&lst, &lnk_data);
			USB_printf("%s  ", lnk_data.payload);
			
			imlib_draw_string(&img, 10, 10, lnk_data.payload, 0xFF00, 3, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
			imlib_draw_rectangle(&img,lnk_data.rect.x, lnk_data.rect.y, lnk_data.rect.w,lnk_data.rect.h, 0xffff,2,0);
			xfree(lnk_data.payload);
//			USB_printf("-------------------------------------------------\n");
		}
		tick1 = HAL_GetTick() - tick1;
		ST7789VW_DrawBitLine16BPP(&img);
//		USB_printf("%d ms\r\n", tick1);
		//printf("FPS = %.1f\n", (1000.0f / (HAL_GetTick() - tick1)));
	}
err:
	while (1);
}

