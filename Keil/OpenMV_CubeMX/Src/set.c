/* Includes ------------------------------------------------------------------*/
#include "set.h"

/* Private variables ---------------------------------------------------------*/
uint8_t  *PdaRecvBuffer = NULL;
uint8_t  PdaRecvBuffer1[PDA_MSG_LENGTH];
uint16_t PdaRecvCount1 = 0;
uint8_t  PdaRecvOverflowFlag = 0;

uint8_t  PdaMsgBuffer[PDA_MSG_LENGTH];
uint16_t PdaMsgCount = 0;

const uint32_t Bo_Te_Lv[] = {300,600,1200,2400,4800,9600,19200,38400,56000,57600,115200,230400};

const settingParamDef_t    globalSettingParamDefault =
{
    0xA0A0, //uint16_t    u16Inited;              //flash初始化标志
    0121,   //uint16_t    u16BuildNumber;         //编译版本

    115200,   //uint32_t    u32BaudRate;          //波特率
    0,      //uint16_t    u16_485_232;            //0:232 , 1:485
    1,      //uint16_t    u16DevAddr;             //设备地址
    0x01,   //uint16_t    u16HardwareVersion;     //硬件版本信息
    
    0,      //uint8_t     u8WorkMode;             //--0正常状态 --1配置状态
};

settingParamDef_t globalSettingParam = {0};


#define AT_CMD_TAB_SIZE     sizeof(atCmdTab)/sizeof(AT_Cmd_Tab_T)
static AT_Cmd_Tab_T atCmdTab[] =
{
    //FLASH 参数列表0-100
    {0,         PAE_BOTH,        PTE_U32,            (void*)&(globalSettingParam.u32BaudRate)}
    ,{1,        PAE_BOTH,        PTE_U16,            (void*)&(globalSettingParam.u16_485_232)}
    ,{2,        PAE_BOTH,        PTE_U16,            (void*)&(globalSettingParam.u16DevAddr)}
    ,{5,        PAE_BOTH,        PTE_U16,            (void*)&(globalSettingParam.u16HardwareVersion)}

    //FLASH 参数列表101-150 可开放给用户写入的参数
    ,{106,        PAE_BOTH,        PTE_U16,          (void*)&(globalSettingParam.u16BuildNumber)}


    //内存参数列表151-200
    ,{151,        PAE_READ,        PTE_U16,          (void*)&(globalSettingParam.u32BaudRate)}

    
    //执行命令201-255
    ,{201,        PAE_WRITE,        PTE_EXCUTE,      (void*)SystemOperate}
    ,{202,        PAE_READ,        PTE_DATA,         (void*)WatchData}
};


void Write_All_Param(settingParamDef_t *setting)
{
    uint32_t Address = 0 ,PageError = 0;
    uint32_t *pBuffer;
    FLASH_EraseInitTypeDef EraseInitStruct;
    
    //delay_ms(10);
    __disable_irq();
    HAL_FLASH_Unlock();
    
    /* Fill EraseInit structure*/
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.Sector = FLASH_SECTOR_7;
    EraseInitStruct.Banks = FLASH_BANK_2;
    EraseInitStruct.NbSectors = 1;
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3; 

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }
    FLASH_WaitForLastOperation(5000,FLASH_BANK_2);
    
    Address = SETTING_ADDRESS_BASE;
    pBuffer = (uint32_t*)setting;
    for(uint16_t i = 0 ; i < sizeof(settingParamDef_t)/4 ; i ++ )
    {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, Address, *pBuffer) == HAL_OK)
        {
            Address = Address + 4;
            pBuffer ++ ;
        }
        else
        {
            // Error occurred while writing data in Flash memory.
            _Error_Handler(__FILE__, __LINE__);
        }
    }
    HAL_FLASH_Lock();
    __enable_irq();
}

void Read_All_Param(settingParamDef_t *setting)
{
    uint32_t Address = SETTING_ADDRESS_BASE;
    uint32_t *pBuffer = (uint32_t*)setting;

    for(uint16_t i=0 ; i < sizeof(settingParamDef_t)/4 ; i++)
    {
        pBuffer[i] = *(__IO uint32_t *)Address;
        Address += 4;   
    }
}
    
void Init_All_Param(void)
{
    Read_All_Param(&globalSettingParam);
    
    if(globalSettingParam.u16Inited != 0xA0A0 )//出厂默认参数未设置
    {
        Write_All_Param((settingParamDef_t*)&globalSettingParamDefault);
        Read_All_Param(&globalSettingParam);
    }
}

uint8_t SystemOperate(uint8_t* data, uint16_t len)
{
    uint8_t res = 0;
    if(data == NULL) return ARCE_SYS_OP_DATA_EMPTY;

    switch(data[0])
    {
        //退出安装状态
        case '0':
            globalSettingParam.u8WorkMode = 0;
            break;
        //进入安装状态
        case '1':
            globalSettingParam.u8WorkMode = 1;
            break;
        //恢复出厂设置
        case '2':
            Write_All_Param((settingParamDef_t*)&globalSettingParamDefault);
            break;
        //重启设备
        case '3':
            NVIC_SystemReset();
            //delay_ms(100);
            break;
        //保存参数(写入出厂设置)
        case '4':
            ;
            break;
        //保存参数(不写入出厂设置)
        case '5':
            Write_All_Param((settingParamDef_t*)&globalSettingParam);
            break;
        default:
            res = ARCE_SYS_OP_NOT_MATCH;
            break;
    }
    return res;
}
uint8_t WatchData(uint8_t* data, uint16_t len)
{                                       
    return 0;
}


/************AT 命令格式**************
1、读取命令AT+READ=XX,YYY    XX-设备地址(固定为两位的十进制字符串),YYY-参数ID(固定为三位的十进制字符串)
   返回内容+READ:XX,YYY=ZZZ    ZZZ (参数内容)
                                            
2、设置命令AT+WRITE=XX,YYY,ZZZ    XX-设备地址(固定为两位的十进制字符串),YYY-参数ID(固定为三位的十进制字符串)        
                                ZZZ-设置内容(长度不定的字符串，格式取决于参数内容)
   返回内容+WRITE:XX,YYY=ZZZ    ZZZ (设置后的结果)

3、执行命令AT+WRITE:XX,YYY,ZZZ    XX-设备地址(固定为两位的十进制字符串),YYY-参数ID        (固定为三位的十进制字符串)        
                                ZZZ-设置内容(可以为空，或可作为执行动作的输入参数)                
        返回内容+WRITE:XX,YYY    =Z  Z(执行错误码)
                                            
*****************************************/
void AT_Cmd_Process(uint8_t u8CmdUartX)
{
    static uint8_t cmdCnt = 0;
    uint8_t i = 0 ,clearFlag = 0;
    uint8_t offset = 0 , devId = 255 ,paraId = 0;
    uint8_t errCode = 0, excCode = 0;
    PARA_ATTR_E attrReq = PAE_INVALID;
    uint8_t* pos = NULL;
    uint16_t* recvCntAddr = NULL;
    
    //指向数据缓冲区地址
    offset = 0;
    if(u8CmdUartX == 1)
    {
        PdaRecvBuffer = PdaRecvBuffer1;
        recvCntAddr = &PdaRecvCount1;
    }
    else    
    {
        return;
    }
    
    pos = (uint8_t*)memistr((char*)PdaRecvBuffer, PDA_MSG_LENGTH, "AT+");
    
    while(pos != NULL)
    {
        offset += 3;
        clearFlag = 1;
        attrReq = PAE_INVALID;
        memset(PdaMsgBuffer, 0x00, sizeof(PdaMsgBuffer));   //初始化数据
        
        StrToUpper_n((char*)pos + offset, 5);               //判读写操作
        if(strncmp((char*)pos + offset, "READ=", 5) == 0)
        {
            offset += 5;
            attrReq = PAE_READ;
            strcpy((char*)PdaMsgBuffer, "+READ:");
            PdaMsgCount = strlen((char*)PdaMsgBuffer);
        }
        else if(strncmp((char*)pos + offset, "WRITE=", 6) == 0)
        {
            offset += 6;
            attrReq = PAE_WRITE;
            strcpy((char*)PdaMsgBuffer, "+WRITE:");
            PdaMsgCount = strlen((char*)PdaMsgBuffer);
        }
        
        
        devId = str2Digit(pos + offset);                    //设备地址
        offset += 3;                                        //两位设备ID + 1个逗号
        
        if(attrReq == PAE_INVALID)
        {
            errCode = ARCE_OPER_INVALID;                    //操作非法(不是读操作或写操作)
        }
        else if((devId != globalSettingParam.u16DevAddr) && (devId != 99))
        {
            errCode = ARCE_DEVADDR_NOT_MATCH;               //设备地址不匹配
        }
        else
        {
            paraId = str2Digit(pos + offset);               //获取命令ID
            offset += 3;
            if(paraId > 255)
            {   
                errCode = ARCE_PARA_ID_INVALID;             //命令ID 超过限定范围
            }
            
            else
            {
                
                xsprintf((char*)(PdaMsgBuffer + PdaMsgCount), "%02d,%03d=", globalSettingParam.u16DevAddr, paraId);
                PdaMsgCount = strlen((char*)PdaMsgBuffer);
                for(i = 0; i < AT_CMD_TAB_SIZE; i++)
                {
                    if(atCmdTab[i].paraId == paraId)
                    {
                        if(atCmdTab[i].paraAddr == NULL)
                        {
                            xsprintf((char*)(PdaMsgBuffer + PdaMsgCount), "NULL\r\n");
                        }
                        else if((atCmdTab[i].attr == attrReq) || (atCmdTab[i].attr == PAE_BOTH))
                        {
                            if(attrReq == PAE_READ)     //读操作
                            {
                                switch(atCmdTab[i].type)
                                {
                                    case PTE_U8:
                                        xsprintf((char*)(PdaMsgBuffer + PdaMsgCount), "%u\r\n", *((uint8_t*)atCmdTab[i].paraAddr));
                                        break;
                                    case PTE_U16:
                                        xsprintf((char*)(PdaMsgBuffer + PdaMsgCount), "%u\r\n", *((uint16_t*)atCmdTab[i].paraAddr));
                                        break;
                                    case PTE_I16:
                                        xsprintf((char*)(PdaMsgBuffer + PdaMsgCount), "%d\r\n", *((int16_t*)atCmdTab[i].paraAddr));
                                        break;
                                    case PTE_U32:
                                        xsprintf((char*)(PdaMsgBuffer + PdaMsgCount), "%u\r\n", *((uint32_t*)atCmdTab[i].paraAddr));
                                        break;
                                    case PTE_I32:
                                        xsprintf((char*)(PdaMsgBuffer + PdaMsgCount), "%d\r\n", *((int32_t*)atCmdTab[i].paraAddr));
                                        break;
                                    case PTE_STRING:
                                        xsprintf((char*)(PdaMsgBuffer + PdaMsgCount), "%s\r\n", ((char*)atCmdTab[i].paraAddr));
                                        break;
                                    case PTE_DATA:
                                        if(pos[offset++] != ',')
                                        {
                                            errCode = ARCE_DATA_EMPTY;  //操作指令附加数据为空
                                        }
                                        else
                                        {
                                            errCode = ((AT_Cmd_Callback)((uint32_t)(atCmdTab[i].paraAddr)))(pos + offset, *recvCntAddr - offset);
                                        }
                                        break;
                                    default:
                                        break;
                                }
                            }
                            else if(attrReq == PAE_WRITE)   //写操作
                            {
                                if(pos[offset++] != ',')
                                {
                                    errCode = ARCE_DATA_EMPTY;  //写入指令附加数据为空
                                }
                                else
                                {
                                    switch(atCmdTab[i].type)
                                    {
                                        case PTE_U8:
                                            *((uint8_t*)atCmdTab[i].paraAddr) = str2Digit(pos + offset);
                                            xsprintf((char*)(PdaMsgBuffer + PdaMsgCount), "%u\r\n", *((uint8_t*)atCmdTab[i].paraAddr));
                                            break;
                                        case PTE_U16:
                                            *((uint16_t*)atCmdTab[i].paraAddr) = str2Digit(pos + offset);
                                            xsprintf((char*)(PdaMsgBuffer + PdaMsgCount), "%u\r\n", *((uint16_t*)atCmdTab[i].paraAddr));
                                            break;
                                        case PTE_I16:
                                            *((int16_t*)atCmdTab[i].paraAddr) = str2Digit(pos + offset);
                                            xsprintf((char*)(PdaMsgBuffer + PdaMsgCount), "%d\r\n", *((int16_t*)atCmdTab[i].paraAddr));
                                            break;
                                        case PTE_U32:
                                            *((uint32_t*)atCmdTab[i].paraAddr) = str2Digit(pos + offset);
                                            xsprintf((char*)(PdaMsgBuffer + PdaMsgCount), "%u\r\n", *((uint32_t*)atCmdTab[i].paraAddr));
                                            break;
                                        case PTE_I32:
                                            *((int32_t*)atCmdTab[i].paraAddr) = str2Digit(pos + offset);
                                            xsprintf((char*)(PdaMsgBuffer + PdaMsgCount), "%d\r\n", *((int32_t*)atCmdTab[i].paraAddr));
                                            break;
                                        case PTE_EXCUTE:
                                            excCode = ((AT_Cmd_Callback)((uint32_t)(atCmdTab[i].paraAddr)))(pos + offset, *recvCntAddr - offset);
                                            xsprintf((char*)(PdaMsgBuffer + PdaMsgCount), "%d\r\n", excCode);
                                            break;
                                        default:
                                            break; 
                                    }
                                    offset += strlen((char*)(pos + offset));
                                }
                            }
                        }
                        else
                        {
                            errCode = ARCE_AUTORIZE_REFUSE;     //权限不足
                        }
                        break;
                    }
                }
                if(i >= AT_CMD_TAB_SIZE)
                {
                    errCode = ARCE_PARA_ID_NOT_MATCH;           //命令ID 匹配失败
                }
            }
        }            

        if(errCode != 0)    //报错 , 当且仅当是485设备,且是地址错误才不会报错
        {
            memset(PdaMsgBuffer, 0x00, sizeof(PdaMsgBuffer));
            if((globalSettingParam.u16_485_232 != ST_RS485) || (errCode != ARCE_DEVADDR_NOT_MATCH)) 
            {
                xsprintf((char*)PdaMsgBuffer, "+ERROR:%02d,%03d=%d\r\n", devId, paraId, errCode);
            }
        }
        PdaMsgCount = strlen((char*)PdaMsgBuffer);
        if(PdaMsgCount > 0)
        {
            if(u8CmdUartX == 1)
            {
                //UsartPutString(&huart3,PdaMsgBuffer, PdaMsgCount); //串口调试输出
                HAL_UART_Transmit(&huart3,PdaMsgBuffer, PdaMsgCount, 10000);
            }
        }

        cmdCnt++;

        if(cmdCnt >= 25)    break;
        pos = (uint8_t*)memistr((char*)pos + offset, PDA_MSG_LENGTH - offset - (pos - PdaRecvBuffer), "AT+");
        
    }

    if(*recvCntAddr >= PDA_MSG_LENGTH)
    {
        clearFlag = 1;
    }
    
    if(clearFlag == 1)
    {
        memset(PdaRecvBuffer, 0, PDA_MSG_LENGTH);
        *recvCntAddr = 0;
        PdaRecvOverflowFlag = 0;
    }
    
}



/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

