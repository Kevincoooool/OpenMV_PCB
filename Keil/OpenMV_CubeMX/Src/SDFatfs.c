/* Includes ------------------------------------------------------------------*/
#include "SDFatfs.h"
//#include "main.h"

typedef struct _SDParamDef
{
    FATFS fs;
    char DiskPath[4];   //SD卡逻辑驱动路径，比盘符0，就是"0:/"
    FRESULT result;
    FIL file;    
}SDParamDef_t;

SDParamDef_t g_tSD =
{
    {0},//FATFS fs;
    {0},//char DiskPath[4];   //SD卡逻辑驱动路径，比盘符0，就是"0:/"
    FR_OK,//FRESULT result;
    {0},//FIL file;  
};

#define SD_RB_SIZE             1*1024
uint8_t u8SDWriteBuffer[SD_RB_SIZE] = {0};


/* FatFs API的返回值 */
const char * FR_Table[]= 
{
    "FR_OK：成功",                                             /* (0) Succeeded */
    "FR_DISK_ERR：底层硬件错误",                             /* (1) A hard error occurred in the low level disk I/O layer */
    "FR_INT_ERR：断言失败",                                     /* (2) Assertion failed */
    "FR_NOT_READY：物理驱动没有工作",                         /* (3) The physical drive cannot work */
    "FR_NO_FILE：文件不存在",                                 /* (4) Could not find the file */
    "FR_NO_PATH：路径不存在",                                 /* (5) Could not find the path */
    "FR_INVALID_NAME：无效文件名",                             /* (6) The path name format is invalid */
    "FR_DENIED：由于禁止访问或者目录已满访问被拒绝",         /* (7) Access denied due to prohibited access or directory full */
    "FR_EXIST：文件已经存在",                                 /* (8) Access denied due to prohibited access */
    "FR_INVALID_OBJECT：文件或者目录对象无效",                 /* (9) The file/directory object is invalid */
    "FR_WRITE_PROTECTED：物理驱动被写保护",                     /* (10) The physical drive is write protected */
    "FR_INVALID_DRIVE：逻辑驱动号无效",                         /* (11) The logical drive number is invalid */
    "FR_NOT_ENABLED：卷中无工作区",                             /* (12) The volume has no work area */
    "FR_NO_FILESYSTEM：没有有效的FAT卷",                     /* (13) There is no valid FAT volume */
    "FR_MKFS_ABORTED：由于参数错误f_mkfs()被终止",             /* (14) The f_mkfs() aborted due to any parameter error */
    "FR_TIMEOUT：在规定的时间内无法获得访问卷的许可",         /* (15) Could not get a grant to access the volume within defined period */
    "FR_LOCKED：由于文件共享策略操作被拒绝",                 /* (16) The operation is rejected according to the file sharing policy */
    "FR_NOT_ENOUGH_CORE：无法分配长文件名工作区",             /* (17) LFN working buffer could not be allocated */
    "FR_TOO_MANY_OPEN_FILES：当前打开的文件数大于_FS_SHARE", /* (18) Number of open files > _FS_SHARE */
    "FR_INVALID_PARAMETER：参数无效"                         /* (19) Given parameter is invalid */
};

//函 数 名 : f_deldir
//函数功能 : 移除一个文件夹，包括其本身和其子文件夹，子文件
//输    入 : const TCHAR *path---指向要移除的空结尾字符串对象的指针
//输    出 : 无
//返 回 值 : FR_OK(0)：           函数成功 
//           FR_NO_FILE：         无法找到文件或目录 
//           FR_NO_PATH：         无法找到路径 
//           FR_INVALID_NAME：    文件名非法 
//           FR_INVALID_DRIVE：   驱动器号非法 
//           FR_DENIED：          函数由于以下原因被拒绝： 
//               对象属性为只读；
//               目录下非空；
//               当前目录。
//           FR_NOT_READY：       磁盘驱动器无法工作，由于驱动器中没有媒体或其他原因 
//           FR_WRITE_PROTECTED： 媒体写保护 
//           FR_DISK_ERR：        函数失败由于磁盘运行的一个错误 
//           FR_INT_ERR：         函数失败由于一个错误的 FAT 结构或内部错误 
//           FR_NOT_ENABLED：     逻辑驱动器没有工作区 
//           FR_NO_FILESYSTEM：   驱动器上没有合法的 FAT 卷 
//           FR_LOCKED：          函数被拒由于文件共享机制（_FS_SHARE） 
//备    注 : f_deldir 函数用来移除一个文件夹及其子文件夹、子文件，但不能移除已经打开的对象。 
//====================================================================================================

FRESULT f_deldir(const TCHAR *path)
{
    FRESULT res;
    DIR   dir;     /* 文件夹对象 */ //36  bytes
    FILINFO fno;   /* 文件属性 */   //32  bytes
    TCHAR file[_MAX_LFN + 2] = {0};

    //打开文件夹
    res = f_opendir(&dir, path);
    
    //持续读取文件夹内容
    while((res == FR_OK) && (FR_OK == f_readdir(&dir, &fno)))
    {
        //若是"."或".."文件夹，跳过
        if(0 == strlen(fno.fname))          break;      //若读到的文件名为空
        if(0 == strcmp(fno.fname, "."))     continue;   //若读到的文件名为当前文件夹
        if(0 == strcmp(fno.fname, ".."))    continue;   //若读到的文件名为上一级文件夹
        
        memset(file, 0, sizeof(file));
        sprintf((char*)file, "%s/%s", path, fno.fname);
        if (fno.fattrib & AM_DIR)
        {//若是文件夹，递归删除
            res = f_deldir(file);
        }
        else
        {//若是文件，直接删除
            res = f_unlink(file);
        }
    }
    
    //删除本身
    if(res == FR_OK)    res = f_unlink(path);
    
    return res;
}
    
void SDCard_Init(void)
{
    char path[64];
    
    SDMMC1_SD_Init();
    FATFS_LinkDriver(&SD_Driver, g_tSD.DiskPath);     //注册SD卡驱动
    delay_ms(10);
        g_tSD.result = f_mount(NULL, g_tSD.DiskPath, 0);    //卸载文件系统 
        if (g_tSD.result != FR_OK)    goto __exit;
    g_tSD.result = f_mount(&g_tSD.fs, g_tSD.DiskPath, 0);        //挂载文件系统 
    if (g_tSD.result != FR_OK)    goto __exit;
  
    /*
    //创建目录
    memset(path , 0x00 , 64);
    sprintf(path, "%sDEBUG_Log", g_tSD.DiskPath);
    g_tSD.result = f_mkdir(path);   //创建DEBUG目录
    if (!(g_tSD.result == FR_OK || g_tSD.result == FR_EXIST))    goto __exit;
    
    memset(path , 0x00 , 64);
    sprintf(path, "%sModbus_Data", g_tSD.DiskPath);
    g_tSD.result = f_mkdir(path);   //创建Modbus_Data目录
    if (!(g_tSD.result == FR_OK || g_tSD.result == FR_EXIST))    goto __exit;
    
    memset(path , 0x00 , 64);
    sprintf(path, "%sJson_Data", g_tSD.DiskPath);
    g_tSD.result = f_mkdir(path);   //创建Json_Data目录
    if (!(g_tSD.result == FR_OK || g_tSD.result == FR_EXIST))    goto __exit;
    */
    
    SDCard_ViewRootDir();
    
    g_tSD.result = FR_OK;
__exit:
    if (g_tSD.result != FR_OK)
    {
        DEBUG("DEBUG :(%s)\r\n", FR_Table[g_tSD.result]);
        f_mount(NULL, g_tSD.DiskPath, 0);    //卸载文件系统 
    }
}
uint8_t SDCard_ViewRootDir(void)
{
    FATFS *fs;
    DWORD fre_clust, fre_sect, tot_sect;
    FRESULT result = FR_OK;
    FILINFO FileInf;
    DIR DirInf  ={0};
    FILINFO fno = {0};
    
    DEBUG("DEBUG:SD card initialization, display a list of files in the root directory.\r\n");

    result = f_opendir(&DirInf, g_tSD.DiskPath);  //打开根文件夹
    if(result!= FR_OK)    goto __exit;
    DEBUG("属性  |  文件大小KiB | 短文件名 | 长文件名\r\n");
    for (uint32_t cnt = 0; ;cnt++)
    {
        result = f_readdir(&DirInf, &FileInf);                //读取目录项，索引会自动下移 
        if(result!= FR_OK)    goto __exit;
        if (FileInf.fname[0] == 0)   break;
        if (FileInf.fname[0] == '.') continue;
        if (FileInf.fattrib & AM_DIR)   
        {
            DEBUG("目录    ");    //判断是文件还是子目录
        }
        else   
        {
            DEBUG("文件    ");
        }

        f_stat(FileInf.fname, &fno);
        DEBUG(" %6dKiB   ", (int)fno.fsize>>10); ///打印文件大小, 最大4G 
        DEBUG("  %s\r\n", (char *)FileInf.fname);    //长文件名 
    }
    //打印卡容量 和剩余容量
    /* Get volume information and free clusters of drive 1 */
    f_getfree(g_tSD.DiskPath, &fre_clust, &fs);

    /* Get total sectors and free sectors */
    tot_sect = (fs->n_fatent - 2) * fs->csize;
    fre_sect = fre_clust * fs->csize;

    /* Print the free space (assuming 512 bytes/sector) */
    DEBUG("%d MB total drive space.\n%d MB available.\n", tot_sect / 2 >>10, fre_sect / 2 >>10);
    
    /* 打印卡速度信息 */    
    if(hsd1.SdCard.CardType == CARD_SDSC)
    {
        DEBUG("Normal Speed Card <12.5MB/S, MAX Clock < 25MHz, Spec Version 1.01\r\n");           
    }
    else if (hsd1.SdCard.CardType == CARD_SDHC_SDXC)
    {
        DEBUG("High Speed Card <25MB/s, MAX Clock < 50MHz, Spec Version 2.00\r\n");            
    }
    else if (hsd1.SdCard.CardType == CARD_SECURED)
    {
        DEBUG("UHS-I SD Card <50MB/S for SDR50, DDR50 Cards, MAX Clock < 50MHz OR 100MHz\r\n");
        DEBUG("UHS-I SD Card <104MB/S for SDR104, MAX Clock < 108MHz, Spec version 3.01\r\n");   
    }    
    
    result = FR_OK;
    
__exit:
    
    if (result != FR_OK)
    {
        DEBUG("DEBUG :SDCard_ViewRootDir false.\r\n(%s)\r\n", FR_Table[result]);
        return false;
    }

    return true;
}



/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
