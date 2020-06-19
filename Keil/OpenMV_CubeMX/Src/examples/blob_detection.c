/*
 * This file is part of the Micro Vision Device MVD project.
 * Copyright (c) 2017 Micro Vision Device [http://www.mvdevice.com]
 *
 * by yuanjun<yuanjun2006@outlook.com>
 *
 * blob detection example.
 *
 */

#include "stm32h7xx_hal.h"
#include "imlib.h"
#include "bsp_mcu.h"
#include "sensor.h"

static int key_state = 0;
static int key_event = 0;

static int vf_state = 0;
static int vf_w = 30;
static int vf_h = 30;

static simple_color_t vf_lt;
static simple_color_t vf_ht;

/* yuanjun <yuanjun2006@outlook.com>
 * 
 * 找颜色块例子(默认红色)
 */
image_t img;
rectangle_t roi;
array_t *blobs;
list_t out;
list_t thresholds;
color_thresholds_list_lnk_data_t lnk_data;

rectangle_t *r;
void example_blob_detection(void)
{
	int i;
	int cnt;
	list_t out;
	int tick1;
	int ret;
	image_t img;
	rectangle_t roi;
	array_t *blobs;
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

//	example_blob_detection();
//	example_qrcodes();
    DEBUG("Running example_find_blobs\n");
    
    list_init(&thresholds, sizeof(color_thresholds_list_lnk_data_t));
    // 红色的LAB上限、下限
    lnk_data.LMin = 25;
    lnk_data.AMin = 40;
    lnk_data.BMin = 20;
	lnk_data.LMax = 100;
	lnk_data.AMax = 100;
	lnk_data.BMax = 100;
    list_push_back(&thresholds, &lnk_data);
    
    while (1)
    {
		UG_Update();
//        Loop_Begin(10)
//        {
			tick1 = HAL_GetTick();
            LED_Power_Ctrl(LED_GREEN, 2);
             
            ret = sensor_snapshot_simple(&img);
            if (ret != 0) 
            {
                DEBUG("sensor_snapshot_simple failed!\n");
                while(1);
            }
            roi.x = 0;
			roi.y = 0;
            roi.w = img.w;
			roi.h = img.h;

            imlib_find_blobs(&out, &img, &roi, 2, 1, &thresholds, 0, 10, 10,0, 0,NULL, NULL,NULL, NULL,NULL,NULL);
            for (size_t i = 0; list_size(&out); i++) 
            {
                find_blobs_list_lnk_data_t tmp_lnk_data;
                list_pop_front(&out, &tmp_lnk_data);
                imlib_draw_rectangle(&img,tmp_lnk_data.rect.x, tmp_lnk_data.rect.y, tmp_lnk_data.rect.w,tmp_lnk_data.rect.h, 0xffff,2,0);
				imlib_draw_string(&img, 10, 10, "Find", 0xFF00, 3, 10, 10, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
            }
            ST7789VW_DrawBitLine16BPP(&img);  
			//USB_printf("USB_DEVICE_Init OK!!\r\n");
//			USB_printf("%d ms\r\n", ((HAL_GetTick() - tick1)));
//			USB_printf("FPS:%.2f\r\n",1000.0f/(HAL_GetTick() - tick1));
//			sprintf(show_buf,"FPS:%.2f",1000.0f/(HAL_GetTick() - tick1));
//			UG_PutString(0,190,show_buf);
//        }Loop_End()
    }
}

