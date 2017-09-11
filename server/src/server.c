#include "server.h"
#include "servercontrol.h"
#include "serverlink.h"
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
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>



int serverfd = 0;
char IP[16] = "";
unsigned short int PORT = 0;
pthread_t pthread;




UserLink 	*userHead = NULL;
OnlineLink	*onlineHead = NULL;
ClientLink	*clientHead = NULL;





/***********************************************admin登录菜单函数**************************************************/
void admin_login_system(void)
{
	char localTime[50] = "";
	get_local_time(localTime);
	if(0 == login())
	{
		printf_log(1,0,"管理员登录成功！");
		sleep(1);
		server_main_menu_control();
	}
	else
	{
		printf_log(3,0,"管理员登录失败5秒内不允许登录！");
		sleep(5);
	}
}
int login(void)
{
	char name[1024] = "";
	char passwd[1024] = "";
	//char passwd_c[1024] = "";
	printf("请输入账户名和密码:");
	scanf("%s %s",name,passwd);
	if((0 != strcmp(name,"admin") ||(0 != strcmp(passwd,"admin"))))
	{
		printf("账户名或密码错误请重新输入,您还有2次输入机会！\n");
		printf("请输入账户名和密码:");
		scanf("%s %s",name,passwd);
		if((0 != strcmp(name,"admin") ||(0 != strcmp(passwd,"admin"))))
		{
			printf("账户名或密码错误请重新输入,您还有1次输入机会！\n");
			printf("请输入账户名和密码:");
			scanf("%s %s",name,passwd);
			if((0 != strcmp(name,"admin") ||(0 != strcmp(passwd,"admin"))))
			{
				printf("账户名或密码错误,本次禁止输入");
				return -1;		
			}		
		}
	}
	return 0;	
}
/*******************服务器主菜单函数*****************************/

void sig_handler_c(int signo)
{
	close(serverfd);
	printf("套接字关闭成功！\n");
	pthread_cancel(pthread);
	printf("子线程关闭成功！\n");
	printf("服务器关闭完成！\n");
	exit(-1);	
}
void sig_handler_z(int signo)
{
//	close(serverfd);
//	printf("套接字关闭\n");
	pthread_cancel(pthread);
	printf("子线程关闭\n");
}
void sig_handler_fg(int signo)
{
	start_server();
}
void start_server(void)
{
	if(signal(SIGINT,sig_handler_c) == SIG_ERR)
	{
		perror("signal sigint error");
		return;
	}
	if(signal(SIGTSTP,sig_handler_z) == SIG_ERR)
	{
		perror("signal sigtstp error");
		return;
	}
	if(signal(SIGCONT,sig_handler_fg) == SIG_ERR)
	{
		perror("signal sigcont error");
		return;
	}
	userHead = (UserLink *)calloc(1,sizeof(UserLink));
	if(userHead == NULL)
	{
		perror("申请userHead失败！");
		exit(-1);
	}
	if(0 == loadData(userHead))
	{
		perror("加载UserLink失败！");
		exit(-1);
	}
	onlineHead = (OnlineLink *)calloc(1,sizeof(OnlineLink));
	if(onlineHead == NULL)
	{
		perror("申请onlineHead失败！");
		exit(-1);
	}
	clientHead = (ClientLink *)calloc(1,sizeof(ClientLink));
	if(clientHead == NULL)
	{
		perror("申请clientHead失败！");
		exit(-1);
	}	
	///////////////////////////////////////////////////以下知道服务端serverfd全局//////////////////////////
	serverfd = socket(AF_INET,SOCK_STREAM,0);
	if(-1 == serverfd)
	{
		perror("serverfd error");
		exit(-1);
	}
	struct sockaddr_in server_addr = {};
	server_addr.sin_family = AF_INET;
	char port_array[1024] = "";
	if(NULL == resolution(port_array,"./config.txt","PORT"))
	{
		perror("resolution port error");
		exit(-1);
	}
	PORT = atoi(port_array);
	server_addr.sin_port = htons(PORT);
	if(NULL == get_system_ipv4(IP))
	{
		perror("resolution IP error");
		exit(-1);
	}
	if(-1 == inet_pton(AF_INET,IP,&server_addr.sin_addr.s_addr))
	{
		perror("inet_pton IP error");
		exit(-1);
	}
	if(-1 == bind(serverfd,(struct sockaddr *)&server_addr,sizeof(server_addr)))
	{
		printf("IP:%s\n",IP);
		printf("PORT:%d\n",PORT);
		perror("bind error");
		exit(-1);
	}
	if(-1 == listen(serverfd,NUMMAX))
	{
		perror("listen error");
		exit(-1);
	}
	printf_log(1,serverfd,"服务器启动成功！");
	
	//开始监听客户端心跳包
	
	printf_log(2,serverfd,"开始监听客户端心跳包");
	if(signal(SIGALRM,sig_handler) == SIG_ERR)
	{
		perror("signal sigalarm error");
		exit(-1);
	}
	alarm(1);	
	//accept
	while(1)
	{
		struct sockaddr_in client_addr = {};
		socklen_t client_len = sizeof(struct sockaddr);
		int clientfd =accept(serverfd,(struct sockaddr *)&client_addr,&client_len);
		if(-1 == clientfd)
		{
			perror("accept clientfd error");
			exit(-1);
		}
		char ip[1024] = "";
		if(NULL == inet_ntop(AF_INET,&client_addr.sin_addr.s_addr,ip,1024))
		{
			perror("inet_ntop client IP error");
			exit(-1);
		}

		unsigned short int PORT = 0;
		PORT = ntohs(client_addr.sin_port);
		char log[100] = "";
		sprintf(log,"%s连上服务器,开始创建点对点服务线程",ip);
		printf_log(1,serverfd,log);	
		Conf *config = (Conf *)malloc(sizeof(Conf));
		//config在栈config里是堆的地址*config是结构体值
		//(void *)config把结构体地址转成(void *)地址
		memset(config,0,sizeof(Conf));
		config -> clientfd = clientfd;
		strcpy(config -> IP,ip);//客户端的ip////////////////
		config -> PORT = PORT;
		void *p = (void *)config;
		config = NULL;

		//点对点
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
		int err = 0;
		if(0 != (err = pthread_create(&pthread,&attr,th_fn,p)))
		{
			
			sprintf(log,"服务器为%s开启器点对点服务线程失败",ip);
			printf_log(3,serverfd,log);
			exit(-1);
		}
		sprintf(log,"服务器为%s开辟点对点线程成功",ip);
		printf_log(1,serverfd,log);
	}
	return;
}
void sig_handler(int signo)
{
	if(signo == SIGALRM)
	{
		if(clientHead -> next == NULL)
		{
			printf_log(2,serverfd,"目前还没有连接的客户端");
		}
		
		else
		{	
			ClientLink *cursor = clientHead;
			while(cursor -> next != NULL)//遍历client链表
			{
				cursor = cursor -> next;
				if(cursor -> data.count >= 3)
				{
					//printf("count = %d\n",cursor -> data.count);
					cursor -> data.count = 0;
				}
				else
				{
					time_t now;
					time(&now);
					char log[100] = "";
					if(now - cursor -> data.walter <= 5)
					{						
						sprintf(log,"客户端%s刚刚连接上服务器",cursor -> data.IP);
						printf_log(2,serverfd,log);
					}
					else
					{
						sprintf(log,"5秒内收到客户端%s数据包小于3个,判定客户端异常关闭套接字，关闭它的服务线程!",cursor -> data.IP);
						printf_log(3,serverfd,log);
						pthread_cancel(cursor -> data.pthread);
					}
				}
			}
		}	
		alarm(5);
	}	
}
void *th_fn_send_heart(void *arg)
{
	int clientfd = (int)arg;//服务器为客户端分配的clientfd
	Pack package_snd_heart = {};
	while(1)
	{
		package_snd_heart.type = 0;
		if(write(clientfd,&package_snd_heart,sizeof(Pack)) <= 0)
		{
			perror("send heart to client error");
			break;
		}
		//printf_log(2,clientfd,"向客户端发送心跳包");
		//由于频率太高所以不用堆，但是log的信息就不包括客户端的IP
		//可以用服务器给客户端分配的clientfd区分
		sleep(1);
	}
	return NULL;
}
void *th_fn(void *arg)
{
	Conf *p_config = (Conf *)arg;
	Conf config = *p_config;
	free(p_config);
	p_config = NULL;

	int clientfd = config.clientfd;        
	char ip[16] = "";
	strcpy(ip,config.IP);

	if(userHead == NULL)
	{
		perror(USERLINK_IS_NULL);
		return NULL;
	}
	if(onlineHead == NULL)
	{
		perror(ONLINELINK_IS_NULL);
		return NULL;
	}
	if(clientHead == NULL)
	{
		perror(CLIENTLINK_IS_NULL);
		return NULL;
	}
	//向在线客户端链表插一个记录
	time_t now;
	time(&now);
	Client client = {};
	client.clientfd = clientfd;
	client.count = 0;
	client.walter = now;
	client.pthread = pthread;
	strcpy(client.IP,ip);
	insertAfterClientLink(clientHead,&client);
	ClientLink *currentNode = getClientLinkPreNode(clientHead,clientfd) -> next;

	//向在线客户端发心跳
	pthread_t pthread_heart;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	int err = 0;
	if(0 != (err = pthread_create(&pthread_heart,&attr,th_fn_send_heart,(void *)clientfd)))
	{
		perror("heart pthread error");
		exit(-1);
	}
	Pack package_rcv = {};
	Pack package_rcv_login = {};
	Pack package_rcv_regist = {};
	Pack package_rcv_shell= {};
	Pack package_rcv_chat = {};
	Pack package_rcv_discuss = {};
	Pack package_rcv_logoff = {};
	Pack package_rcv_disconn = {};
	Pack package_rcv_heart = {};
	//收客户端数据包
	char log[100] = "";
	while(1)
	{
		if(read(clientfd,&package_rcv,sizeof(Pack)) <= 0)
		{
			sprintf(log,"客户端%s套接被关闭",ip);
			printf_log(3,clientfd,log);
			break;
		}
		++(currentNode -> data.count);
	

		switch(package_rcv.type)//解析client包头
		{			
			case LOGIN:
				package_rcv_login = package_rcv;
				package_rcv_login.clientfd = clientfd;
				//填入服务器端分配的clientfd
				create_pthread(package_rcv_login);
				break;
			case REGIST:
				package_rcv_regist = package_rcv;
				package_rcv_regist.clientfd = clientfd;
				create_pthread(package_rcv_regist);
				break;
			case SHELL:
				package_rcv_shell = package_rcv;
				package_rcv_shell.clientfd = clientfd;
				create_pthread(package_rcv_shell);
				break;
			case CHAT:
				package_rcv_chat = package_rcv;
				package_rcv_chat.clientfd = clientfd;
				create_pthread(package_rcv_chat);
				break;
			case DISCUSS:
				package_rcv_discuss = package_rcv;
				package_rcv_discuss.clientfd = clientfd;
				create_pthread(package_rcv_discuss);				
				break;
			case LOGOFF:
				package_rcv_logoff = package_rcv;
				package_rcv_logoff.clientfd = clientfd;
				create_pthread(package_rcv_logoff);
				break;
			case DISCONN:
				package_rcv_disconn = package_rcv;
				package_rcv_disconn.clientfd = clientfd;
				create_pthread(package_rcv_disconn);
				break;
			case HEART:
				package_rcv_heart = package_rcv;
				//package_rcv_heart.clientfd = clientfd;
				//sprintf(log,"收到客户端%s心跳包",package_rcv_heart.ip);
				//printf_log(2,clientfd,log);
				create_pthread(package_rcv_heart);
				break;
			default:
				printf_log(3,clientfd,"不识别客户端的包");
				break;
		}
	}
	return NULL;
}

void create_pthread(Pack package)
{
	int err = 0;
	pthread_t pthread;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	if(package.type == LOGIN)
	{
		Pack *p_pack = (Pack *)malloc(sizeof(Pack));
		memset(p_pack,0,sizeof(Pack));
		*p_pack = package;
		void *p = (void *)p_pack;
		p_pack = NULL;
		if((err = pthread_create(&pthread,&attr,th_fn_login,p)) != 0)
		{
			perror("pthread error");
			return;
		}
	}
	else if(package.type == REGIST)
	{
		Pack *p_pack = (Pack *)malloc(sizeof(Pack));
		memset(p_pack,0,sizeof(Pack));
		*p_pack = package;
		void *p = (void *)p_pack;
		p_pack = NULL;
		if((err = pthread_create(&pthread,&attr,th_fn_regist,p)) != 0)
		{
			perror("pthread error");
			return;
		}
		
	}
	else if(package.type == SHELL)
	{
		Pack *p_pack = (Pack *)malloc(sizeof(Pack));
		memset(p_pack,0,sizeof(Pack));
		*p_pack = package;
		void *p = (void *)p_pack;
		p_pack = NULL;
		if((err = pthread_create(&pthread,&attr,th_fn_shell,p)) != 0)
		{
			perror("pthread error");
			return;
		}

	}
	else if(package.type == CHAT)
	{
		Pack *p_pack = (Pack *)malloc(sizeof(Pack));
		memset(p_pack,0,sizeof(Pack));
		*p_pack = package;
		void *p = (void *)p_pack;
		p_pack = NULL;
		if((err = pthread_create(&pthread,&attr,th_fn_chat,p)) != 0)
		{
			perror("pthread error");
			return;
		}

	}
	else if(package.type == DISCUSS)
	{
		Pack *p_pack = (Pack *)malloc(sizeof(Pack));
		memset(p_pack,0,sizeof(Pack));
		*p_pack = package;
		void *p = (void *)p_pack;
		p_pack = NULL;
		if((err = pthread_create(&pthread,&attr,th_fn_discuss,p)) != 0)
		{
			perror("pthread error");
			return;
		}

	}
	else if(package.type == LOGOFF)
	{
		Pack *p_pack = (Pack *)malloc(sizeof(Pack));
		memset(p_pack,0,sizeof(Pack));
		*p_pack = package;
		void *p = (void *)p_pack;
		p_pack = NULL;
		if((err = pthread_create(&pthread,&attr,th_fn_logoff,p)) != 0)
		{
			perror("pthread error");
			return;
		}

	}
	else if(package.type == DISCONN)
	{
		Pack *p_pack = (Pack *)malloc(sizeof(Pack));
		memset(p_pack,0,sizeof(Pack));
		*p_pack = package;
		void *p = (void *)p_pack;
		p_pack = NULL;
		if((err = pthread_create(&pthread,&attr,th_fn_disconn,p)) != 0)
		{
			perror("pthread error");
			return;
		}

	}
	else if(package.type == HEART)
	{
		Pack *p_pack = (Pack *)malloc(sizeof(Pack));
		memset(p_pack,0,sizeof(Pack));
		*p_pack = package;
		void *p = (void *)p_pack;
		p_pack = NULL;
		if((err = pthread_create(&pthread,&attr,th_fn_heart,p)) != 0)
		{
			perror("pthread error");
			return;
		}

	}
	else
	{
		perror("create pthread error");
	}
}
///////////////////////////////////////////////以下所有信息都知道//////////////////////////////////
void *th_fn_login(void *arg)
{
	Pack *p_pack = (Pack *)arg;
	Pack package = *p_pack;
	free(p_pack);
	p_pack = NULL;
	int clientfd = package.clientfd;//服务器给自己分配的clientfd

	UserLink *pre = getUserLinkPreNode(userHead,package.name);
	if(pre != NULL)
	{
		if(getOnlineFriendPreNode(onlineHead,package.name) == NULL)
		{
			if(strcmp(package.passwd,pre -> next -> data.passwd) != 0)
			{
				strcpy(package.text,"密码错误");
				if(write(clientfd,&package,sizeof(Pack)) <= 0)
				{
					perror("back write error");
					return NULL;
				}
				printf_log(3,clientfd,"密码错误");
				return NULL;
			}
			Online online = {};
			strcpy(online.name,package.name);
			online.on = 0;//临时
			online.clientfd = clientfd;
			insertAfterOnlineLink(onlineHead,&online);
			//把登录的用户插入到在线链表
			//但是on属性默认为0
			//表示用户没有进入聊天界面
			strcpy(package.text,"登录成功");
			if(write(clientfd,&package,sizeof(Pack)) <= 0)
			{
				perror("back write error");
				return NULL;
			}
			printf_log(1,clientfd,"登录成功");
			OnlineLink *cursor = onlineHead;
			Pack packagenew = {CHAT};
			strcpy(packagenew.text,"login");
			strcpy(packagenew.onlineName[0],package.name);
			while(cursor -> next != NULL)
			{
				cursor = cursor -> next;
				if(strcmp(cursor -> data.name,package.name) == 0)
				{
					continue;
				}
				if(write(cursor -> data.clientfd,&packagenew,sizeof(Pack)) <= 0)
				{
					perror("error");
					return NULL;
				}
				printf("分发成功！\n");
			}
			return NULL;
		}
		else
		{
			strcpy(package.text,"已在线");
			if(write(clientfd,&package,sizeof(Pack)) <= 0)
			{
				perror("back write error");
				return NULL;
			}
			printf_log(3,clientfd,"已在线");
			return NULL;
		}
	}
	else
	{
		strcpy(package.text,"用户不存在");
		if(write(clientfd,&
		package,sizeof(Pack)) <= 0)
		{
			perror("back write error");
			return NULL;
		}
		printf_log(3,clientfd,"用户不存在");
		return NULL;
	}
}
void *th_fn_regist(void *arg)
{
	Pack *p_pack = (Pack *)arg;
	Pack package = *p_pack;
	free(p_pack);
	p_pack = NULL;
	int clientfd = package.clientfd;
	UserLink *pre = getUserLinkPreNode(userHead,package.name);
	if(pre != NULL)//已经注册
	{
		strcpy(package.text,"已注册");
		if(write(clientfd,&package,sizeof(Pack)) <= 0)
		{
			perror("back write error");
			return NULL;
		}
		printf_log(3,clientfd,"已注册拒绝再次注册");
	}
	else//没有注册
	{
		//User user = {package.name,package.passwd,NULL,NULL};
		User user = {};
		strcpy(user.name,package.name);
		strcpy(user.passwd,package.passwd);
		insertAfterUserLink(userHead,&user);
		saveData(userHead);
		strcpy(package.text,"注册成功");
		if(write(clientfd,&package,sizeof(Pack)) <= 0)
		{
			perror("back write error");
			return NULL;
		}
		printf_log(1,clientfd,"注册成功");
	}
	return NULL;
}
void *th_fn_shell(void *arg)
{
	Pack *p_pack = (Pack *)arg;
	Pack package = *p_pack;
	free(p_pack);
	p_pack = NULL;
	int clientfd = package.clientfd;

	char shell[20] = "";
	strcpy(shell,package.text);

	int fd = open("shell.txt",O_RDWR|O_CREAT|O_TRUNC,0664);
	int fd_save = dup(1);
	dup2(fd,1);
	system(shell);
	dup2(fd_save,1);
	close(fd);
	printf("shell命令执行完毕！\n");

	FILE *fp = fopen("shell.txt","rt");
	while(1)
	{
		if(fread(package.text,sizeof(char),1024,fp) <= 0)
		{
			break;
		}
		if(write(clientfd,&package,sizeof(Pack)) <= 0)
		{
			perror("error");
			return NULL;
		}
		usleep(10000);
	}
	fclose(fp);
	fp = NULL;
	strcpy(package.text,"end");
	if(write(clientfd,&package,sizeof(Pack)) <= 0)
	{
		perror("error");
		return NULL;
	}
	return NULL;
}
void *th_fn_chat(void *arg)
{
	Pack *p_pack = (Pack *)arg;
	Pack package = *p_pack;
	free(p_pack);
	p_pack = NULL;
	int clientfd = package.clientfd;
	if(strcmp(package.text,"") == 0)
	{
		int i = 0;
		OnlineLink *cursor = onlineHead;
		while(cursor -> next != NULL)
		{
			cursor = cursor -> next;
			if(cursor -> data.clientfd == clientfd)
			{
				continue;
			}
			strcpy(package.onlineName[i],cursor -> data.name);
			++i;
		}
		getOnlineLinkPreNode(onlineHead,package.clientfd) -> next -> data.on = 1;
		if(i == 0)
		{
			strcpy(package.text,"好友现在都不在线");
		
			if(write(clientfd,&package,sizeof(Pack)) <= 0)
			{
				perror("send online names error");
				return NULL;
			}
			printf("好友都不在线！\n");
		}
		if(write(clientfd,&package,sizeof(Pack)) <= 0)
		{
			perror("send online names error");
			return NULL;
		}
	}
	else
	{
		if(getOnlineFriendPreNode(onlineHead,package.otherName) == NULL)
		{
			strcpy(package.text,"您找的人不在线");
			
			if(write(clientfd,&package,sizeof(Pack)) <= 0)
			{
				perror("send no online name error");
				return NULL;
			}
			printf("找的人不在线！\n");
			return NULL;
		}
		
		OnlineLink *current = getOnlineFriendPreNode(onlineHead,package.otherName) -> next;
		Online online = current -> data;
		if(online.clientfd == clientfd)
		{
			strcpy(package.text,"不允许给自己发送消息");
			if(write(online.clientfd,&package,sizeof(Pack)) <= 0)
			{
				perror("error");
				return NULL;
			}
		}
		OnlineLink *send = getOnlineLinkPreNode(onlineHead,clientfd) -> next;
		strcpy(package.name,send -> data.name);
		if(strcmp(package.text,"bye") == 0)
		{
			//忙碌
			send ->data.on = 0;
			strcat(package.text," 对方已转为忙碌状态！");
			if(write(online.clientfd,&package,sizeof(Pack)) <= 0)
			{
				perror("error");
				return NULL;
			}
			printf("转为忙碌状态成功！\n");
			return NULL;
		}
		if(online.on == 1)
		{
			if(write(online.clientfd,&package,sizeof(Pack)) <= 0)
			{
				perror("back write error");
				return NULL;
			}
			strcpy(package.text,"传递消息成功");
			if(write(clientfd,&package,sizeof(Pack)) <= 0)
			{
				perror("error");
				return NULL;
			}
			printf_log(1,clientfd,"传递消息成功");
		}
		else
		{
			Pack packageother = package;
			strcpy(package.text,"对方在线正忙");
			if(write(clientfd,&package,sizeof(package)) <= 0)
			{
				perror("back write error");
				return NULL;
			}
			if(write(online.clientfd,&packageother,sizeof(Pack)) <= 0)
			{
				perror("send busy error");
				return NULL;
			}
			printf_log(3,clientfd,"信息已传达，对方正忙");
		}
	}


	return NULL;
}
void *th_fn_discuss(void *arg)
{
	Pack *p_pack = (Pack *)arg;
	Pack package = *p_pack;
	free(p_pack);
	p_pack = NULL;
	int clientfd = package.clientfd;

	
	OnlineLink *cursor = onlineHead;
	if(cursor -> next ->next  == NULL)
	{
		strcpy(package.text,"群里没有好友在线");
		if(write(clientfd,&package,sizeof(Pack)) <= 0)
		{
			perror("error");
			return NULL;
		}
		printf("群里没有好友在线！\n");
		return NULL;
	}
	Pack packagenew = {DISCUSS};
	strcpy(packagenew.name,getOnlineLinkPreNode(onlineHead,clientfd) -> next -> data.name);
	strcpy(packagenew.text,package.text);
	while(cursor -> next != NULL)
	{
		cursor = cursor -> next;
		if(cursor -> data.clientfd == clientfd)
		{
			continue;
		}
		if(write(cursor -> data.clientfd,&packagenew,sizeof(Pack)) <= 0)
		{
			perror("error");
			return NULL;
		}
		printf("消息分发成功！\n");
	}
	strcpy(package.text,"群发成功");
	if(write(clientfd,&package,sizeof(Pack)) <= 0)
	{
		perror("error");
		return NULL;
	}
	
	return NULL;
}
void *th_fn_logoff(void *arg)
{
	Pack *p_pack = (Pack *)arg;
	Pack package = *p_pack;
	free(p_pack);
	p_pack = NULL;
	int clientfd = package.clientfd;

	freeOnlineLinkOneNode(onlineHead,clientfd);
	printf("节点释放成功！\n");
	strcpy(package.text,"允许下线");
	if(write(clientfd,&package,sizeof(Pack)) <= 0)
	{
		perror("error");
		return NULL;
	}
	printf("clientfd:%d下线成功\n",clientfd);
	return NULL;
}
void *th_fn_disconn(void *arg)
{
	Pack *p_pack = (Pack *)arg;
	Pack package = *p_pack;
	free(p_pack);
	p_pack = NULL;
	int clientfd = package.clientfd;

	freeClientLinkOneNode(clientHead,clientfd);
	printf("节点释放成功！\n");
	strcpy(package.text,"允许关闭客户端");
	if(write(clientfd,&package,sizeof(Pack)) <= 0)
	{
		perror("error");
		return NULL;
	}
	close(clientfd);
	printf("关闭套接字成功!\n");
	printf("client:%s断开socket成功\n",package.ip);
	return NULL;
}
void *th_fn_heart(void *arg)
{
	return NULL;
}





void server_status(void);
void reboot_server(void);
void shutdown_server(void)
{
	printf("yes\n");
	close(serverfd);
	printf("套接字关闭成功！\n");
	pthread_cancel(pthread);
	printf("子线程关闭成功！\n");
	printf("服务器关闭完成！\n");
	exit(-1);
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
		ip[i] = '\0';
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

//自定义获取当前时间函数
void get_local_time(char *strtime)
{
	time_t now;
	struct tm *tm_now;
	time(&now);
	tm_now = localtime(&now);
	sprintf(strtime,"[%d-%02d-%02d %02d:%02d:%02d]",\
	tm_now -> tm_year + 1900,\
	tm_now -> tm_mon + 1,\
	tm_now -> tm_mday,\
	tm_now -> tm_hour,\
	tm_now -> tm_min,\
	tm_now -> tm_sec);
}
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
