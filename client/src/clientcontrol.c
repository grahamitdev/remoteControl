#include "clientcontrol.h"
#include "client.h"
#include "clientview.h"
#include <stdio.h>


void client_login_menu_control(void)
{
	int choose = 0;
	while(1)
	{
		client_login_menu();
		scanf("%d",&choose);//阻塞
		switch(choose)
		{
			case 1://客户端登录
				client_login();
				break;
			case 2://客户端注册
				client_regist();
				break;
			case 0://发送关闭客户端套接字请求
				client_disconn();
				return;
			default:
				break;
		}
	}
}

void user_main_menu_control(void)
{
	int choose = 0;
	while(1)
	{
		user_main_menu();
		scanf("%d",&choose);
		switch(choose)
		{
			case 1://用户聊天
				user_chat();
				break;
			case 2://用户群聊
				user_discuss();
				break;
			case 3://用户shell
				user_shell();
				break;
			case 4://用户空间云盘
				//user_space();
				break;
			case 0://用户下线
				user_logoff();
				return;
			default:
				break;
		}
	}
}
void user_chat_menu_control(void)
{
	user_chat_menu();
}
int client_dialog_menu_control(void)
{
	int choose = 0;
	while(1)
	{
		client_dialog_menu();
		scanf("%d",&choose);
		switch(choose)
		{
			case 1://确定
				return 1;
				break;
			case 0://取消
				return 0;
				break;
			default:
				return 0;
				break;
		}
	}
}
