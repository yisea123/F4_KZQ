/*
*********************************************************************************************************
*
*	模块名称 : G代码需要的驱动
*	文件名称 : user_cnc_gcode.c
*	版    本 : V1.1
*	说    明 : 需要用户提供驱动
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
#include "bsp.h"
#include "user_all.h"

g_t gcode_f2g(uint8_t fn,uint8_t *pf,f_t *pfile);
void gcode_g04(uint32_t times);

cnc_gcode_api_typedef gcode_driver=
{
 gcode_f2g,
	gcode_g04,
};

/*
*********************************************************************************************************
*	函 数 名: gcode_f2g_0
*	功能说明: 0类型文件解析
*	形    参: uint8_t *pf  数组 add 数组地址
*	返 回 值: 无
*********************************************************************************************************
*/ 
g_t gcode_f2g_0(uint8_t *pf,f_t *pfile)
{
 g_t _g;
 float _data;
 char buf[16];
 uint16_t i=0;
 uint16_t j=0;
 uint16_t n=0;
 n=pfile->add;
 memset(&_g,0,sizeof(_g)); memset(buf,0,sizeof(buf));
 _g.ldata=1;
 for(;;)
 {
  switch(*(pf+n))
  {
   case 'N':
          memcpy(buf,pg+n+1,4);
          n+=4;
          _g.number=atoi(buf);
          memset(buf,0,sizeof(buf));
          break;
   case 'G':
          _g.gcode='G';
										memcpy(buf,pg+n+1,2);
										_g.gdata=atoi(buf);
          n+=2;
          memset(buf,0,sizeof(buf));
          break;
   case 'M':_g.gcode='M';
											memcpy(buf,pg+n+1,2);
										_g.gdata=atoi(buf);
          n+=2;
          memset(buf,0,sizeof(buf));
          break; 
   case 'F':
          j=0;
          for(i=0;i<=4;i++)/*字母后面的数据最多4位 7+2 7位数字 2位.-*/
          {
           if(((pg[n+i])==' ')||((pg[n+i])=='N')||((pg[n+i])=='L')){break;} /*遇到 空格 N退出*/
           if((pg[n+i]>='0')&&(pg[n+i]<='9')){buf[j++]=pg[n+i];}/*0~9*/
          }
										_g.fdata=atoi(buf);
          n=n+i-1;
          memset(buf,0,sizeof(buf));
          break; 
   case 'L':
										j=0;
          for(i=0;i<=4;i++)/*字母后面的数据最多4位 7+2 7位数字 2位.-*/
          {
           if(((pg[n+i])==' ')||((pg[n+i])=='N')||((pg[n+i])=='F')){break;} /*遇到 空格 N退出*/
           if((pg[n+i]>='0')&&(pg[n+i]<='9')){buf[j++]=pg[n+i];}/*0~9*/
          }
										_g.ldata=atoi(buf);
          n=n+i-1;
          memset(buf,0,sizeof(buf));
          break;
   case 'W':/*小数点之前的数*/
          j=0;
          for(i=0;i<sizeof(buf);i++)/*字母后面的数据最多9位 7+2 7位数字 2位.-*/
          {
           if(((pg[n+i])==' ')||((pg[n+i])=='N')){break;} /*遇到 空格 N退出*/
           if(((pg[n+i]>='0')&&(pg[n+i]<='9'))||(pg[n+i]=='-')||(pg[n+i]=='.')){buf[j++]=pg[n+i];}/*0~9*/
          }
          _data=atof(buf);
          _g.x=_data*1000;/*取出的值是小数都X1000*/
          memset(buf,0,sizeof(buf));/*清缓存*/
          /*小数点之后的数*/
          n=n+i-1;
          memset(buf,0,sizeof(buf));
          break;
   case 'U':
          j=0;
          for(i=0;i<sizeof(buf);i++)/*字母后面的数据最多9位 7+2 7位数字 2位.-*/
          {
           if(((pg[n+i])==' ')||((pg[n+i])=='N')){break;} /*遇到 空格 N退出*/
           if(((pg[n+i]>='0')&&(pg[n+i]<='9'))||(pg[n+i]=='-')||(pg[n+i]=='.')){buf[j++]=pg[n+i];}/*0~9*/
          }
          _data=atof(buf);
          _g.y=_data*1000;/*取出的值是小数都X1000*/
          memset(buf,0,sizeof(buf));/*清缓存*/
          /*小数点之后的数*/
          n=n+i-1;
          memset(buf,0,sizeof(buf));
          break;
   case 'V':
          j=0;
          for(i=0;i<sizeof(buf);i++)/*字母后面的数据最多9位 7+2 7位数字 2位.-*/
          {
           if(((pg[n+i])==' ')||((pg[n+i])=='N')){break;} /*遇到 空格 N退出*/
           if(((pg[n+i]>='0')&&(pg[n+i]<='9'))||(pg[n+i]=='-')||(pg[n+i]=='.')){buf[j++]=pg[n+i];}/*0~9*/
          }
          _data=atof(buf);
          _g.z=_data*1000;/*取出的值是小数都X1000*/
          memset(buf,0,sizeof(buf));/*清缓存*/
          /*小数点之后的数*/
          n=n+i-1;
          memset(buf,0,sizeof(buf));
          break;
   case 'A':
          j=0;
          for(i=0;i<sizeof(buf);i++)/*字母后面的数据最多9位 7+2 7位数字 2位.-*/
          {
           if(((pg[n+i])==' ')||((pg[n+i])=='N')){break;} /*遇到 空格 N退出*/
           if(((pg[n+i]>='0')&&(pg[n+i]<='9'))||(pg[n+i]=='-')||(pg[n+i]=='.')){buf[j++]=pg[n+i];}/*0~9*/
          }
          _data=atof(buf);
          _g.a=_data*1000;/*取出的值是小数都X1000*/
          memset(buf,0,sizeof(buf));/*清缓存*/
          /*小数点之后的数*/
          n=n+i-1;
          memset(buf,0,sizeof(buf));
          break;
   deault:break;       
  }
  if(pg[n++]==ENTER){if(n>1){break;}}
 }
 pfile->add=n;
 return _g;
}


/*
*********************************************************************************************************
*	函 数 名: gcode_f2g
*	功能说明: 数组解析成G代码
*	形    参: uint8_t fn 文件类型  uint8_t *pf  数组 uint16_t add 数组地址
*	返 回 值: 无
*********************************************************************************************************
*/ 
g_t gcode_f2g(uint8_t fn,uint8_t *pf,f_t *pfile)
{
 g_t _g;
 switch(fn)
 {
  case 0: _g=gcode_f2g_0(pf,pfile);
           break;
 default:break;
 }
 return _g;
}


/*
*********************************************************************************************************
*	函 数 名: gcode_g04
*	功能说明: g04延时函数
*	形    参: times 单位1ms
*	返 回 值: 无
*********************************************************************************************************
*/
void gcode_g04(uint32_t times)
{
	HAL_Delay(times);
}	

/***************************** lcj1031@foxmail.com (END OF FILE) *********************************/
