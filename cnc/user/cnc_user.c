/*
*********************************************************************************************************
*
*	模块名称 : CNC用户驱动
*	文件名称 : user_cnc.c
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
#include "guiapp_user.h"
#include "user_all.h"

uint8_t cnc_user_rkey_block(void);
uint8_t cnc_user_rkey_nblock(void);
void    cnc_user_diszb(uint8_t state);
void    cnc_user_disn(int16_t n);
void    cnc_user_diszt(uint8_t mode,uint8_t state);
void    cnc_user_disgcode(char *pfile,uint16_t add);
void    cnc_user_wr_parameter(void);

cnc_user_typedef user_driver=
{
	.pkey_block = cnc_user_rkey_block,
 .pkey_nblock = cnc_user_rkey_nblock,
	.pdiszb = cnc_user_diszb,
 .pdisn = cnc_user_disn,
 .pdiszt = cnc_user_diszt,
 .pdisgcode = cnc_user_disgcode,
 .pwr_parameter = cnc_user_wr_parameter,
};

/*
*********************************************************************************************************
*	函 数 名: user_rkey_block
*	功能说明: 判断是否读取到按键  阻塞式按键
*	形    参: key 需要判断的按键
*	返 回 值: 当前按下的键值
*********************************************************************************************************
*/
uint8_t cnc_user_rkey_block(void)
{
	switch(key.now)
	{
		case(K_XU):return CNC_KEY_XP;
  case(K_XD):return CNC_KEY_XN;
  case(K_YL):return CNC_KEY_YP;
  case(K_YR):return CNC_KEY_YN;
  case(K_ZP):return CNC_KEY_ZP;
  case(K_ZN):return CNC_KEY_ZN;
  case(K_AP):return CNC_KEY_AP;
  case(K_AN):return CNC_KEY_AN;
  case(K_START):return CNC_KEY_QD; /*启动*/
  case(K_STOP):return CNC_KEY_ZT;  /*暂停*/
  case(K_HOME):return CNC_KEY_TC;  /*退出*/
//  case(K_UP):bsp_GetKey();if(key.buf==K_UP){return CNC_KEY_VU;}else{return CNC_KEY_NONE;}
//  case(K_DOWN):bsp_GetKey();if(key.buf==K_DOWN){return CNC_KEY_VD;}else{return CNC_KEY_NONE;}
		default:return CNC_KEY_NONE;
	}	
}

/*
*********************************************************************************************************
*	函 数 名: user_rkey_nblock
*	功能说明: 判断是否读取到按键  非阻塞式按键
*	形    参: key 需要判断的按键
*	返 回 值: 当前按下的键值
*********************************************************************************************************
*/
uint8_t cnc_user_rkey_nblock(void)
{
 bsp_GetKey();
 if(key.buf==K_UP){return CNC_KEY_VU;}
 if(key.buf==K_DOWN){return CNC_KEY_VD;}
} 

/*
*********************************************************************************************************
*	函 数 名: cnc_user_wr_parameter
*	功能说明: 存取所有参数，并用宏定义控制参数上下限的值
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void cnc_user_wr_parameter(void)
{
 save_parameter_all();
 read_parameter_all();
} 

/*
*********************************************************************************************************
*	函 数 名: cnc_user_diszb
*	功能说明: 显示坐标值主函数
*	形    参: state CNC运行在什么状态
*	返 回 值: 无
*********************************************************************************************************
*/
void cnc_user_diszb(uint8_t state)
{
	//cnc_dis(CNC_X);
	//cnc_dis(CNC_Y);
 sprintf(mstr.zb1buff,"X:%+09.3f",(float)	cnc_dis(CNC_X)/1000);
 sprintf(mstr.zb2buff,"Y:%+09.3f",(float)	cnc_dis(CNC_Y)/1000);
 sprintf(mstr.zb3buff,"Z:%+09.3f",(float)	cnc_dis(CNC_Z)/1000);
 sprintf(mstr.zb4buff,"A:%+09.3f",(float)	cnc_dis(CNC_A)/1000);
 mstr.zb4buff[12]=mstr.zb4buff[11];
 mstr.zb4buff[11]=mstr.zb4buff[10];
 mstr.zb4buff[10]=mstr.zb4buff[9];
 mstr.zb4buff[9]=mstr.zb4buff[8];
 mstr.zb4buff[7]=0xc2;/*小数点转换成度*/
 mstr.zb4buff[8]=0xb0;
	WM_SendMessageNoPara(hWin_main,MS_ZB);  /*显示字符串,后台进行重绘*/
 GUI_Exec();
}

/*
*********************************************************************************************************
*	函 数 名: cnc_user_disn
*	功能说明: 自动运行的时候显示自动速度百分比
*	形    参: int16_t n 范围-50 到 50
*	返 回 值: 无
*********************************************************************************************************
*/
void cnc_user_disn(int16_t n)
{
 user_cnc.zdn=n; 
 sprintf(mstr.zdnbuff, "自动  F%04d  %03d%%",user_cnc.zd,100+user_cnc.zdn);
 WM_SendMessageNoPara(hWin_main,MS_ZDN);  /*显示字符串,后台进行重绘*/
 //GUI_Exec();
}

/*
*********************************************************************************************************
*	函 数 名: cnc_user_diszt
*	功能说明: 显示运动模式和状态
*	形    参:  mode 自动/手动  state 暂停/运行
*	返 回 值: 无
*********************************************************************************************************
*/
void cnc_user_diszt(uint8_t mode,uint8_t state)
{
 if(mode==CNC_ZD){user_cnc.mode=1;sprintf(mstr.modebuff, "自动");}
 if(mode==CNC_SD){user_cnc.mode=0;sprintf(mstr.modebuff, "手动");}
 if(state==CNC_MNRUN){sprintf(mstr.ztbuff, "");}
 if(state==CNC_MBREAK){sprintf(mstr.ztbuff, "");}
 if(state==CNC_MRUN){sprintf(mstr.ztbuff, "运行");}
 if(state==CNC_MSTOP){sprintf(mstr.ztbuff, "暂停");}
 WM_SendMessageNoPara(hWin_main,MS_MODE);  /*显示字符串,后台进行重绘*/
 WM_SendMessageNoPara(hWin_main,MS_ZT);    /*显示字符串,后台进行重绘*/
 GUI_Exec();
}

/*数组排序*/
void bubble(uint16_t *a,uint16_t n) /*定义两个参数：数组首地址与数组大小*/

{

		int i,j,temp;

		for(i=0;i<n-1;i++)

		for(j=i+1;j<n;j++) /*注意循环的上下限*/

		if(a[i  ]>a[j]) {

		temp=a[i  ];

		a[i  ]=a[j];

		a[j]=temp;

		}

}

/*
*********************************************************************************************************
*	函 数 名: cnc_user_disgcode
*	功能说明: 显示运动中的G代码
*	形    参: *f 文件 add文件地址 
*	返 回 值: 无
*********************************************************************************************************
*/
void cnc_user_disgcode(char *pfile,uint16_t add)
{
 /*
   N0
   N1
   N2  //当前运行，彩色显示，传导来的地址是N2结尾处
   N3
   N4
 */
 uint8_t i=0;
 uint8_t n=0;   /*N的计数器，每次显示5行N*/
 uint16_t t_add;   /*add的备份*/
 uint16_t nadd[6];/*N地址偏移*/
 uint16_t cadd; /*当前运行地址*/
 t_add=--add;
 for(add=add;add>0;add--)
 {
  if(*(pfile+add)=='N'){nadd[n]=add;n++;}
  if(n==3){break;}
 }
 add=t_add;
 for(add=add;add<4096;add++)
 {
  if(*(pfile+add)=='N'){nadd[n]=add;n++;}
  if((*(pfile+add)==0xFF)||(n==6)){break;}
 }
 n--;
 cadd=nadd[0];
 bubble(nadd,6);
 
 for(i=0;i<n;i++)
 {
  memset(mstr.yh1buff,0,sizeof(mstr.yh1buff));/*清缓存**/
  if(cadd==nadd[i]){mstr.yh1buff[0]='>';memcpy(mstr.yh1buff+1,pfile+nadd[i],nadd[i+1]-nadd[i]);}
  else{memcpy(mstr.yh1buff,pfile+nadd[i],nadd[i+1]-nadd[i]);}
  if(i==0){WM_SendMessageNoPara(hWin_main,MS_USER_0);}  /*显示字符串,后台进行重绘*/
  else    {WM_SendMessageNoPara(hWin_main,MS_USER_1);} /*显示字符串,后台进行重绘*/
 }

// memset(mstr.yh1buff,0,sizeof(mstr.yh1buff));/*清缓存**/
// if(cadd==nadd[0]){mstr.yh1buff[0]='>';memcpy(mstr.yh1buff+1,pfile+nadd[0],nadd[1]-nadd[0]);}
// else{memcpy(mstr.yh1buff,pfile+nadd[0],nadd[1]-nadd[0]);}
// WM_SendMessageNoPara(hWin_main,MS_USER_0);  /*显示字符串,后台进行重绘*/
// //GUI_Exec();

// memset(mstr.yh1buff,0,sizeof(mstr.yh1buff));/*清缓存*/
// if(cadd==nadd[1]){mstr.yh1buff[0]='>';memcpy(mstr.yh1buff+1,pfile+nadd[1],nadd[2]-nadd[1]);}
// else{memcpy(mstr.yh1buff,pfile+nadd[1],nadd[2]-nadd[1]);}
// WM_SendMessageNoPara(hWin_main,MS_USER_1);  /*显示字符串,后台进行重绘*/
// //GUI_Exec();

// memset(mstr.yh1buff,0,sizeof(mstr.yh1buff));/*清缓存*/
// if(cadd==nadd[2]){mstr.yh1buff[0]='>';memcpy(mstr.yh1buff+1,pfile+nadd[2],nadd[3]-nadd[2]);}
// else{memcpy(mstr.yh1buff,pfile+nadd[2],nadd[3]-nadd[2]);}
// WM_SendMessageNoPara(hWin_main,MS_USER_1);  /*显示字符串,后台进行重绘*/
// //GUI_Exec();

// memset(mstr.yh1buff,0,sizeof(mstr.yh1buff));/*清缓存*/
// if(cadd==nadd[3]){mstr.yh1buff[0]='>';memcpy(mstr.yh1buff+1,pfile+nadd[3],nadd[4]-nadd[3]);}
// else{memcpy(mstr.yh1buff,pfile+nadd[3],nadd[4]-nadd[3]);}
// WM_SendMessageNoPara(hWin_main,MS_USER_1);  /*显示字符串,后台进行重绘*/
// //GUI_Exec();

// memset(mstr.yh1buff,0,sizeof(mstr.yh1buff));/*清缓存*/
// if(cadd==nadd[4]){mstr.yh1buff[0]='>';memcpy(mstr.yh1buff+1,pfile+nadd[4],nadd[5]-nadd[4]);}
// else{memcpy(mstr.yh1buff,pfile+nadd[4],nadd[5]-nadd[4]);}
// WM_SendMessageNoPara(hWin_main,MS_USER_1);  /*显示字符串,后台进行重绘*/
// //GUI_Exec();

}

/***************************** lcj1031@foxmail.com (END OF FILE) *********************************/
