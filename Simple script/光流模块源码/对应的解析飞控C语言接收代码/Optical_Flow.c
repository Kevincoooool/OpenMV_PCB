/* ©2015-2016 Beijing Bechade Opto-Electronic, Co.,Ltd. All rights reserved.
 + 文件名  ：Optical_Flow.c
 + 描述    ：光流数据处理
代码尽量做到逐行注释 代码有问题，及时加群交流 作者有偿支持对开源代码的完善 */
#include "stm32f4xx.h"
#include "Optical_Flow.h"
#include "driver_usart.h"

/* 发送帧头 接收帧头 */
#define title1_received 0xAA
#define title2_received 0xAE

/* 读取X偏移量 读取Y偏移量 读取置信度 */
int Optical_flow_org_x,Optical_flow_org_y,Optical_flow_confidence;
float Optical_flow_x,Optical_flow_y,Optical_flow_con;

/*----------------------------------------------------------
 + 实现功能：数据分析
 + 调用参数：接收到的单字节数据
----------------------------------------------------------*/
void Optical_Flow_Receive_Anl(u8 *data_buf,u8 num)
{
	/* 读取X偏移量原始数据  */
	Optical_flow_org_x = (int)(*(data_buf+1)<<0) | (int)(*(data_buf+2)<<8) | (int)(*(data_buf+3)<<16) | (int)(*(data_buf+4)<<24) ;
	/* 换算到X像素速度  光流像素要结合高度才能算出速度 值范围约+-200.000 */
	Optical_flow_x = (float)((double)Optical_flow_org_x*0.001);

	/* 读取Y偏移量原始数据 */
	Optical_flow_org_y = (int)(*(data_buf+5)<<0) | (int)(*(data_buf+6)<<8) | (int)(*(data_buf+7)<<16) | (int)(*(data_buf+8)<<24) ;
	/* 换算到Y像素速度  光流像素要结合高度才能算出速度 值范围约+-200.000 */
	Optical_flow_y = (float)((double)Optical_flow_org_y*0.001);

	/* 读取Confidence原始数据 */
	Optical_flow_confidence = (int)(*(data_buf+9)<<0) | (int)(*(data_buf+10)<<8) | (int)(*(data_buf+11)<<16) | (int)(*(data_buf+12)<<24) ;
    /* 读取画面不变率 值范围0-100 */
    Optical_flow_con = (float)((double)Optical_flow_confidence*0.001);
}

/*----------------------------------------------------------
 + 实现功能：数据接收并保存
 + 调用参数：接收到的单字节数据
----------------------------------------------------------*/
void Optical_Flow_Receive_Prepare(u8 data)
{
    /* 局部静态变量：接收缓存 */
    static u8 RxBuffer[14];
    /* 数据长度 *//* 数据数组下标 */
    static u8  _data_cnt = 0;
    /* 接收状态 */
    static u8 state = 0;

    /* 帧头1 */
    if(state==0&&data==title1_received)
    {
        state=1;
    }
    /* 帧头2 */
    else if(state==1&&data==title2_received)
    {
        state=2;
		_data_cnt = 0;
    }
    /* 接收数据租 */
    else if(state==2)
    {
        RxBuffer[++_data_cnt]=data;
        if(_data_cnt>=12)
		{
			state = 0;
			Optical_Flow_Receive_Anl(RxBuffer,_data_cnt);
		}
    }
    /* 若有错误重新等待接收帧头 */
    else
        state = 0;
}


/*----------------------------------------------------------
 + 实现功能：光流初始化
----------------------------------------------------------*/
void Optical_Flow_init()
{
    /*----------------------------------------------------------
     + 实现功能：串口2初始化
     + 调用参数功能：
     - u32 bound：波特率 38400
     - u8 Priority：中断主优先级 2
     - u8 SubPriority：中断从优先级 0
     - FunctionalState TXenable：发送中断使能 失能
     - FunctionalState RXenable：就收中断使能 使能
    ----------------------------------------------------------*/
    Device_Usart3_ENABLE_Init(115200,2,0,DISABLE,ENABLE);
}

/* ©2015-2016 Beijing Bechade Opto-Electronic, Co.,Ltd. All rights reserved. */
