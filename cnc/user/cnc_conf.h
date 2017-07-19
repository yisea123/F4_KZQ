/*
*********************************************************************************************************
*
*	模块名称 : 定义初始化常量
*	文件名称 : cncconf.h
*	版    本 : V1.1
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

#ifndef _CNCCONF_H
#define _CNCCONF_H

#include "bsp.h"

#define CNC_MAX_V 110000  /*110.000 KHZ      */
#define CNC_MIN_V 1       /*000.001 KHZ =1HZ */

#define CNC_MAX_XV 3100  /*X轴最高速*/
#define CNC_MIN_XV 1100  /*X轴最低速*/

#define CNC_MAX_YV 3200  /*Y轴最高速*/
#define CNC_MIN_YV 1200  /*Y轴最低速*/

#define CNC_MAX_ZV 3300  /*Z轴最高速*/
#define CNC_MIN_ZV 1300  /*Z轴最低速*/

#define CNC_MAX_AV 3400  /*A轴最高速*/
#define CNC_MIN_AV 1400  /*A轴最低速*/

#define CNC_MAX_LADDER  1024 /*表格最大值*/

#define CNC_TIM_CLK  84                                       /*定时器时钟，单位Mhz*/
#define CNC_TIM_PSC  83                                       /*定时器预分频*/  
#define CNC_TIM_CNT  1000000*CNC_TIM_CLK/(CNC_TIM_PSC+1)      /*定时器的值*/  

/*定义轴对应的管脚*/
#define CNC_XENH    
#define CNC_XENL
#define CNC_XDIRH    XDIR_GPIO_Port->BSRR = XDIR_Pin
#define CNC_XDIRL    XDIR_GPIO_Port->BSRR = (XDIR_Pin<<16)
#define CNC_XCPH     XCP_GPIO_Port->BSRR  = XCP_Pin
#define CNC_XCPL     XCP_GPIO_Port->BSRR  = (XCP_Pin<<16)

#define CNC_YENH
#define CNC_YENL
#define CNC_YDIRH   YDIR_GPIO_Port->BSRR = YDIR_Pin
#define CNC_YDIRL   YDIR_GPIO_Port->BSRR = (YDIR_Pin<<16)
#define CNC_YCPH    YCP_GPIO_Port->BSRR =  YCP_Pin
#define CNC_YCPL    YCP_GPIO_Port->BSRR =  (YCP_Pin<<16)

#define CNC_ZENH
#define CNC_ZENL
#define CNC_ZDIRH
#define CNC_ZDIRL
#define CNC_ZCPH
#define CNC_ZCPL

#define CNC_AENH
#define CNC_AENL
#define CNC_ADIRH
#define CNC_ADIRL
#define CNC_ACPH
#define CNC_ACPL

/*定义M功能*/
#define CNC_M03
#define CNC_M08
#define CNC_M09

#endif

/***************************** lcj1031@foxmail.com (END OF FILE) *********************************/
