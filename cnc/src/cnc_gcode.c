/*
*********************************************************************************************************
*
*	模块名称 : CNC控制
*	文件名称 : cnc_gcode.c
*	版    本 : V1.1
*	说    明 : G代码解析
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

cnc_gcode_api_typedef user_gcode;/*G代码驱动接口*/
extern cnc_user_typedef user;          /*user驱动接口*/

/*
*********************************************************************************************************
*	函 数 名: cnc_gcode_run
*	功能说明: 运行G代码
*	形    参: *pgcode 缓存G代码的数组 f_t *pfile 数组地址循环 uint16_t _f 自动速度  vpct 速度百分比
												8位  16位   32位     32位    8位   16位   8位   16位
												G/M  指令   X坐标    Y坐标   F/L   数据   L     数据
*	返 回 值: CNC_MSTATE 返回工作状态
*********************************************************************************************************
*/
CNC_MSTATE cnc_gcode_run(g_t *pcode,f_t *pfile,uint16_t _f,uint8_t _vpct)
{
 uint16_t i;
 uint8_t state;
	switch(pcode->gcode)    
	{
		case('G'):
			         switch(pcode->gdata)  
												{
													case(0):
                      state=cnc_xd(pcode->x,pcode->y,pcode->z,pcode->a,_f,_vpct);
													         break;
													case(10):for(i=0;i<pcode->ldata;i++)
                      {
                       state=cnc_xd(pcode->x,pcode->y,pcode->z,pcode->a,pcode->fdata,_vpct);
                       if(state==CNC_BREAK){break;}
                      }
													         break;
             case(11):for(i=0;i<pcode->ldata;i++)
                      {
                       state=cnc_xd(pcode->x,pcode->y,pcode->z,pcode->a,pcode->fdata,_vpct);
                       if(state==CNC_BREAK){break;}
                      }
													         break;
													case(4): user_gcode.pg04(pcode->ldata);
                      state=CNC_OK;
														        break;
             case(99):state=CNC_GCODE_FINSH;
                      break;
             case(90):pfile->xhn=pcode->ldata;
                      pfile->add_xh=pfile->add;
                      state=CNC_OK;
                      break;
             case(91):pfile->xhn--; 
                      if(pfile->xhn!=0){pfile->add=pfile->add_xh;}
                      state=CNC_OK;
                      break;
													default:break;
												}
			         break;
		case('M'):
			         switch(pcode->gdata)  
												{
													case(2): 
													         break;
													case(3):CNC_M03;
													         break;
													case(8):CNC_M08;
												          break;
													case(9):CNC_M09;
														        break;
													default:break;
												}
            state=CNC_OK;
			         break;
  default:break;
	}
 return state;
}

/*
*********************************************************************************************************
*	函 数 名: cnc_run
*	功能说明: 自动运行一组cnc代码
*	形    参: uint8_t *pf 需要运行的数组		_f 自动速度  vpct 速度百分比							
*	返 回 值: CNC_MSTATE 返回工作状态
*********************************************************************************************************
*/
CNC_STATE cnc_run(uint8_t *pf,uint16_t _f,uint8_t _vpct)
{
 uint16_t i;
 uint8_t state;
 g_t _g;
 f_t _fadd;
 memset(&_g,0,sizeof(_g));
 memset(&_fadd,0,sizeof(_fadd));
 for(;;)
 {
  
  _g=user_gcode.pf2g(0,pf,&_fadd);
  //cnc_user_disgcode(pf,_fadd.add);
  user.pdisgcode((char *)pf,_fadd.add);
  state=cnc_gcode_run(&_g,&_fadd,_f,_vpct);
  if((state==CNC_BREAK)||(state==CNC_GCODE_FINSH)){break;}
 }
 return state;
}
/***************************** lcj1031@foxmail.com (END OF FILE) *********************************/
