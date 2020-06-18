/*
 * This file is part of the Micro Vision Device MVD project.
 * Copyright (c) 2017 Micro Vision Device [http://www.mvdevice.com]
 *
 * by yuanjun<yuanjun2006@outlook.com>
 *
 * face detection example.
 *
 */

#include <stdlib.h>
#include "stm32h7xx_hal.h"
#include "imlib.h"
#include "bsp_mcu.h"
#include "sensor.h"
#include "omv_boardconfig.h"

int imlib_load_cascade_from_flash(cascade_t *cascade, const char *path);

/* yuanjun <yuanjun2006@outlook.com> 
 * 
 * 人脸检测例子
 */

void example_face_detection(void)
{
	int tick1;
	int i;
	int cnt;
	rectangle_t *r;
	
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
    
    ret = sensor_set_pixformat(PIXFORMAT_RGB565);
    if (ret < 0) {
        DEBUG("sensor_set_pixformat return %d\n", ret);
        while(1);
    }
    
    ret = sensor_set_framesize(FRAMESIZE_HQVGA);
    if (ret < 0) {
        DEBUG("sensor_set_framesize return %d\n", ret);
        while(1);
    }
	
	/* yuanjun : 加载人脸数据*/
	ret = imlib_load_cascade(&face_cascade, "frontalface");
	if (ret) {
		DEBUG("[Failed] imlib_load_cascade_from_flash\n");
		LED_Power_Ctrl(LED_GREEN, 2);
	}

	face_cascade.n_stages = 25;
	
	DEBUG("Running example_face_detection\n");

	delay_ms(1000);

	while (1) {
		tick1 = HAL_GetTick();
		ret = sensor_snapshot_simple(&img);
		
		if (ret != 0) {
			// printf("sensor_snapshot_simple failed!\n");
			// mdelay(1);
			continue;
		}

		roi.x = 0;
		roi.y = 0;
		roi.w = img.w;
		roi.h = img.h;

		face_cascade.threshold = 0.5f;
		face_cascade.scale_factor = 1.5f;

		objects_array = imlib_detect_objects(&img, &face_cascade, &roi);
		if (!objects_array) {
			/* 没有找到人脸*/
		} else {
			/* 找到人脸*/
			cnt = array_length(objects_array);
			for (i = 0; i < cnt; i++) {
				r = (rectangle_t *)array_pop_back(objects_array);
				imlib_draw_rectangle(&img,
									 r->x, r->y, r->w, r->h, 0xffff,2,0);
				//printf("[%d, %d, %d, %d]\n", cb->x, cb->y, cb->w, cb->h);
				free(r);
			}
			
			array_free(objects_array);
		}
		ST7789VW_DrawBitLine16BPP(&img);
//		USB_printf("%d ms\n", ((HAL_GetTick() - tick1)));
//		//printf("fps = %01f", (1000.0f / (HAL_GetTick() - tick1)));
	}
}

