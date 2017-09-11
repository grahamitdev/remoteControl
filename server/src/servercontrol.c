#include "servercontrol.h"
#include "server.h"
#include "serverview.h"
#include <stdio.h>


void admin_login_menu_control(void)
{
	int choose = 0;
	while(1)
	{
		admin_login_menu();
		scanf("%d",&choose);
		switch(choose)
		{
			case 1://管理员登录系统
				admin_login_system();
				break;
			case 0://管理员关闭窗口
				return;
			default:
				break;
		}
	}
}
void server_main_menu_control()
{
	int choose = 0;
	while(1)
	{
		server_main_menu();
		scanf("%d",&choose);
		switch(choose)
		{
			case 1://启动服务器
				start_server();
				break;
			case 2://服务器状态
				//server_status();
				break;
			case 3://重启服务器
				//reboot_server();
				break;
			case 4://关闭服务器
				shutdown_server();
				return;
			default:
				break;
		}
	}
	
}
void server_status_menu_control(void);
void server_dialog_menu_control(void);
