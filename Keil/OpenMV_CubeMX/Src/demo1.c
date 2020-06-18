#include <string.h>

#include "ff_gen_drv.h"
#include "sd_diskio.h"

#include "framebuffer.h"
#include "sensor.h"
#include "imlib.h"

extern FATFS fs;
extern FIL file;
extern char FsReadBuf[];
extern char FsWriteBuf[];
extern uint8_t g_TestBuf[];

extern DIR DirInf;
extern FILINFO FileInf;
extern char DiskPath[4]; /* SD卡逻辑驱动路径，比盘符0，就是"0:/" */

extern const char * FR_Table[];


void OpenMV_test1(void)
{
    int ret;
    image_t img;
    image_t img2;
    int i;
    int cnt = 0;
    rectangle_t roi;
    array_t *blobs;
    list_t out;
    rectangle_t *r;
    static int vf_state = 0;
    
    char path[64];
    
    fb_alloc_init0();
    ret = sensor_init();
    if (ret < 0) {
        printf("sensor_init return %d\n", ret);
        while(1);
    }

    ret = sensor_reset();
    if (ret < 0) {
        printf("sensor_reset return %d\n", ret);
        while(1);
    }
    
    ret = sensor_set_pixformat(PIXFORMAT_RGB565);
    if (ret < 0) {
        printf("sensor_set_pixformat return %d\n", ret);
        while(1);
    }
    
    ret = sensor_set_framesize(FRAMESIZE_QQVGA2);
    if (ret < 0) {
        printf("sensor_set_framesize return %d\n", ret);
        while(1);
    }

    printf("Running example_find_blobs\n");
    
    ret = sensor_snapshot_simple(&img);
    if (ret != 0) 
    {
        printf("sensor_snapshot_simple failed!\n");
        while(1);
    }
    
    list_t thresholds;
    color_thresholds_list_lnk_data_t lnk_data;
    list_init(&thresholds, sizeof(color_thresholds_list_lnk_data_t));
    
    // 红色的LAB上限、下限
    lnk_data.LMin = 25;
    lnk_data.LMax = 50;
    lnk_data.AMin = 40;
    lnk_data.AMax = 70;
    lnk_data.BMin = 20;
    lnk_data.BMax = 60;

    list_push_back(&thresholds, &lnk_data);
    
    FATFS_LinkDriver(&SD_Driver, DiskPath);
    f_mount(&fs, DiskPath, 0);
    
    while (1)
    {
        Loop_Begin(500)
        {
            //printf("nihao ");
            //LED_GREEN_Toggle;
        }Loop_End();
        
        ret = sensor_snapshot_simple(&img);
        if (ret != 0) 
        {
            printf("sensor_snapshot_simple failed!\n");
            while(1);
        }
        
        roi.x = 0;
        roi.y = 0;
        roi.w = img.w;
        roi.h = img.h;
#if 0
        if (vf_state == 0) {
            imlib_find_blobs(&out, &img, &roi, 2, 1, 
                &thresholds, 0, 10, 10,
                0, 0,
                NULL, NULL,
                NULL, NULL,NULL,NULL);

            for (size_t i = 0; list_size(&out); i++) {
                    find_blobs_list_lnk_data_t tmp_lnk_data;
                    list_pop_front(&out, &tmp_lnk_data);
                
                    imlib_draw_rectangle(&img,
                                         tmp_lnk_data.rect.x, 
                                         tmp_lnk_data.rect.y, 
                                         tmp_lnk_data.rect.w,
                                         tmp_lnk_data.rect.h, 0xffff,2,0);

                }
            }
#endif        
        //imlib_draw_rectangle(&img,10, 10, 100,100, 0xffff,2,0);
        //ST7735R_DrawBitLine16BPP(&img);
        //USART3_TransmitImage(&img);
       
            
        sprintf(path, "%sdemo%d.bmp", DiskPath,cnt++);
            
        imlib_save_image(&img, path, &roi, 1);
        printf("imlib_save_image:%s",path);
        delay_ms(1000);     
        imlib_load_image(&img2,path);
        printf("imlib_load_image:%s",path);
        ST7735R_DrawBitLine16BPP(&img2);

    }
    f_mount(NULL, DiskPath, 0);
    
}



void OpenMV_test2(void)
{
    FRESULT result;
    uint32_t cnt = 0;
    FILINFO fno;
    image_t img;
    char path[64];
    
    FATFS_LinkDriver(&SD_Driver, DiskPath);
    
    f_mount(&fs, DiskPath, 0);
    sprintf(path, "%sdemo1.bmp", DiskPath);
    printf("%s",path);
//    result = f_open(&file, path, FA_OPEN_EXISTING | FA_READ);
    //result = file_read_open(&file, path
//    if (result !=  FR_OK)
//    {
//        printf("Don't Find File : demo1.bmp\r\n");
//    }
//    else
//    {
//        printf("Find File : demo1.bmp\r\n");
//    }
//    f_close(&file);
//    printf("关闭成功\r\n");
    
    //bmp_read(&img,path);
    imlib_load_image(&img,path);
    
    printf("img.h = %d, img.w=%d",img.h,img.w);
    ST7735R_DrawBitLine16BPP(&img);
    //f_mount(NULL, DiskPath, 0);
    
    while(1);
    
}