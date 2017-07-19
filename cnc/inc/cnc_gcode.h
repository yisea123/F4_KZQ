/*
*********************************************************************************************************
*
*	模块名称 : cncgcode.c头文件
*	文件名称 : cncgcode.h
*	版    本 : V1.0
*	说    明 : 头文件
*
*	修改记录 :
*		版本号  日期       作者    说明
*		V1.0    2016-12-15  lcj    开写
*  V1.1    2017-06-30  lcj      添加每个轴的速度限制，在cnc_conf.h中增加高低速宏定义  CNC_MAX_XV ...
*                                                     在move_typedef中增加min_speed 和max_speed
*                               更改cnc_motor_conf为cnc_motor_init
*                               更改cnc_conf为cnc_statr_conf 和 cnc_parameter_conf
*
*	Copyright (C), 2016-2017, lcj1031@foxmail.com
*
*********************************************************************************************************
*/

#ifndef _CNCGCODE_H
#define _CNCGCODE_H

typedef struct
{
 char     gcode;  /*G/M*/
 uint16_t gdata;  /*命令值*/
 int32_t  x;      /*x坐标值*/
 int32_t  y;      /*y坐标值*/ 
 int32_t  z;      /*z坐标值*/
 int32_t  a;      /*a坐标值*/
// char     fcode;  /**/
 int16_t  fdata; /**/
// char     lcode; /**/
 int16_t  ldata; /**/
 int16_t  number; /*当前段号*/
} g_t;


typedef struct
{
 int16_t  add;     /*数组首地址*/
 int16_t  add_xh;  /*循环首地址*/
 int16_t  xhn;     /*循环次数*/
} f_t;

#endif

/***************************** lcj1031@foxmail.com (END OF FILE) *********************************/
