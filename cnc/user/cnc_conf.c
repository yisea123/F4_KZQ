/*
*********************************************************************************************************
*
*	模块名称 : CNC初始化配置
*	文件名称 : cnc_conf.c
*	版    本 : V1.1
*	说    明 : CNC初始化
*
*	修改记录 :
*		版本号  日期        作者     说明
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
#include "cnc.h"

/*
*********************************************************************************************************
*需要初始化的参数 
*	ucnc_stable  V0        a        t
*	             初速度    加速度   加速周期
*	ucnc_x       gear.gs   gear.cp  n
*              um        脉冲     速度倍数 
*
*********************************************************************************************************
*/
extern cnc_user_typedef user_driver;         /*用户代码驱动接口*/
extern cnc_tim_api_typedef stm32_tim2;       /*定时器代码驱动接口*/
extern cnc_gcode_api_typedef gcode_driver;   /*G代码驱动接口*/

/*
*********************************************************************************************************
*	函 数 名: cnc_parameter_conf
*	功能说明: cnc参数初始化
*	形    参: speed_table_typedef v0 a t  motor_typedef gs cp n
*	返 回 值: 无
*********************************************************************************************************
*/
void cnc_parameter_conf(speed_table_typedef *pstable,motor_typedef *px,motor_typedef *py,motor_typedef *pz,motor_typedef *pa)
{
 cnc_gear_conf(&(px->gear),CNC_X);                        /*初始化X电子齿轮*/
 cnc_gear_conf(&(py->gear),CNC_Y);                        /*初始化Y电子齿轮*/
 cnc_gear_conf(&(pz->gear),CNC_Z);                        /*初始化Z电子齿轮*/
 cnc_gear_conf(&(pa->gear),CNC_A);                        /*初始化A电子齿轮*/
 cnc_stable_conf(pstable);                                /*初始化爬频表格*/
 cnc_motor_init(CNC_ALL);                                 /*清参数*/
}

/*
*********************************************************************************************************
*	函 数 名: cnc_start_conf
*	功能说明: cnc启动初始化
*	形    参: speed_table_typedef v0 a t  motor_typedef gs cp n
*	返 回 值: 无
*********************************************************************************************************
*/
void cnc_start_conf(speed_table_typedef *pstable,motor_typedef *px,motor_typedef *py,motor_typedef *pz,motor_typedef *pa)
{
 cnc_parameter_conf(pstable,px,py,pz,pa);                  /*初始化参数*/
 cnc_api_conf(&user_driver,&stm32_tim2,&gcode_driver);     /*初始化API*/
}
/***************************** lcj1031@foxmail.com (END OF FILE) *********************************/
