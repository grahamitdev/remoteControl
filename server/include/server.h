#ifndef _SERVER_H_
#define _SERVER_H_
#include <pthread.h>

//服务器连接最大客户端数
#define NUMMAX      5
//包头宏  
#define LOGIN		1
#define REGIST		2
#define SHELL		3
#define CHAT		4
#define DISCUSS		5
#define LOGOFF		6
#define DISCONN		7
#define HEART		0

//是否打印log宏
#define PRINTF      1



//数据包结构体
typedef struct package{
	long type;
	char name[20];
	char passwd[20];
	char ip[16];
	int  clientfd;
	char onlineName[NUMMAX][20];
	char otherName[20];
	char text[1024];
}Pack;

//主线程向子线程传递的数据结构体
typedef struct config{
	int clientfd;
	pthread_t pthread;
	char IP[16];
	unsigned short PORT;
}Conf;



/************admin登录菜单函数*****************/
void admin_login_system();
int login();

/************服务器主菜单函数******************/
void start_server(void);
void sig_handler(int signo);
char *get_system_ipv4(char *ip);
void *th_fn(void *arg);
void create_pthread(Pack package);

void *th_fn_login(void *arg);
void *th_fn_regist(void *arg);
void *th_fn_shell(void *arg);
void *th_fn_chat(void *arg);
void *th_fn_discuss(void *arg);
void *th_fn_logoff(void *arg);
void *th_fn_disconn(void *arg);
void *th_fn_heart(void *arg);

void server_status(void);
void reboot_server(void);
void shutdown_server(void);

/**********************tool************************/
void get_local_time(char *time);
char *resolution(char *p,char *filename,char *value);
void *th_fn_send_heart(void *arg);
void printf_log(int level,int socketfd,char *log);
#endif
