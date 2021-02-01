/* Includes ------------------------------------------------------------------*/
#include "SDFatfs.h"
//#include "main.h"

typedef struct _SDParamDef
{
    FATFS fs;
    char DiskPath[4];   //SD���߼�����·�������̷�0������"0:/"
    FRESULT result;
    FIL file;    
}SDParamDef_t;

SDParamDef_t g_tSD =
{
    {0},//FATFS fs;
    {0},//char DiskPath[4];   //SD���߼�����·�������̷�0������"0:/"
    FR_OK,//FRESULT result;
    {0},//FIL file;  
};

#define SD_RB_SIZE             1*1024
uint8_t u8SDWriteBuffer[SD_RB_SIZE] = {0};


/* FatFs API�ķ���ֵ */
const char * FR_Table[]= 
{
    "FR_OK���ɹ�",                                             /* (0) Succeeded */
    "FR_DISK_ERR���ײ�Ӳ������",                             /* (1) A hard error occurred in the low level disk I/O layer */
    "FR_INT_ERR������ʧ��",                                     /* (2) Assertion failed */
    "FR_NOT_READY����������û�й���",                         /* (3) The physical drive cannot work */
    "FR_NO_FILE���ļ�������",                                 /* (4) Could not find the file */
    "FR_NO_PATH��·��������",                                 /* (5) Could not find the path */
    "FR_INVALID_NAME����Ч�ļ���",                             /* (6) The path name format is invalid */
    "FR_DENIED�����ڽ�ֹ���ʻ���Ŀ¼�������ʱ��ܾ�",         /* (7) Access denied due to prohibited access or directory full */
    "FR_EXIST���ļ��Ѿ�����",                                 /* (8) Access denied due to prohibited access */
    "FR_INVALID_OBJECT���ļ�����Ŀ¼������Ч",                 /* (9) The file/directory object is invalid */
    "FR_WRITE_PROTECTED������������д����",                     /* (10) The physical drive is write protected */
    "FR_INVALID_DRIVE���߼���������Ч",                         /* (11) The logical drive number is invalid */
    "FR_NOT_ENABLED�������޹�����",                             /* (12) The volume has no work area */
    "FR_NO_FILESYSTEM��û����Ч��FAT��",                     /* (13) There is no valid FAT volume */
    "FR_MKFS_ABORTED�����ڲ�������f_mkfs()����ֹ",             /* (14) The f_mkfs() aborted due to any parameter error */
    "FR_TIMEOUT���ڹ涨��ʱ�����޷���÷��ʾ�����",         /* (15) Could not get a grant to access the volume within defined period */
    "FR_LOCKED�������ļ�������Բ������ܾ�",                 /* (16) The operation is rejected according to the file sharing policy */
    "FR_NOT_ENOUGH_CORE���޷����䳤�ļ���������",             /* (17) LFN working buffer could not be allocated */
    "FR_TOO_MANY_OPEN_FILES����ǰ�򿪵��ļ�������_FS_SHARE", /* (18) Number of open files > _FS_SHARE */
    "FR_INVALID_PARAMETER��������Ч"                         /* (19) Given parameter is invalid */
};

//�� �� �� : f_deldir
//�������� : �Ƴ�һ���ļ��У������䱾��������ļ��У����ļ�
//��    �� : const TCHAR *path---ָ��Ҫ�Ƴ��Ŀս�β�ַ��������ָ��
//��    �� : ��
//�� �� ֵ : FR_OK(0)��           �����ɹ� 
//           FR_NO_FILE��         �޷��ҵ��ļ���Ŀ¼ 
//           FR_NO_PATH��         �޷��ҵ�·�� 
//           FR_INVALID_NAME��    �ļ����Ƿ� 
//           FR_INVALID_DRIVE��   �������ŷǷ� 
//           FR_DENIED��          ������������ԭ�򱻾ܾ��� 
//               ��������Ϊֻ����
//               Ŀ¼�·ǿգ�
//               ��ǰĿ¼��
//           FR_NOT_READY��       �����������޷�������������������û��ý�������ԭ�� 
//           FR_WRITE_PROTECTED�� ý��д���� 
//           FR_DISK_ERR��        ����ʧ�����ڴ������е�һ������ 
//           FR_INT_ERR��         ����ʧ������һ������� FAT �ṹ���ڲ����� 
//           FR_NOT_ENABLED��     �߼�������û�й����� 
//           FR_NO_FILESYSTEM��   ��������û�кϷ��� FAT �� 
//           FR_LOCKED��          �������������ļ�������ƣ�_FS_SHARE�� 
//��    ע : f_deldir ���������Ƴ�һ���ļ��м������ļ��С����ļ����������Ƴ��Ѿ��򿪵Ķ��� 
//====================================================================================================

FRESULT f_deldir(const TCHAR *path)
{
    FRESULT res;
    DIR   dir;     /* �ļ��ж��� */ //36  bytes
    FILINFO fno;   /* �ļ����� */   //32  bytes
    TCHAR file[_MAX_LFN + 2] = {0};

    //���ļ���
    res = f_opendir(&dir, path);
    
    //������ȡ�ļ�������
    while((res == FR_OK) && (FR_OK == f_readdir(&dir, &fno)))
    {
        //����"."��".."�ļ��У�����
        if(0 == strlen(fno.fname))          break;      //���������ļ���Ϊ��
        if(0 == strcmp(fno.fname, "."))     continue;   //���������ļ���Ϊ��ǰ�ļ���
        if(0 == strcmp(fno.fname, ".."))    continue;   //���������ļ���Ϊ��һ���ļ���
        
        memset(file, 0, sizeof(file));
        sprintf((char*)file, "%s/%s", path, fno.fname);
        if (fno.fattrib & AM_DIR)
        {//�����ļ��У��ݹ�ɾ��
            res = f_deldir(file);
        }
        else
        {//�����ļ���ֱ��ɾ��
            res = f_unlink(file);
        }
    }
    
    //ɾ������
    if(res == FR_OK)    res = f_unlink(path);
    
    return res;
}
    
void SDCard_Init(void)
{
    char path[64];
    
    SDMMC1_SD_Init();
    FATFS_LinkDriver(&SD_Driver, g_tSD.DiskPath);     //ע��SD������
    delay_ms(10);
        g_tSD.result = f_mount(NULL, g_tSD.DiskPath, 0);    //ж���ļ�ϵͳ 
        if (g_tSD.result != FR_OK)    goto __exit;
    g_tSD.result = f_mount(&g_tSD.fs, g_tSD.DiskPath, 0);        //�����ļ�ϵͳ 
    if (g_tSD.result != FR_OK)    goto __exit;
  
    
    //����Ŀ¼
    memset(path , 0x00 , 64);
    sprintf(path, "%sDEBUG_Log", g_tSD.DiskPath);
    g_tSD.result = f_mkdir(path);   //����DEBUGĿ¼
    if (!(g_tSD.result == FR_OK || g_tSD.result == FR_EXIST))    goto __exit;
    
    memset(path , 0x00 , 64);
    sprintf(path, "%sModbus_Data", g_tSD.DiskPath);
    g_tSD.result = f_mkdir(path);   //����Modbus_DataĿ¼
    if (!(g_tSD.result == FR_OK || g_tSD.result == FR_EXIST))    goto __exit;
    
    memset(path , 0x00 , 64);
    sprintf(path, "%sJson_Data", g_tSD.DiskPath);
    g_tSD.result = f_mkdir(path);   //����Json_DataĿ¼
    if (!(g_tSD.result == FR_OK || g_tSD.result == FR_EXIST))    goto __exit;
    
    
    SDCard_ViewRootDir();
    
    g_tSD.result = FR_OK;
__exit:
    if (g_tSD.result != FR_OK)
    {
        DEBUG("DEBUG :(%s)", FR_Table[g_tSD.result]);
        f_mount(NULL, g_tSD.DiskPath, 0);    //ж���ļ�ϵͳ 
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

    result = f_opendir(&DirInf, g_tSD.DiskPath);  //�򿪸��ļ���
    if(result!= FR_OK)    goto __exit;
    DEBUG("����  |  �ļ���СKiB | ���ļ��� | ���ļ���\r\n");
    for (uint32_t cnt = 0; ;cnt++)
    {
        result = f_readdir(&DirInf, &FileInf);                //��ȡĿ¼��������Զ����� 
        if(result!= FR_OK)    goto __exit;
        if (FileInf.fname[0] == 0)   break;
        if (FileInf.fname[0] == '.') continue;
        if (FileInf.fattrib & AM_DIR)   
        {
            DEBUG("Ŀ¼    ");    //�ж����ļ�������Ŀ¼
        }
        else   
        {
            DEBUG("�ļ�    ");
        }

        f_stat(FileInf.fname, &fno);
        DEBUG(" %6dKiB   ", (int)fno.fsize>>10); ///��ӡ�ļ���С, ���4G 
        DEBUG("  %s\r\n", (char *)FileInf.fname);    //���ļ��� 
    }
    //��ӡ������ ��ʣ������
    /* Get volume information and free clusters of drive 1 */
    f_getfree(g_tSD.DiskPath, &fre_clust, &fs);

    /* Get total sectors and free sectors */
    tot_sect = (fs->n_fatent - 2) * fs->csize;
    fre_sect = fre_clust * fs->csize;

    /* Print the free space (assuming 512 bytes/sector) */
    DEBUG("%d MB total drive space.\n%d MB available.\n", tot_sect / 2 >>10, fre_sect / 2 >>10);
    
    /* ��ӡ���ٶ���Ϣ */    
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
