/*
*********************************************************************************************************
*
*	模块名称 : 定义外部接口文件
*	文件名称 : cnc.h
*	版    本 : V1.1
*	说    明 : 头文件
*
*	修改记录 :
*		版本号  日期       作者      说明
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

#ifndef _CNC_H
#define _CNC_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "cnc_base.h"
#include "cnc_move.h"
#include "cnc_gcode.h"
#include "cnc_conf.h"


/*定时器的函数驱动*/
typedef  struct
{
	void (*pinit)(uint32_t arr);    /*初始化定时器*/
	void (*popen)(void);            /*开定时器*/
	void (*pclose)(void);           /*关定时器*/   
 void  (*pupdata)(uint32_t arr);  /*更新计数寄存器*/	
}cnc_tim_api_typedef;

/*外围的函数驱动*/
typedef  struct
{
 uint8_t (*pkey_block)(void);           /*阻塞式按键函数驱动*/
 uint8_t (*pkey_nblock)(void);          /*非阻塞式按键函数驱动*/
 void    (*pwr_parameter)(void);         /*存取参数，并设定参数上下限值*/
	void    (*pdiszb)(uint8_t state);      /*显示坐标函数驱动*/
 void    (*pdisn)(int16_t n);                       /*显示自动速度百分比函数驱动*/
 void    (*pdiszt)(uint8_t mode,uint8_t state);     /*显示运动模式和状态*/
 void    (*pdisgcode)(char *pf,uint16_t add);       /*自动运行时显示运动中的G代码*/
}cnc_user_typedef;

/*G代码的接口函数*/
typedef struct
{
 g_t (*pf2g) (uint8_t fn,uint8_t *pf,f_t *pfile); /*从数组解析成G代码*/
	void (*pg04) (uint32_t times);        /*G04延时*/
}cnc_gcode_api_typedef;

/*定义工作状态*/
typedef enum 
{
 CNC_SD=0,  /*手动状态*/
 CNC_ZD,    /*自动状态*/
 CNC_ZT,    /*暂停状态*/
	CNC_LX,    /*连续状态*/
}CNC_MODE;

/*定义电机运动状态*/
typedef enum 
{
 CNC_MNRUN=0,  /*未运行状态*/
 CNC_MRUN,     /*运行状态*/
 CNC_MSTOP,    /*暂停状态*/
 CNC_MBREAK,   /*退出状态*/
}CNC_MSTATE;   /*在process函数中进行设置*/

/*定义轴选*/
typedef enum 
{
 CNC_X=0x01,    /*X轴为长轴*/
 CNC_Y=0x02,    
 CNC_Z=0x04,   
 CNC_A=0x08,
 CNC_ALL=0x0F,
}CNC_MCS;

/*通过api返回的内部状态*/
typedef enum 
{
 CNC_OK=0,      /*工作正常*/
	CNC_BREAK,     /*强制退出*/
 CNC_GCODE_FINSH,/*Gcode完成*/
 CNC_ERR_MCS,   /*轴选错误*/
}CNC_STATE;

/*外部调用的函数 和 外部设置的变量*/

void cnc_api_conf(cnc_user_typedef *puser,cnc_tim_api_typedef *ptim,cnc_gcode_api_typedef *puser_gcode);
void cnc_gear_conf(gear_typedef *gear,CNC_MCS motor_cs);
void cnc_stable_conf(speed_table_typedef *speed_table);
void cnc_motor_init(CNC_MCS motor_cs);
void cnc_motor_clean(CNC_MCS motor_cs);
void cnc_parameter_conf(speed_table_typedef *pstable,motor_typedef *px,motor_typedef *py,motor_typedef *pz,motor_typedef *pa);
void cnc_start_conf(speed_table_typedef *pstable,motor_typedef *px,motor_typedef *py,motor_typedef *pz,motor_typedef *pa);
int32_t cnc_dis(CNC_MCS motor_cs);
CNC_STATE cnc_run(uint8_t *pf,uint16_t _f,uint8_t _vpct);
#endif

/***************************** lcj1031@foxmail.com (END OF FILE) *********************************/
