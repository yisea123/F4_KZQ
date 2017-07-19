/*
*********************************************************************************************************
*
*	模块名称 : CNC基本规划
*	文件名称 : cnc_base.c
*	版    本 : V1.1
*	说    明 : CNC运动规划
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

uint16_t speed_table[CNC_MAX_LADDER];
uint16_t step_table[CNC_MAX_LADDER];
const uint16_t time_table[9]={1000,2000,3000,4000,5000,6000,7000,8000,9000};/*时间值*/

extern motor_run_typedef run;
extern speed_table_typedef stable; 
extern speed_typedef speed; 
extern motor_typedef x;
extern motor_typedef y;
extern motor_typedef z;
extern motor_typedef a;
extern move_typedef move_init;

extern cnc_user_typedef user;          /*user驱动接口*/
extern cnc_tim_api_typedef user_tim;  /*定时器驱动接口*/
extern cnc_gcode_api_typedef user_gcode;/*G代码驱动接口*/

/*
*********************************************************************************************************
*	函 数 名: cnc_STable
*	功能说明: 制作爬频表格
*	形    参: *set 根据V0,a,vt规划表格 速度单位hz  1000hz=1Khz
*	返 回 值: 无
*********************************************************************************************************
*/
void cnc_stable(speed_table_typedef *pset)
{
	uint16_t   i=0;     
 uint32_t   step_cp=0;  /*每个台阶需要脉冲*/                  
 uint32_t   temp;
	
	/*定时器时钟周期设置为1us*/  
	uint32_t   t_vt; /*转化为定时器的时间常数*/           
	uint32_t   t_v0;
	uint32_t   t_a;
	uint32_t   t_t;
	
	pset->vt=CNC_MAX_V;
	if(pset->v0<=16){pset->v0=16;}    /*stm32定时器为16位，小于16hz的初速度，定时器将会越界*/    
	
	t_vt=(CNC_TIM_CNT/pset->vt); /*转化为定时器的时间常数*/           
	t_v0=(CNC_TIM_CNT/pset->v0);
	t_a=(CNC_TIM_CNT/pset->a);
	t_t=time_table[pset->t-1];
 
 while(t_v0>=t_vt)
 {
  if(t_v0>=t_t)                     /*v0的脉冲周期大于t的周期*/    
  {
   speed_table[i]=t_v0;
	  step_table[i]=1;
   i++;
	  if(i>=CNC_MAX_LADDER){break;}   /*如果超出表格最大值，则直接跳出*/
			t_v0=((t_v0*t_a)/(t_v0+t_a));   /*运行Vt=V0+AT公式 换算出来*/
  }
  else
  {
	  temp=t_v0;
	  step_cp=0;
	  while(temp<t_t){step_cp++;temp+=t_v0;}
   speed_table[i]=t_v0;
	  step_table[i]=step_cp;
   i++;
	  if(i>=CNC_MAX_LADDER){break;}      /*如果超出表格最大值，则直接跳出*/
	  t_v0=((t_v0*t_a)/(t_v0+t_a));      /*运行Vt=V0+AT公式 换算出来*/
  } 
	}
	pset->max_ladder=(i-1);
}


/*
*********************************************************************************************************
*	函 数 名: cnc_F2V
*	功能说明: 把工进速度 转变为 电机的脉冲频率和爬频表格上ladder
            1mm=0.1dmm=0.01cmm=0.001um
            f单位   mm/min
												gs单位  umm
												cp单位  个		
												v单位    hz
												v=(f*cp/gs*60)
            vladder是speed_table数组中速度台阶
*	形    参: *pspeed *paxial 
*	返 回 值: 无
*********************************************************************************************************
*/
void cnc_f2v(speed_typedef *pspeed,motor_typedef *paxial)
{
	uint32_t temp;
	uint16_t i;
	pspeed->f*=1000;
	pspeed->v=((pspeed->f)*(paxial->gear.cp))/(paxial->gear.gs*60);
 pspeed->v*=paxial->gear.n;  /*有外部搭齿轮*/
	if(pspeed->v>=CNC_MAX_V){pspeed->v=CNC_MAX_V;}
	else
	if(pspeed->v<=CNC_MIN_V){pspeed->v=CNC_MIN_V;}
	temp=(CNC_TIM_CNT/pspeed->v);
	for(i=0;i<CNC_MAX_LADDER;i++)
	{
		if(speed_table[i]<temp){break;}
	}
 if(i>0){i--;}
	pspeed->vladder=i;      
 pspeed->v=(1000000/speed_table[i]);/*此处可以求出实际脉冲频率 1000000/speed[i] = hz*/
}

/*
*********************************************************************************************************
*	函 数 名: cnc_UTable
*	功能说明: 运动前开始规划爬频表格
            根据run_cp和vladder,换算max_ladder和down_cp
            vpct  150%  50
                  130%  30 
                  100%  0
                  10%  -10 
                  50%  -50 
            vladder=vladder+（vladder*vpct/100）
*	形    参: *pspeed
*	返 回 值: motor_run_typedef
*********************************************************************************************************
*/
void cnc_utable(speed_typedef *pspeed,motor_run_typedef *prun)
{
	uint16_t i;
	uint32_t cp=0;
 pspeed->vladder+=((pspeed->vladder*pspeed->vpct)/100);/*换算百分比*/
 for(i=0;i<pspeed->vladder;i++)                     
	{
		cp+=step_table[i];
		if(((cp<<1)>prun->rcp)||(i>stable.max_ladder))
  {
			cp-=step_table[i];
			i--;
		 break;
		}                      /*求出最小降频脉冲*/
	}
 if(cp==0){cp=9999;}     /*保证降频脉冲不是0，防止手动低速bug*/
	prun->dcp=cp;
 if(i==0){i++;}         /*保证最低速度不是0，防止手动低速bug*/
	prun->max_ladder=i; 
	prun->step=step_table[0];
	user_tim.pinit(speed_table[0]);
}

/*
*********************************************************************************************************
*	函 数 名: cnc_gs2cp
*	功能说明: 根据电子齿轮，把mm换算成脉冲数
*	形    参: *axial
*	返 回 值: 无
*********************************************************************************************************
*/
void cnc_gs2cp(motor_typedef *paxial)
{
 int32_t temp;
 temp=paxial->move.cp_buff;	
	//axial->gear->cprcmm=(axial->gear->cp%axial->gear->gs);
	paxial->move.cp_now= ((int32_t)paxial->gear.cp*paxial->move.gs_now)/((int32_t)paxial->gear.gs);  /*长度换算成多少脉冲*/
	paxial->move.cp_buff=((int32_t)paxial->gear.cp*paxial->move.gs_now)%((int32_t)paxial->gear.gs); /*保留除不尽的余数*/
	paxial->move.cp_buff+=temp;
	if(paxial->move.cp_buff>=0)                                                   /*buff为正的*/
	{
	if(paxial->move.cp_buff>=paxial->gear.gs){paxial->move.cp_now++;paxial->move.cp_buff-=paxial->gear.gs;}/*到达gs值就溢出1个cp*/
 }
	else
	{
		if(paxial->move.cp_buff<=(0-paxial->gear.gs)){paxial->move.cp_now--;paxial->move.cp_buff+=paxial->gear.gs;}/*到达gs值就溢出1个cp*/
	}
}

/*
*********************************************************************************************************
*	函 数 名: cnc_api_conf
*	功能说明: 把外部API和内部函数对接上
*	形    参: *puser
*	返 回 值: 无
*********************************************************************************************************
*/
void cnc_api_conf(cnc_user_typedef *puser,cnc_tim_api_typedef *ptim,cnc_gcode_api_typedef *puser_gcode)
{
	user=*puser;
	user_tim=*ptim;
 user_gcode=*puser_gcode;
	user_tim.pinit(speed_table[0]);  /*初始化定时器*/
}

/*
*********************************************************************************************************
*	函 数 名: cnc_gear_conf
*	功能说明: 电子齿轮的参数和内部结构体对接上
*	形    参: cp 脉冲  gs cmm  n外加倍率  轴选
*	返 回 值: 无
*********************************************************************************************************
*/
void cnc_gear_conf(gear_typedef *gear,CNC_MCS motor_cs)
{
 if((motor_cs&CNC_ALL)==CNC_ALL)
 {
//  x.gear=gear;
//  y.gear=gear;
//  z.gear=gear;
//  a.gear=gear;
 }
 else
 {
		if((motor_cs&CNC_X)==CNC_X){memcpy(&(x.gear),gear,sizeof(x.gear));}
		if((motor_cs&CNC_Y)==CNC_Y){memcpy(&(y.gear),gear,sizeof(y.gear));}
		if((motor_cs&CNC_Z)==CNC_Z){memcpy(&(z.gear),gear,sizeof(z.gear));}
		if((motor_cs&CNC_A)==CNC_A){memcpy(&(a.gear),gear,sizeof(a.gear));}
 }
}

/*
*********************************************************************************************************
*	函 数 名: cnc_stable_conf
*	功能说明: 把爬频参数和内部结构体对接上,并规划爬频表格
*	形    参: v0 初速度 a 加速度 t 加速时间
*	返 回 值: 无
*********************************************************************************************************
*/
void cnc_stable_conf(speed_table_typedef *speed_table)
{
 stable.v0=speed_table->v0;
	stable.a=speed_table->a;
	stable.t=speed_table->t;
	cnc_stable(&stable);
}

/*
*********************************************************************************************************
*	函 数 名: cnc_motor_init
*	功能说明: 把选定的轴参数进行清零
*	形    参: 轴选
*	返 回 值: 无
*********************************************************************************************************
*/
void cnc_motor_init(CNC_MCS motor_cs)
{
 if((motor_cs&CNC_ALL)==CNC_ALL)
 {
  memset(&x.move,0,sizeof(x.move));
		memset(&y.move,0,sizeof(y.move));
		memset(&z.move,0,sizeof(z.move));
		memset(&a.move,0,sizeof(a.move));
 }
 else
 {
  if((motor_cs&CNC_X)==CNC_X){memset(&x.move,0,sizeof(x.move));}
		if((motor_cs&CNC_Y)==CNC_Y){memset(&y.move,0,sizeof(y.move));}
		if((motor_cs&CNC_Z)==CNC_Z){memset(&z.move,0,sizeof(z.move));}
		if((motor_cs&CNC_A)==CNC_A){memset(&a.move,0,sizeof(a.move));}
 }
}

/*
*********************************************************************************************************
*	函 数 名: cnc_dis
*	功能说明: 显示坐标值函数
*	形    参: 轴选   gs=(gear.gs*cp+buff)/gear.cp
*	返 回 值: 无
*********************************************************************************************************
*/
int32_t cnc_dis(CNC_MCS motor_cs)
{
	int32_t temp;
	switch(motor_cs)
	{
		case(CNC_X):x.move.gs_dis=(((int32_t)x.gear.gs*x.move.cp_dis)+x.move.cp_buff)/(int32_t)x.gear.cp;
		            temp=x.move.gs_dis;
		            break;
		case(CNC_Y):y.move.gs_dis=(((int32_t)y.gear.gs*y.move.cp_dis)+y.move.cp_buff)/(int32_t)y.gear.cp;
		            temp=y.move.gs_dis;
		            break;
		case(CNC_Z):z.move.gs_dis=(((int32_t)z.gear.gs*z.move.cp_dis)+z.move.cp_buff)/(int32_t)z.gear.cp;
		            temp=z.move.gs_dis;
		            break;
		case(CNC_A):a.move.gs_dis=(((int32_t)a.gear.gs*a.move.cp_dis)+a.move.cp_buff)/(int32_t)a.gear.cp;
		            temp=a.move.gs_dis;
		            break;
		default:temp=CNC_ERR_MCS;break;
	}
	return temp;
}
/***************************** lcj1031@foxmail.com (END OF FILE) *********************************/
