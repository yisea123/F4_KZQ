/*
*********************************************************************************************************
*
*	模块名称 : cnc_move.c头文件
*	文件名称 : cncmove.h
*	版    本 : V1.0
*	说    明 : 头文件
*
*	修改记录 :
*		版本号  日期       作者    说明
*		V1.0    2016-12-15  lcj      开写
*  V1.1    2017-06-30  lcj      添加每个轴的速度限制，在cnc_conf.h中增加高低速宏定义  CNC_MAX_XV ...
*                                                     在move_typedef中增加min_speed 和max_speed
*                               更改cnc_motor_conf为cnc_motor_init
*                               更改cnc_conf为cnc_statr_conf 和 cnc_parameter_conf
*
*	Copyright (C), 2016-2017, lcj1031@foxmail.com
*
*********************************************************************************************************
*/

#ifndef _CNCMOVE_H
#define _CNCMOVE_H

void cnc_sd(int32_t _x,int32_t _y,int32_t _z,int32_t _a,uint16_t _f,uint8_t _vpct);
uint8_t cnc_xd(int32_t _x,int32_t _y,int32_t _z,int32_t _a,uint16_t _f,uint8_t _vpct);
uint8_t  cnc_jd(int32_t _x,int32_t _y,int32_t _z,int32_t _a,uint16_t _f,uint8_t _vpct,uint8_t motor_cs);
void cnc_tim_ccr(void);
void cnc_tim_updata(void);
#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
