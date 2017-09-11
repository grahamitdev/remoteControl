#include "client.h"
#include "clientcontrol.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

int clientfd = 0;
Pack package_snd = {};
Pack package_rcv = {};
Pack package_rcv_login = {};	sem_t sem_login;
Pack package_rcv_regist = {};	sem_t sem_regist;
Pack package_rcv_shell = {};	sem_t sem_shell;
Pack package_rcv_chat = {};		sem_t sem_chat;
Pack package_rcv_discuss = {};	sem_t sem_discuss;
Pack package_rcv_logoff = {};	sem_t sem_logoff;
Pack package_rcv_disconn = {};	sem_t sem_disconn;
Pack package_rcv_heart = {};	sem_t sem_heart;

Pack package_snd_login = {LOGIN};	
Pack package_snd_regist = {REGIST};	
Pack package_snd_shell = {SHELL};	
Pack package_snd_chat = {CHAT};		
Pack package_snd_discuss = {DISCUSS};	
Pack package_snd_logoff = {LOGOFF};	
Pack package_snd_disconn = {DISCONN};
Pack package_snd_heart = {HEART};
char IP[16] = "";
unsigned short int PORT = 0;
int count = 0;
pthread_t pthread;
pthread_t pthread_heart;
int flag = 0;
int flag_discuss = 0;
int flag_friend = 0;
int flag_search = 0;
int connect_server(void)
{
	sem_init(&sem_login,0,0);
	sem_init(&sem_regist,0,0);
	sem_init(&sem_shell,0,0);
	sem_init(&sem_chat,0,0);
	sem_init(&sem_discuss,0,0);
	sem_init(&sem_logoff,0,0);
	sem_init(&sem_disconn,0,0);
	sem_init(&sem_heart,0,0);
	



	clientfd = socket(AF_INET,SOCK_STREAM,0);

	if(-1 == clientfd)
	{
		perror("clientfd error");
		return -1;
	}
	
	struct sockaddr_in s_addr;
	s_addr.sin_family = AF_INET;
	char port_array[1024] = "";
	if(NULL == resolution(port_array,"./config.txt","PORT"))
	{
		perror("resolution port error");
		return -1;
	}
	PORT = atoi(port_array);
	s_addr.sin_port = htons(PORT);
	//if(NULL == resolution(IP,"./config.txt","IP"))
	if(NULL == get_system_ipv4(IP))
	{
		perror("resolution IP error");
		return -1;
	}
	if(-1 == inet_pton(AF_INET,IP,&s_addr.sin_addr.s_addr))
	{
		perror("inet_ption IP error");
		return -1;
	}	
	if(-1 == connect(clientfd,(struct sockaddr *)&s_addr,sizeof(struct sockaddr)))
	{
		perror("connect error");
		return -1;
	}
	else
	{
		printf("连接服务器%s成功！\n",IP);
		sleep(2);
	}

	

    //收包线程	
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	
	int err = 0;
	if(0 != (err = pthread_create(&pthread,&attr,th_fn,NULL)))
	{
		perror("pthread error");
		return -1;
	}
	//发心跳包线程
	pthread_attr_t attr1;
	pthread_attr_init(&attr1);
	pthread_attr_setdetachstate(&attr1,PTHREAD_CREATE_DETACHED);
	if(0 != (err = pthread_create(&pthread_heart,&attr1,th_fn_heart,NULL)))
	{
		perror("pthread error");
		return -1;
	}
	client_login_menu_control();
	return 0;
}


void sig_handler(int signo)
{
	if(signo == SIGALRM)
	{
		if(count >= 3)
		{
			count = 0;
			alarm(5);
		}
		else
		{
			printf("5秒内收到服务器数据包小于3个，判定服务器关机，5秒后关闭客户端 ...\n");
			int i = 0;
			for(i = 5;i >= 1;--i)
			{
				printf("倒计时：%d 秒",i);
				sleep(1);
			}
			pthread_cancel(pthread);
			exit(-1);
		}
	}
}
void *th_fn(void *arg)
{
	count = 0;
	if(signal(SIGALRM,sig_handler) == SIG_ERR)
	{
		perror("signal sigalarm error");
		exit(-1);
	}
	alarm(5);

	while(1)
	{
		if(read(clientfd,&package_rcv,sizeof(Pack)) <= 0)
		{
			perror("读服务器套接字失败！");
			break;
		}
		++count;




		//解析数据包头
		switch(package_rcv.type)
		{
			case LOGIN: 
				package_rcv_login = package_rcv;
				sem_post(&sem_login);
				break;	
			case REGIST:
				package_rcv_regist = package_rcv;
				sem_post(&sem_regist);
				break;
			case SHELL:
				package_rcv_shell = package_rcv;
				sem_post(&sem_shell);
				break;
			case CHAT:
				package_rcv_chat = package_rcv;
				sem_post(&sem_chat);
				//临时消息
				if(flag == 0)
				{
					sem_wait(&sem_chat);
					if((strcmp(package_rcv_chat.text,"") != 0)\
					&&strcmp(package_rcv_chat.text,"login") != 0)
					{
						printf("\033[31m收到%s的消息:%s\033[0m\n",\
						package_rcv_chat.name,\
						package_rcv_chat.text);
					}
					if(strcmp(package_rcv_chat.onlineName[0],"") != 0)
					{
						printf("\033[31m好友%s上线！\033[0m\n",\
						package_rcv_chat.onlineName[0]);
					}
				}
				break;
			case DISCUSS:
				package_rcv_discuss = package_rcv;
				sem_post(&sem_discuss);
				if(flag_discuss == 0)
				{
					sem_wait(&sem_discuss);
					if(strcmp(package_rcv_discuss.text,"") != 0)
					{
						printf("\033[32m收到%s的群聊消息:%s\033[0m\n",\
						package_rcv_discuss.name,\
						package_rcv_discuss.text);
					}
				}
				break;
			case LOGOFF:
				package_rcv_logoff = package_rcv;
				sem_post(&sem_logoff);
				break;
			case DISCONN:
				package_rcv_disconn = package_rcv;
				sem_post(&sem_disconn);
				break;
			case HEART:
				//printf("serverheart\n");
				break;
			default:
				printf("不识别的包！\n");
				break;
		}
	}
	return NULL;
}
void *th_fn_heart(void *arg)
{
	while(1)
	{
		if(write(clientfd,&package_snd_heart,sizeof(Pack)) <= 0)
		{
			perror("send heart error");
			break;
		}
		//printf("send heart\n");
		sleep(1);
	}
	return NULL;
}


void client_login(void)
{	
	printf("账户名:");
	scanf("%s",package_snd_login.name);
	printf("密码:");
	scanf("%s",package_snd_login.passwd);
	//发送登录数据包
	if(write(clientfd,&package_snd_login,sizeof(Pack)) <= 0)//发数据包
	{
		perror("send error");
		return;
	}
	sem_wait(&sem_login);
	if(strcmp(package_rcv_login.text,"登录成功") == 0)
	{
		//进入用户菜单
		printf("登录成功！\n");
		sleep(1);
		user_main_menu_control();
	}
	else if(strcmp(package_rcv_login.text,"密码错误") == 0)
	{
		printf("密码错误！\n");
		sleep(2);
	}
	else if(strcmp(package_rcv_login.text,"已在线") == 0)
	{
		printf("您已在线！\n");
		sleep(2);
	}
	else if(strcmp(package_rcv_login.text,"用户不存在") == 0)
	{
		printf("您还没有注册，请先注册后再登录！\n");
		sleep(2);
	}
	else
	{
		printf("解析服务器数据包text出错！\n");
		sleep(2);
	}
}

void client_regist(void)
{
	char name[20] = "";
	char passwd[20] = "";
	char cfmPasswd[20] = "";
	printf("呢称:");
	scanf("%s",name);
	printf("请输入注册密码:");
	scanf("%s",passwd);
	printf("再输入一次密码:");
	scanf("%s",cfmPasswd);
	if(strcmp(passwd,cfmPasswd) != 0)
	{
		printf("两次密码不一致，请重新注册！\n");
		return;
	}

	strcpy(package_snd_regist.name,name);
	strcpy(package_snd_regist.passwd,passwd);
	if(write(clientfd,&package_snd_regist,sizeof(Pack)) <= 0)
	{
		perror("send error");
		return;
	}
	sem_wait(&sem_regist);
	Pack package = package_rcv_regist;
	if(strcmp(package.text,"注册成功") == 0)
	{
		printf("注册成功，前往登录！\n");
		sleep(1);
		return;
	}
	else if(strcmp(package.text,"已注册") == 0)
	{
		printf("您已经注册，不要重复注册！\n");
		sleep(1);
	}
	else
	{
		printf("解析服务器数据包text出错！\n");
	}
}
void user_shell(void)
{
	getchar();
	while(1)
	{
		printf("\033[32m%s\033[0m:/~$ ",IP);
		fgets(package_snd_shell.text,100,stdin);
		package_snd_shell.text[strlen(package_snd_shell.text) - 1] = '\0';
		if(strcmp(package_snd_shell.text,"exit") == 0)
		{
			return;
		}
		if(write(clientfd,&package_snd_shell,sizeof(Pack)) <= 0)
		{
			perror("write shell error");
			return;
		}
		printf_shell();				
	}
}
void printf_shell(void)
{
	while(1)
	{
		sem_wait(&sem_shell);
		if(strcmp(package_rcv_shell.text,"end") == 0)
		{
			break;
		}
		printf("%s",package_rcv_shell.text);
	}
}
void *th_fn_chat_rcv(void *arg)
{
	while(1)
	{
		sem_wait(&sem_chat);
		flag_search = 1;
		//没有更新在线列表,只是聊天信息
		if(strcmp(package_rcv_chat.onlineName[0],"") == 0)
		{
			if(strcmp(package_rcv_chat.text,"好友现在都不在线") == 0)
			{
				printf("好友现在都不在线！\n");
				flag_friend = 0;
			}
			else if(strcmp(package_rcv_chat.text,"您找的人不在线") == 0)
			{
				printf("您找的人不在线！\n");
				flag_search = 0;
			}
			else if(strcmp(package_rcv_chat.text,"传递消息成功") == 0)
			{
				printf("对方已阅\n");
			}
			else if(strcmp(package_rcv_chat.text,"对方在线正忙") == 0)
			{
				printf("对方正忙,已提醒对方\n");
			}
			else if(strcmp(package_rcv_chat.text,"不允许给自己发送消息") == 0)
			{
				printf("拒绝给自己发消息！\n");
			}
			else
			{//打印有效信息
				printf("\033[31m%s:%s\033[0m\n",\
				package_rcv_chat.name,\
				package_rcv_chat.text);
			}			
		}
		else if((strcmp(package_rcv_chat.onlineName[0],"") != 0)&&\
		strcmp(package_rcv_chat.text,"login") != 0)//更新在线用户列表
		{
			int num = 0;
			while(strcmp(package_rcv_chat.onlineName[num],"") != 0)
			{
				++num;
			}
			srand(time(NULL));
			int color = rand() % 6 + 31;
			printf("\n\n");
			printf("\033[%dm\t\t—客户端———————————————————关闭—\n",color);
			printf("\t\t|                             |\n");
			printf("\t\t|        在线好友列表         |\n");
			printf("\t\t|                             |\n");
			printf("\t\t|—————————————————————————————|\n\033[0m");
			int i = 0;
			for(i = 0;i < num;++i)
			{
				printf("\033[%dm\t\t| (^.^) %s                |\n",\
				color,package_rcv_chat.onlineName[i]);
			}
			printf("\t\t|                             |\n");
			printf("\t\t———————————————————————————————\n\033[0m");
			flag_friend = i;
		}
	}
}
void user_chat(void)
{
	flag = 1;

	int err = 0;
	pthread_t pthread_rcv;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	if((err = pthread_create(&pthread_rcv,&attr,th_fn_chat_rcv,NULL)) != 0)
	{
		perror("pthread_rcv error");
		return;
	}
	//发一个空包过去让服务器把自己的on置为1
	//等待接收在线联系人列表
	strcpy(package_snd_chat.otherName,"");
	strcpy(package_snd_chat.text,"");
	if(write(clientfd,&package_snd_chat,sizeof(Pack)) <= 0)
	{
		perror("进入聊天失败！");
		return;
	}
	sleep(1);//等在线列表加载好
	if(flag_friend == 0)
	{
		printf("没朋友\n");
		return;
	}
	while(1)
	{
		printf("选择在线好友姓名:\n");
		scanf("%s",package_snd_chat.otherName);
		getchar();
		while(1)
		{		
			if(flag_search == 0)
			{
				flag_search = 1;
				break;
			}
			//scanf("%s",package_snd_chat.text);
			fgets(package_snd_chat.text,1024,stdin);
			package_snd_chat.text[strlen(package_snd_chat.text) - 1] = '\0';
			if(strcmp(package_snd_chat.text,"bye") == 0)
			{
				if(write(clientfd,&package_snd_chat,sizeof(Pack)) <= 0)
				{
					perror("sand text error");
					return;
				}
				break;
			}
			else if(strcmp(package_snd_chat.text,"exit") == 0)
			{
				flag = 0;
				pthread_cancel(pthread_rcv);
				return;
			}
			else
			{
				if(write(clientfd,&package_snd_chat,sizeof(Pack)) <= 0)
				{
					perror("sand text error");
					return;
				}
			}
			sleep(1);
		}
	}
}
void *th_fn_discuss_rcv(void *arg)
{
	while(1)
	{
		sem_wait(&sem_discuss);
		if(strcmp(package_rcv_discuss.text,"群发成功") == 0)
		{
			printf("群发成功!\n");
		}
		else if(strcmp(package_rcv_discuss.text,"群列没有好友在线") == 0)
		{
			printf("群里没有好有在线！\n");
		}
		else
		{
			printf("\033[32m%s群发:%s\033[0m\n",\
			package_rcv_discuss.name,\
			package_rcv_discuss.text);
		}
	}
}
void user_discuss(void)
{
	flag_discuss = 1;
	int err = 0;
	pthread_t pthread_discuss_rcv;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	if((err = pthread_create(&pthread_discuss_rcv,&attr,th_fn_discuss_rcv,NULL)) != 0)
	{
		perror("pthread_discuss_rcv error");
		return;
	}
	getchar();
	while(1)
	{
		printf("群发:");
		//scanf("%s",package_snd_discuss.text);
		fgets(package_snd_discuss.text,1024,stdin);
		package_snd_discuss.text[strlen(package_snd_discuss.text) - 1] = '\0';
		if(strcmp(package_snd_discuss.text,"exit") == 0)
		{
			flag_discuss = 0;
			pthread_cancel(pthread_discuss_rcv);
			return;
		}
		if(write(clientfd,&package_snd_discuss,sizeof(Pack)) <= 0)
		{
			perror("send discuss error");
			return;
		}
	}
}



	
void user_logoff(void)
{
	//发一个下线空包
	if(write(clientfd,&package_snd_logoff,sizeof(Pack)) <= 0)
	{
		perror("send error");
		return;
	}
	sem_wait(&sem_logoff);
	if(strcpy(package_rcv_logoff.text,"允许下线") == 0)
	{
		printf("服务器允许下线\n");
		client_login_menu_control();
		pthread_cancel(pthread);
		return;
	}
}

void client_disconn(void)
{
	if(write(clientfd,&package_snd_disconn,sizeof(Pack)) <= 0)
	{
		perror("write close error");
		return;
	}
	sem_wait(&sem_disconn);
	if(strcpy(package_rcv_disconn.text,"允许关闭客户端") == 0)
	{
		printf("服务器允许关闭客户端！\n");
		exit(-1);
	}
		
}



char *get_system_ipv4(char *ip)
{	
	int fd = open("ipv4.txt",O_RDWR|O_CREAT|O_TRUNC,0666);
	if(fd == -1)
	{
		perror("open file error");
		return NULL;
	}
	int fd_save = dup(1);
	dup2(fd,1);
	system("ifconfig ens33 | grep \"inet 地址:\" | awk '{ print $2}' | awk -F: '{print $2}'");
	dup2(fd_save,1);
	close(fd);
	FILE *fp = fopen("ipv4.txt","r");
	if(fp == NULL)
	{
		perror("fopen file error");
		return NULL;
	}
	fgets(ip,16,fp);
	int i = 0;
	for(i = 0;i < 16;++i)
	{
		if(ip[i] - '\n' == 0)
		{
			ip[i] = '\0';
		}
	}
	fclose(fp);
	return ip;
}
char *resolution(char *p,char *filename,char *value)
{

	FILE *fp = fopen(filename,"r");
	if(fp == NULL)
	{
		printf("yes\n");
		return NULL;
	}
	while(1)
	{
		char buff_line[1024] = "";
		char buff_config[1024] = "";
		char buff_sym[1024] = "";
		char buff_value[1024] = "";
		if(fgets(buff_line,1024,fp) == NULL)
		{
			break;
		}
		sscanf(buff_line,"%s %s %s",buff_config,buff_sym,buff_value);
		if(0 == strcmp(buff_config,value))
		{
			fclose(fp);
			strcpy(p,buff_value);
			return p;
		}
	}
	fclose(fp);
	return NULL;
}
#if 0
//打印log到屏幕
//打印log到server_log.txt
void printf_log(int level,int socketfd,char *log)
{
	char localtime[50] = "";
	get_local_time(localtime);
	FILE *fp = fopen("server_log.txt","a+");
	if(PRINTF == 1)
	{
		switch (level)
		{
			case 1:
				printf("\033[32m%s IP:%s  PORT:%6hu  socket:%d\t[done]%s\033[0m\n",\
				localtime,IP,PORT,socketfd,log);
				fprintf(fp,"%s IP:%s  PORT:%6hu  socket:%d\t[done]%s\n",\
				localtime,IP,PORT,socketfd,log);
				break;
			case 2:
				printf("\033[33m%s IP:%s  PORT:%6hu  socket:%d\t[note]%s\033[0m\n",\
				localtime,IP,PORT,socketfd,log);			
				fprintf(fp,"%s IP:%s  PORT:%6hu  socket:%d\t[note]%s\n",\
				localtime,IP,PORT,socketfd,log);
				break;
			case 3:
				printf("\033[31m%s IP:%s  PORT:%6hu  socket:%d\t[error]%s\033[0m\n",\
				localtime,IP,PORT,socketfd,log);
				fprintf(fp,"%s IP:%s  PORT:%6hu  socket:%d\t[error]%s\n",\
				localtime,IP,PORT,socketfd,log);
				break;
			default:
				break;
		}
	}
	else
	{
		switch(level)
		{
			case 1:
				fprintf(fp,"%s IP:%s  PORT:%6hu  socket_s:%d\t[done]%s\n",\
				localtime,IP,PORT,socketfd,log);
				break;
			case 2:
				fprintf(fp,"%s IP:%s  PORT:%6hu  socket_s:%d\t[note]%s\n",\
				localtime,IP,PORT,socketfd,log);
				break;
			case 3:
				fprintf(fp,"%s IP:%s  PORT:%6hu  socket_s:%d\t[error]%s\n",\
				localtime,IP,PORT,socketfd,log);
				break;
			default:
				break;
			
		}
	}
}
#endif
