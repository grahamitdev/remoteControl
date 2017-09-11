#ifndef _CLIENT_H_
#define _CLIENT_H_

#define NUMMAX     5

#define LOGIN      1
#define REGIST     2
#define SHELL      3
#define CHAT       4
#define DISCUSS    5
#define LOGOFF     6
#define DISCONN    7
#define HEART      0

//socket数据包结构体
typedef struct package{
	long type;
	char name[20];
	char passwd[20];
	char ip[16];
	int  clientfd;
	char onlineName[NUMMAX][20];//最多允许5个用户同时on聊天
	char otherName[20];
	char text[1024];
}Pack;



/**************************连接服务器一级菜单*******************/
int connect_server(void);
void sig_handler(int signo);
void *th_fn(void *arg);
void *th_fn_heart(void *arg);
void client_login(void);
void client_regist(void);
void user_shell(void);
void printf_shell(void);
void *th_fn_chat_rcv(void *arg);
void user_chat(void);
void *th_fn_discuss_rcv(void *arg);
void user_discuss(void);
void user_logoff(void);
void client_disconn(void);
char *get_system_ipv4(char *ip);
char *resolution(char *p,char *filename,char *value);
//void printf_log(int level,char *log);




#endif
