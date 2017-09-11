#include "clientview.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


void client_login_menu(void)
{
	srand(time(NULL));
	int color = rand() % 6 + 31;
	system("clear");
	printf("\n");
	printf("\033[%dm\t\t—客户端———————————————————关闭—\n",color);
	printf("\t\t|                             |\n");
	printf("\t\t|      Wechat the world       |\n");
	printf("\t\t|                             |\n");
	printf("\t\t|—————————————————————————————|\n");
	printf("\t\t|            登录             |\n");
	printf("\t\t|            注册             |\n");
	printf("\t\t———————————————————————————————\n");
	printf("\t\t选择:\033[0m");
}
void user_main_menu(void)
{
	srand(time(NULL));
	int color = rand() % 6 + 31;
	system("clear");
	printf("\n");
	printf("\033[%dm\t\t—客户端———————————————————关闭—\n",color);
	printf("\t\t|                             |\n");
	printf("\t\t|      Welcome to Wechat      |\n");
	printf("\t\t|                             |\n");
	printf("\t\t|—————————————————————————————|\n");
	printf("\t\t|            聊天             |\n");
	printf("\t\t|            群聊             |\n");
	printf("\t\t|            shell            |\n");
	printf("\t\t|           云盘空间          |\n");
	printf("\t\t———————————————————————————————\n");
	printf("\t\t选择:\033[0m");

}
void user_chat_menu(void)
{
	srand(time(NULL));
	int color = rand() % 6 + 31;
	printf("\033[%dm\t\t—客户端———————————————————关闭—\n",color);
	printf("\t\t|                             |\n");
	printf("\t\t|        在线联系人列表       |\n");
	printf("\t\t|                             |\n");
	printf("\t\t|—————————————————————————————|\n");
	printf("\t\t| (^.^) walter                |\n");
	printf("\t\t| (^.^) zhang                 |\n");
	printf("\t\t———————————————————————————————\n");
	printf("\t\t选择:\033[0m");
}
//void client_chats_menu(void);
//void client_shell_menu(void);
void client_dialog_menu(void)
{
	srand(time(NULL));
	int color = rand() % 6 + 31;
	printf("\033[%dm\t\t—对话框——————————————————关闭—\n",color);
	printf("\t\t|                            |\n");
	printf("\t\t|         确定  取消         |\n");
	printf("\t\t|                            |\n");
	printf("\t\t——————————————————————————————\n");
	printf("\t\t选择:\033[0m");
}

