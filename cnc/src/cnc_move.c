/*
*********************************************************************************************************
*
*	模块名称 : CNC控制
*	文件名称 : cnc_move.c
*	版    本 : V1.1
*	说    明 : CNC电机基本运动控制
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
//#include "bsp.h"

motor_run_typedef run;
//motor_run_typedef run2;    /*连续模式下,求出连续速度和降频脉冲*/
speed_table_typedef stable; 
speed_typedef speed; 
motor_typedef x;
motor_typedef y;
motor_typedef z;
motor_typedef a;

cnc_user_typedef user;          /*user驱动接口*/
cnc_tim_api_typedef user_tim;   /*定时器驱动接口*/

extern uint16_t speed_table[CNC_MAX_LADDER];
extern uint16_t step_table[CNC_MAX_LADDER];
extern const uint16_t time_table[9];/*时间值*/

/*
*********************************************************************************************************
*	函 数 名: cnc_mconf
*	功能说明: 把电机运行状态设置为运行
            设置方向IO
            把长度换算成脉冲
            找出长轴
            设置dda累加器的值，初始化各个轴的dda累加器
            规划长轴的爬频表
*	形    参: _x x的长度 _y y的长度 _z z的长度 _a a的长度 _f 工进速度 _vpct 速度的百分比
           _x _y _z _a 都是相对坐标值
*	返 回 值: 返回轴选
*********************************************************************************************************
*/
static void cnc_mconf(int32_t _x,int32_t _y,int32_t _z,int32_t _a,uint16_t _f,int8_t _vpct)
{
	uint32_t max[2];
	if(_x>=0){CNC_XDIRH;x.move.dir=1;}else{CNC_XDIRL;x.move.dir=0;}  /*设置运动脉冲的方向*/
	if(_y>=0){CNC_YDIRH;y.move.dir=1;}else{CNC_YDIRL;y.move.dir=0;}
	if(_z>=0){CNC_ZDIRH;z.move.dir=1;}else{CNC_ZDIRL;z.move.dir=0;}
	if(_a>=0){CNC_ADIRH;a.move.dir=1;}else{CNC_ADIRL;a.move.dir=0;}
	x.move.gs_now=_x;  
	cnc_gs2cp(&x);    /*转换出X的脉冲值*/
	y.move.gs_now=_y;
	cnc_gs2cp(&y);    /*转换出Y的脉冲值*/
	z.move.gs_now=_z;  
	cnc_gs2cp(&z);    /*转换出Z的脉冲值*/
	a.move.gs_now=_a;
	cnc_gs2cp(&a);    /*转换出A的脉冲值*/
	x.move.dda[1]=x.move.cp_now=labs(x.move.cp_now);  /*求出绝对值和初始化dda*/
	y.move.dda[1]=y.move.cp_now=labs(y.move.cp_now);
	z.move.dda[1]=z.move.cp_now=labs(z.move.cp_now);
	a.move.dda[1]=a.move.cp_now=labs(a.move.cp_now);
	max[0] = x.move.cp_now>=y.move.cp_now ? x.move.cp_now : y.move.cp_now;/*求出4个轴中的长轴*/
	max[1] = z.move.cp_now>=a.move.cp_now ? z.move.cp_now : a.move.cp_now;
	run.ddam=run.rcp = max[0]>=max[1]?max[0]:max[1];
	speed.f=_f;
	speed.vpct=_vpct;
	/*把最长轴的速度，作为运行速度*/
 /*X轴*/
	if(run.rcp == x.move.cp_now)
 {
  if(speed.f<x.move.min_speed){speed.f=x.move.min_speed;}     /*限制速度上下限*/
  else
  if(speed.f>x.move.max_speed){speed.f=x.move.max_speed;}
  cnc_f2v(&speed,&x);cnc_utable(&speed,&run);run.mcs=CNC_X;/*换算速度,并规划爬频表格,设置轴选*/
 }
	else
 /*Y轴*/
	if(run.rcp == y.move.cp_now)
 {
  if(speed.f<y.move.min_speed){speed.f=y.move.min_speed;}     /*限制速度上下限*/
  else
  if(speed.f>y.move.max_speed){speed.f=y.move.max_speed;}
  cnc_f2v(&speed,&y);cnc_utable(&speed,&run);run.mcs=CNC_Y;
 }
	else
 /*Z轴*/
	if(run.rcp == z.move.cp_now)
 {
  if(speed.f<z.move.min_speed){speed.f=z.move.min_speed;}     /*限制速度上下限*/
  else
  if(speed.f>z.move.max_speed){speed.f=z.move.max_speed;}
  cnc_f2v(&speed,&z);cnc_utable(&speed,&run);run.mcs=CNC_Z;
 }
	else
 /*A轴*/
	if(run.rcp == a.move.cp_now)
 {
  if(speed.f<a.move.min_speed){speed.f=a.move.min_speed;}     /*限制速度上下限*/
  else
  if(speed.f>a.move.max_speed){speed.f=a.move.max_speed;}
  cnc_f2v(&speed,&a);cnc_utable(&speed,&run);run.mcs=CNC_A;
 }	
}

/*
*********************************************************************************************************
*	函 数 名: cnc_sd_Process
*	功能说明: 手动运行工作函数
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void cnc_sd_process(CNC_MCS motor_cs)
{
	run.mstate=CNC_MRUN;                                 /*设置电机运动状态*/
 user.pdiszt(run.mode,run.mstate);                     /*显示状态和模式*/
	run.sdcp=0;                                          /*设置手动脉冲为0*/
	user_tim.popen();                                    /*开定时器*/
 while(user.pkey_block()!=CNC_KEY_NONE){user.pdiszb(CNC_SD);}/*按住键的情况下，一直升频率*/
 /*松开按键，开始降频,保证一直在降频*/
	while(run.sdcp!=0){run.rcp=run.dcp=9999;user.pdiszb(CNC_SD);} /*把台阶降到0,同时手动脉冲保证除尽,循环显示坐标*/
	user.pdiszb(CNC_SD);                                   /*显示坐标*/
	run.mstate=CNC_MNRUN;                                /*设置电机运动状态*/
 user.pdiszt(run.mode,run.mstate);                     /*显示状态和模式*/
}	
/*
*********************************************************************************************************
*	函 数 名: cnc_sd
*	功能说明: 手动运行主函数
*	形    参: _x x的长度 _y y的长度 _z z的长度 _a a的长度 _f 工进速度  _vpct 速度的百分比 50～0～-50
*	返 回 值: 无
*********************************************************************************************************
*/
void cnc_sd(int32_t _x,int32_t _y,int32_t _z,int32_t _a,uint16_t _f,uint8_t _vpct)
{
 run.mode=CNC_SD;
 /*在换算gs/cp的时候防止cp_buff多累积*/
 _x=_x/x.gear.gs*x.gear.gs;
 _y=_y/y.gear.gs*y.gear.gs;
 _z=_z/z.gear.gs*z.gear.gs;
 _a=_a/a.gear.gs*a.gear.gs;
	cnc_mconf(_x,_y,_z,_a,_f,_vpct);
	cnc_sd_process(run.mcs);
}

/*
*********************************************************************************************************
*	函 数 名: cnc_vchange
*	功能说明: 改变速度的百分比
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
 static void cnc_vchange(void)
	{
		if(user.pkey_nblock()==CNC_KEY_VU)
  {
   speed.vpct+=10;
   if(speed.vpct>=50){speed.vpct=50;}
			cnc_utable(&speed,&run); /*数据在cnc_mconf中完成初始化，这里vpct发生改变重新规划下表格*/
		}
		if(user.pkey_nblock()==CNC_KEY_VD)
  {
   speed.vpct-=10;
   if(speed.vpct<=-50){speed.vpct=-50;}
			cnc_utable(&speed,&run); /*数据在cnc_mconf中完成初始化，这里vpct发生改变重新规划下表格*/
		}
  user.pdisn(speed.vpct);
  //while(user.pkey()!=CNC_KEY_NONE){;}
	}	
	
/*
*********************************************************************************************************
*	函 数 名: cnc_mstop
*	功能说明: 运动暂停函数
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void cnc_mstop(void)
{
 uint32_t rcp;
	if(user.pkey_block()==CNC_KEY_ZT)
	{
		run.mstate=CNC_MSTOP;                                /*进入暂停状态*/           
  user.pdiszt(run.mode,run.mstate);                             
  run.dcp=run.rcp;                                     /*开始降频*/
 	while(run.ladder!=0){user.pdiszb(CNC_SD);}           /*把台阶降到0,循环显示坐标*/
	 user.pdiszb(CNC_SD);                                 /*显示坐标*/
		while(1)
		{
			if(user.pkey_block()==CNC_KEY_TC){run.mstate=CNC_MBREAK;break;}/*进入退出状态*/
		 if(user.pkey_block()==CNC_KEY_QD)                              /*进入启动状态*/
   {
				run.mstate=CNC_MRUN;  
				cnc_utable(&speed,&run); /*数据在cnc_mconf中完成初始化，这里rcp和dcp发生改变重新规划下表格*/
    user_tim.popen();/*开定时器*/
				break;
			}
		}
	}
 user.pdiszt(run.mode,run.mstate);   
}	

/*
*********************************************************************************************************
*	函 数 名: cnc_zd_Process
*	功能说明: 自动运行工作函数
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void cnc_zd_process(void)
{
	run.mstate=CNC_MRUN;                   /*设置电机运动状态*/
 user.pdiszt(run.mode,run.mstate);        /*显示状态和模式*/
	user_tim.popen();/*开定时器*/
 while((run.rcp!=0))
 {
		user.pdiszb(CNC_ZD);  /*实时显示坐标*/
		cnc_mstop();        /*暂停状态子程序*/
		cnc_vchange();      /*根据按键改变速度百分比*/  
  if(run.mstate==CNC_MBREAK){break;} /*按了退出键，退出自动运行*/ 
	}
	if(run.mstate==CNC_MRUN){run.mstate=CNC_MNRUN;}/*如果是运行状态，则变化为停止运行状态*/
	else                    {run.mstate=CNC_MBREAK;}/*如果不是运行状态，就是退出状态*/
 user.pdiszt(run.mode,run.mstate);               /*显示状态和模式*/
}	

/*
*********************************************************************************************************
*	函 数 名: cnc_xd
*	功能说明: 运行相对坐标值
*	形    参: _x x的长度 _y y的长度 _z z的长度 _a a的长度 _f 工进速度  _vpct 速度的百分比 50～0～-50
*	返 回 值: 速度百分比
*********************************************************************************************************
*/
CNC_STATE cnc_xd(int32_t _x,int32_t _y,int32_t _z,int32_t _a,uint16_t _f,uint8_t _vpct)
{
 run.mode=CNC_ZD;
 cnc_mconf(_x,_y,_z,_a,_f,_vpct);/*初始化电机参数*/
	if(run.rcp!=0){cnc_zd_process();}
	user.pdiszb(CNC_ZD);     
 if(run.mstate==CNC_MNRUN){return CNC_OK;}
 if(run.mstate==CNC_MBREAK){return CNC_BREAK;}     
	//return speed.vpct;
}

/*
*********************************************************************************************************
*	函 数 名: cnc_jd
*	功能说明: 运行绝对坐标值 可以选择哪几个轴走动
*	形    参: _x x的长度 _y y的长度 _z z的长度 _a a的长度 _f 工进速度  _vpct 速度的百分比 50～0～-50 CNC_MCS 轴选
*	返 回 值: 速度百分比
*********************************************************************************************************
*/
CNC_STATE cnc_jd(int32_t _x,int32_t _y,int32_t _z,int32_t _a,uint16_t _f,uint8_t _vpct,CNC_MCS motor_cs)
{
 run.mode=CNC_ZD;
 if((motor_cs&CNC_ALL)==CNC_ALL)
 {
		_x-=x.move.gs_dis;
		_y-=y.move.gs_dis;
		_z-=z.move.gs_dis;
		_a-=a.move.gs_dis;
 }
 else
 {
		if((motor_cs&CNC_X)==CNC_X){_x-=x.move.gs_dis;if((motor_cs&CNC_Y)!=CNC_Y){_y=0;}if((motor_cs&CNC_Z)!=CNC_Z){_z=0;}if((motor_cs&CNC_A)!=CNC_A){_a=0;}}
		if((motor_cs&CNC_Y)==CNC_Y){_y-=y.move.gs_dis;if((motor_cs&CNC_X)!=CNC_X){_x=0;}if((motor_cs&CNC_Z)!=CNC_Z){_z=0;}if((motor_cs&CNC_A)!=CNC_A){_a=0;}}
		if((motor_cs&CNC_Z)==CNC_Z){_z-=z.move.gs_dis;if((motor_cs&CNC_X)!=CNC_X){_y=0;}if((motor_cs&CNC_Y)!=CNC_Y){_y=0;}if((motor_cs&CNC_A)!=CNC_A){_a=0;}}
		if((motor_cs&CNC_A)==CNC_A){_a-=a.move.gs_dis;if((motor_cs&CNC_X)!=CNC_X){_y=0;}if((motor_cs&CNC_Y)!=CNC_Y){_y=0;}if((motor_cs&CNC_Z)!=CNC_Z){_z=0;}}
 }
 cnc_mconf(_x,_y,_z,_a,_f,_vpct);/*初始化电机参数*/
	if(run.rcp!=0){cnc_zd_process();}
	user.pdiszb(CNC_ZD); 
 if(run.mstate==CNC_MNRUN){return CNC_OK;}
 if(run.mstate==CNC_MBREAK){return CNC_BREAK;}
	//return speed.vpct;
}

/*
*********************************************************************************************************
*	函 数 名: cnc_motor_clean(CNC_MCS motor_cs)
*	功能说明: 清零
*	形    参: motor_cs 轴选
*	返 回 值: 无
*********************************************************************************************************
*/
void cnc_motor_clean(CNC_MCS motor_cs)
{
 cnc_motor_init(motor_cs);
 user.pdiszb(CNC_SD); 
}

/*
*********************************************************************************************************
*	函 数 名: cnc_tim_ccr
*	功能说明: 定时器比较中断
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void cnc_tim_ccr(void)
{
	x.move.dda[0]=x.move.cp_now+x.move.dda[1];
	y.move.dda[0]=y.move.cp_now+y.move.dda[1];
	z.move.dda[0]=z.move.cp_now+z.move.dda[1];
	a.move.dda[0]=a.move.cp_now+a.move.dda[1];
	if(run.rcp!=0)
	{
		switch(run.mcs)
		{
			case(CNC_X):CNC_XCPL;
			            if(x.move.dir){x.move.cp_dis++;}else{x.move.cp_dis--;}
															if(y.move.dda[0]>=run.ddam){y.move.dda[1]=y.move.dda[0]-run.ddam;if(y.move.dir){y.move.cp_dis++;}else{y.move.cp_dis--;}CNC_YCPL;}
															else                       {y.move.dda[1]=y.move.dda[0];}	
															if(z.move.dda[0]>=run.ddam){z.move.dda[1]=z.move.dda[0]-run.ddam;if(z.move.dir){z.move.cp_dis++;}else{z.move.cp_dis--;}CNC_ZCPL;}
															else                       {z.move.dda[1]=z.move.dda[0];}	
															if(a.move.dda[0]>=run.ddam){a.move.dda[1]=a.move.dda[0]-run.ddam;if(a.move.dir){a.move.cp_dis++;}else{a.move.cp_dis--;}CNC_ACPL;}
															else                       {a.move.dda[1]=a.move.dda[0];}	
				           break;
			case(CNC_Y):CNC_YCPL;
			            if(y.move.dir){y.move.cp_dis++;}else{y.move.cp_dis--;}
															if(x.move.dda[0]>=run.ddam){x.move.dda[1]=x.move.dda[0]-run.ddam;if(x.move.dir){x.move.cp_dis++;}else{x.move.cp_dis--;}CNC_XCPL;}
															else                       {x.move.dda[1]=x.move.dda[0];}	
															if(z.move.dda[0]>=run.ddam){z.move.dda[1]=z.move.dda[0]-run.ddam;if(z.move.dir){z.move.cp_dis++;}else{z.move.cp_dis--;}CNC_ZCPL;}
															else                       {z.move.dda[1]=z.move.dda[0];}	
															if(a.move.dda[0]>=run.ddam){a.move.dda[1]=a.move.dda[0]-run.ddam;if(a.move.dir){a.move.cp_dis++;}else{a.move.cp_dis--;}CNC_ACPL;}
															else                       {a.move.dda[1]=a.move.dda[0];}	
				           break;
			case(CNC_Z):CNC_ZCPL;
			            if(z.move.dir){z.move.cp_dis++;}else{z.move.cp_dis--;}
															if(x.move.dda[0]>=run.ddam){x.move.dda[1]=x.move.dda[0]-run.ddam;if(x.move.dir){x.move.cp_dis++;}else{x.move.cp_dis--;}CNC_XCPL;}
															else                       {x.move.dda[1]=x.move.dda[0];}	
															if(y.move.dda[0]>=run.ddam){y.move.dda[1]=y.move.dda[0]-run.ddam;if(y.move.dir){y.move.cp_dis++;}else{y.move.cp_dis--;}CNC_YCPL;}
															else                       {y.move.dda[1]=y.move.dda[0];}	
															if(a.move.dda[0]>=run.ddam){a.move.dda[1]=a.move.dda[0]-run.ddam;if(a.move.dir){a.move.cp_dis++;}else{a.move.cp_dis--;}CNC_ACPL;}
															else                       {a.move.dda[1]=a.move.dda[0];}	
				           break;
			case(CNC_A):CNC_ACPL;
			            if(a.move.dir){a.move.cp_dis++;}else{a.move.cp_dis--;}
															if(x.move.dda[0]>=run.ddam){x.move.dda[1]=x.move.dda[0]-run.ddam;if(x.move.dir){x.move.cp_dis++;}else{x.move.cp_dis--;}CNC_XCPL;}
															else                       {x.move.dda[1]=x.move.dda[0];}	
															if(y.move.dda[0]>=run.ddam){y.move.dda[1]=y.move.dda[0]-run.ddam;if(y.move.dir){y.move.cp_dis++;}else{y.move.cp_dis--;}CNC_YCPL;}
															else                       {y.move.dda[1]=y.move.dda[0];}	
															if(z.move.dda[0]>=run.ddam){z.move.dda[1]=z.move.dda[0]-run.ddam;if(z.move.dir){z.move.cp_dis++;}else{z.move.cp_dis--;}CNC_ZCPL;}
															else                       {z.move.dda[1]=z.move.dda[0];}	
               break;
			default:break;
		}
	}
}	

/*
*********************************************************************************************************
*	函 数 名: cnc_tim_updata
*	功能说明: 定时器溢出中断
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void cnc_tim_updata(void)
{
 int32_t t,tt;
	CNC_XCPH;CNC_YCPH;CNC_ZCPH;CNC_ACPH;
	run.rcp--;
 if(run.rcp>run.dcp)
	{
		if(run.ladder!=run.max_ladder)
		{
			run.step--;
			if(run.step==0)
			{
				if(run.ladder<run.max_ladder){run.step=step_table[run.ladder];run.ladder++;}
				if(run.ladder>run.max_ladder){run.step=step_table[run.ladder];run.ladder--;}
			}
		}
		else{run.step=1;}
	}
	else
	{
		if(run.ladder!=0)
		{
			if(run.step==0){run.step=1;}
			run.step--;
			if(run.step==0){run.ladder--;run.step=step_table[run.ladder];}
		}
		else{run.step=1;}
	}
	switch(run.mode)
	{
		case(CNC_ZD):if(run.rcp==0){
                user_tim.pclose();}
               if(run.mstate==CNC_MSTOP){if(run.ladder==0){user_tim.pclose();}}
			            break;
// 	case(CNC_SD):
//               if(run.ladder==0)
//               {
//																switch(run.mcs)
//																{ /*为了手动的脉冲转换长度为整数*/
//																	case(CNC_X):user_tim.pclose();break;
//																	case(CNC_Y):user_tim.pclose();break;
//																	default:break;
//																}
//															}
//		             break;
		case(CNC_SD):run.sdcp++;
               if((run.ladder==0)&(run.sdcp>1))
               {
                if((run.mcs&CNC_X)==CNC_X){if(((run.sdcp/x.gear.cp)*x.gear.cp)==run.sdcp){user_tim.pclose();run.sdcp=0;}}
																if((run.mcs&CNC_Y)==CNC_Y){if(((run.sdcp/y.gear.cp)*y.gear.cp)==run.sdcp){user_tim.pclose();run.sdcp=0;}}
																if((run.mcs&CNC_Z)==CNC_Z){if(((run.sdcp/z.gear.cp)*z.gear.cp)==run.sdcp){user_tim.pclose();run.sdcp=0;}}
																if((run.mcs&CNC_A)==CNC_A){if(((run.sdcp/a.gear.cp)*a.gear.cp)==run.sdcp){user_tim.pclose();run.sdcp=0;}}
															}
		             break;
		default:break;
	}
	user_tim.pupdata(speed_table[run.ladder]);/*更改定时器寄存器*/
}	
/***************************** lcj1031@foxmail.com (END OF FILE) *********************************/
