#ifndef _SERVERLINK_H_
#define _SERVERLINK_H_
#include <pthread.h>

#define USERLINK_IS_NULL "用户链表头节点为空！"
#define ONLINELINK_IS_NULL "在线用户链表头节点为空！"
#define CLIENTLINK_IS_NULL "客户端联链表头节点为空！"
//用户结构体
typedef struct user{
	char name[20];
	char passwd[20];
	char registTime[50];
	char lastLogTime[50];
}User;

//用户链表结构体
typedef struct userLink{
	User data;
	struct userLink *next;
}UserLink;

//在线客户端
typedef struct client{
	int clientfd;//服务器分的clientfd
	int count;
	time_t walter;//连接成功第一时间
	pthread_t pthread;//服务器分pthreadid
	char IP[16];//客户端IP
}Client;

typedef struct clientLink{
	Client data;
	struct clientLink *next;
}ClientLink;

//在线用户结构体
typedef struct online{
	char name[20];
	int  on;	//是否打开聊天界面
	int  clientfd;//服务器分的clientfd
}Online;

//在线用户链表结构体
typedef struct onlineLink{
	Online data;
	struct onlineLink *next;
}OnlineLink;

/*******************用户链表操作函数************************/
int loadData(UserLink *userHead);
int saveData(UserLink *userHead);
int freeUserLinkAllNode(UserLink *userHead);
int freeUserLinkOneNode(UserLink *userHead,char *name);
int insertAfterUserLink(UserLink *userHead,User *user);
UserLink *createUserLinkNode(User *user);
UserLink *getUserLinkPreNode(UserLink *userHead,char *name);

/*******************在线用户链表操作函数*********************/
int insertAfterOnlineLink(OnlineLink *onlineHead,Online *online);
int freeOnlineLinkAllNode(OnlineLink *onlineHead);
int freeOnlineLinkOneNode(OnlineLink *onlineHead,int clientfd);
OnlineLink *createOnlineLinkNode(Online *online);
OnlineLink *getOnlineLinkPreNode(OnlineLink *onlineHead,int clientfd);
OnlineLink *getOnlineFriendPreNode(OnlineLink *onlineHead,char *name);

/*******************客户端链表操作函数***********************/
int insertAfterClientLink(ClientLink *clientHead,Client *client);
int freeClientLinkAllNode(ClientLink *clientHead);
int freeClientLinkOneNode(ClientLink *clientHead,int clientfd);
ClientLink *createClientLinkNode(Client *client);
ClientLink *getClientLinkPreNode(ClientLink *clientHead,int clientfd);

#endif
