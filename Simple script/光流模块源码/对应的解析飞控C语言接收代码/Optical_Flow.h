/* ©2015-2016 Beijing Bechade Opto-Electronic, Co.,Ltd. All rights reserved.
 + 文件名  ：Optical_Flow.h
 + 描述    ：数据传输头文件
代码尽量做到逐行注释 代码有问题，及时加群交流 作者有偿支持对开源代码的完善 */
#ifndef _OPTICAL_FLOW_H
#define	_OPTICAL_FLOW_H

#include "stm32f4xx.h"

/* 读取X偏移量原始数据  */
extern int Optical_flow_org_x;
/* 读取Y偏移量原始数据 */
extern int Optical_flow_org_y;
/* 读取Confidence原始数据 */
extern int Optical_flow_confidence;

/* 换算到X像素速度  光流像素要结合高度才能算出速度 值范围约+-200.000 */
extern float Optical_flow_x;
/* 换算到Y像素速度  光流像素要结合高度才能算出速度 值范围约+-200.000 */
extern float Optical_flow_y;
/* 读取画面不变率 值范围0-100 */
extern float Optical_flow_con;

/*----------------------------------------------------------
 + 实现功能：数据接收并保存
 + 调用参数：接收到的单字节数据
----------------------------------------------------------*/
extern void Optical_Flow_Receive_Prepare(u8 data);

/*----------------------------------------------------------
 + 实现功能：光流初始化
----------------------------------------------------------*/
extern void Optical_Flow_init();

#endif
/* ©2015-2016 Beijing Bechade Opto-Electronic, Co.,Ltd. All rights reserved. */
