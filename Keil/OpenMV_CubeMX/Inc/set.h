/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_SET_H
#define __BSP_SET_H

/* Includes ------------------------------------------------------------------*/
#include "bsp_mcu.h"
#include "bsp_usart.h"

/* Private define ------------------------------------------------------------*/
#define SETTING_ADDRESS_BASE    0X081E0000      //配置参数 FLASH地址
#define PDA_MSG_LENGTH          128             //修改AT 命令或串口通讯协议时，需要考虑此长度是否足够
typedef uint8_t (*AT_Cmd_Callback)(uint8_t* data, uint16_t len);

typedef enum
{
    ST_RS232 = 0,    
    ST_RS485,
    ST_NONE
}sendType;

typedef enum
{
    PAE_READ = 0x00,    //只读
    PAE_WRITE,          //只写
    PAE_BOTH,           //读写
    PAE_INVALID         //无效
}PARA_ATTR_E;

typedef enum
{
    PTE_U8 = 0x00,        
    PTE_U16,
    PTE_I16,
    PTE_U32,
    PTE_I32,
    PTE_FLOAT,
    PTE_DOUBLE,
    PTE_CH,                //字符
    PTE_STRING,            //字串
    PTE_EXCUTE,            //执行程序-返回执行结果
    PTE_DATA,            //执行程序-在函数中获取指定的数据
    PTE_INVALID
}PARA_TYPE_E;

typedef enum
{
    ARCE_NONE = 0,
    ARCE_OPER_INVALID = 1,            //操作非法(不是读操作或写操作)
    ARCE_DEVADDR_NOT_MATCH = 2,        //设备地址不匹配
    ARCE_PARA_ID_INVALID = 3,        //命令ID 超过限定范围
    ARCE_PARA_ID_NOT_MATCH = 4,        //命令ID 匹配失败
    ARCE_AUTORIZE_REFUSE = 5,        //权限不足
    ARCE_DATA_EMPTY = 6,            //写入或操作指令附加数据为空
    ARCE_KEY_PARA_INVALID = 8,        //参数超出限定范围
    ARCE_SYS_OP_DATA_EMPTY = 10,    //系统操作子命令为空
    ARCE_SYS_OP_NOT_MATCH = 11,        //系统操作子命令匹配失败
}AT_RES_CODE_E;

typedef struct
{
    uint8_t            paraId;            //参数ID
    PARA_ATTR_E        attr;            //读写标志
    PARA_TYPE_E        type;           //数据类型
    void*            paraAddr;        //参数在内存中的地址，或回调函数地址
}AT_Cmd_Tab_T;


//配置参数定义
typedef struct _settingParamDef
{
    uint16_t    u16Inited;                //flash初始化标志
    uint16_t    u16BuildNumber;         //编译版本
    
    uint32_t    u32BaudRate;            //波特率
    uint16_t    u16_485_232;            //232-485
    uint16_t    u16DevAddr;             //设备地址     
    uint16_t    u16HardwareVersion;     //硬件版本信息 

    uint8_t     u8WorkMode;             //--0正常状态 --1配置状态   

    

}settingParamDef_t;

extern settingParamDef_t globalSettingParam;
extern uint8_t     PdaRecvBuffer1[PDA_MSG_LENGTH];
extern uint16_t PdaRecvCount1;
void AT_Cmd_Process(uint8_t u8CmdUartX);

void Write_All_Param(settingParamDef_t *setting);
void Read_All_Param(settingParamDef_t *setting);
void Init_All_Param(void);

uint8_t SystemOperate(uint8_t* data, uint16_t len);
uint8_t WatchData(uint8_t* data, uint16_t len);
#endif /* __BSP_SET_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

