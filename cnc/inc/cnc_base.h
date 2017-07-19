/*
*********************************************************************************************************
*
*	模块名称 : cnc_base.c头文件
*	文件名称 : cnc_base.h
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

#ifndef _CNCBASE_H
#define _CNCBASE_H

#define     __I     volatile const            /*!< defines 'read only' permissions      */
#define     __O     volatile                  /*!< defines 'write only' permissions     */
#define     __IO    volatile                  /*!< defines 'read / write' permissions   */

/*	速度结构体*/
typedef struct
{
	uint32_t f;          /*单位  mm/min*/
	uint32_t v;          /*单位  hz*/
	int16_t  vladder;    /*爬频表格上的速度阶梯*/
	int16_t  vpct;       /*速度百分比*/
}speed_typedef;

/*爬频表格结构体*/
typedef struct
{
	uint32_t vt;     					/*最终速度*/
	uint32_t v0;     					/*初速度*/
	uint32_t a;      					/*加速度*/
	uint8_t t;      				 	/*加速周期*/
	uint32_t max_ladder;  /*到达CNC_MAX_V速度时候阶梯*/
}speed_table_typedef;

/*电机运行结构体*/
typedef struct
{
	__IO uint32_t rcp;        /*所有脉冲值*/  
 __IO uint32_t dcp;        /*降频最小脉冲值*/
 __IO uint16_t max_ladder; /*爬频到最高的阶梯*/
	__IO uint32_t sdcp;       /*手动运行的脉冲*/  
	uint16_t ladder;          /*中断中爬频阶梯*/
	uint16_t step;            /*中断中每个台阶的步数*/
	uint32_t ddam;            /*积分累加器*/
 uint8_t  mcs;             /*长轴的轴选*/
	uint8_t  mstate;          /*当前轴的状态*/
 uint8_t  mode;            /*当前轴的工作模式 自动/手动*/
}motor_run_typedef;

/*	脉冲比例结构体*/
typedef  struct
{
	uint32_t gs;		  /*单位  cmm*/
 uint32_t cp;		  /*单位  个*/
	uint32_t n;     /*外部有齿轮的倍率*/
}gear_typedef;

/*运动结构体*/
typedef struct
{
 int32_t max_speed;       /*设定轴的最高速*/
 int32_t min_speed;       /*设定轴的最低速*/
	int32_t gs_now;    					 /*现在需要运行的长度，单位cmm*/
	int32_t cp_now;     					/*现在需要运行的脉冲，通过电子齿轮换算出来*/
	int32_t cp_buff;  					  /*小于1cp的走动量，保持住，用于下次累计*/
	int32_t gs_dis;          /*显示使用的总长度*/
	int32_t cp_dis;          /*显示使用的总脉冲*/
	uint32_t dda[2];         /*积分器的值*/
	uint8_t  dir;            /*当前的运动方向*/
}move_typedef;

typedef  struct
{
 gear_typedef gear;
	//speed_typeddf speed;
	move_typedef  move;
}motor_typedef;


/*定义按键代码*/
typedef enum 
{
	CNC_KEY_NONE=0,
	CNC_KEY_XP,    /*正*/
	CNC_KEY_XN,    /*负*/
	CNC_KEY_YP,    /*正*/
	CNC_KEY_YN,    /*负*/
 CNC_KEY_ZP,    /*正*/
	CNC_KEY_ZN,    /*负*/
 CNC_KEY_AP,    /*正*/
	CNC_KEY_AN,    /*负*/
	CNC_KEY_ZT,    /*暂停*/
	CNC_KEY_QD,    /*启动*/
	CNC_KEY_TC,    /*退出*/
	CNC_KEY_VU,    /*速度百分比加*/
	CNC_KEY_VD,    /*速度百分比减*/
}CNC_KEY;

void cnc_utable(speed_typedef *pspeed,motor_run_typedef *prun);
//motor_run_typedef cnc_utable(speed_typedef *axial);
void cnc_f2v(speed_typedef *pspeed,motor_typedef *paxial);
void cnc_gs2cp(motor_typedef *axial);
#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
