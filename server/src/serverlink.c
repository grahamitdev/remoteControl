#include "serverlink.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

/*********************************用户链表操作函数********************************/
//从文件读
int loadData(UserLink *userHead)
{
	if(NULL == userHead)
	{
		printf(USERLINK_IS_NULL);
		return 0;
	}
	FILE *fp = fopen("./user.txt","r");//读打开
	if(NULL == fp)
	{
		system("touch ./user.txt");
		printf("user.txt创建完毕！\n");
		return 1;
	}
	User user = {};
	while(fscanf(fp,"name:%s\tpasswd:%s\tregistTime:%s\tlastLogTime%s\n",\
	user.name,\
	user.passwd,\
	user.registTime,\
	user.lastLogTime) > 0)
	{
		insertAfterUserLink(userHead,&user);
	}
	fclose(fp);
	fp = NULL;
	printf("UserLink加载完毕！\n");
	return 1;
}
//向文件写
int saveData(UserLink *userHead)
{
	if(NULL == userHead)
	{
		printf(USERLINK_IS_NULL);
		return 0;
	}
	FILE *fp = fopen("./user.txt","w");//写创打开
	if(NULL == fp)
	{
		printf("user.txt读打开失败！\n");
		return 0;
	}
	userHead = userHead -> next;
	while(NULL != userHead)
	{
		fprintf(fp,"name:%s\tpasswd:%s\tregistTime:%s\tlastLogTime:%s\n",\
		userHead -> data.name,\
		userHead -> data.passwd,\
		userHead -> data.registTime,\
		userHead -> data.lastLogTime);
		userHead = userHead -> next;
	}
	fclose(fp);
	fp = NULL;
	printf("UserLink保存完毕！\n");
	return 1;
}
int freeUserLinkAllNode(UserLink *userHead)
{
	if(NULL == userHead)
	{
		printf(USERLINK_IS_NULL);
		return 0;
	}
	UserLink *pre = NULL;
	while(NULL != userHead)
	{
		pre = userHead -> next;
		free(userHead);
		userHead = pre;
	}
	printf("UserLink释放完毕！\n");
	return 1;
}
int freeUserLinkOneNode(UserLink *userHead,char *name)
{
	if(userHead == NULL)
	{
		perror(USERLINK_IS_NULL);
		return 0;
	}
	UserLink *pre = getUserLinkPreNode(userHead,name);
	if(pre == NULL)
	{
		return 0;
	}
	else
	{
		UserLink *currentNode = pre -> next;
		pre -> next = currentNode -> next;
		free(currentNode);
		currentNode = NULL;
		return 1;
	}	
}
int insertAfterUserLink(UserLink *userHead,User *user)
{
	if(NULL == userHead)
	{
		printf("USERLINK_IS_NULL");
		return 0;
	}	
	UserLink *newNode = createUserLinkNode(user);
	while(NULL != userHead -> next)
	{
		userHead = userHead -> next;
	}
	userHead -> next = newNode;
	return 1;
}
UserLink *createUserLinkNode(User *user)
{
	UserLink *newNode = (UserLink *)calloc(1,sizeof(UserLink));
	newNode -> data = *user;
	newNode -> next = NULL;
	return newNode;
}
UserLink *getUserLinkPreNode(UserLink *userHead,char *name)
{
	if(NULL == userHead)
	{
		printf(USERLINK_IS_NULL);
		return NULL;
	}
	UserLink *pre = userHead;
	userHead = userHead -> next;
	while(NULL != userHead)
	{	
		if(0 == strcmp(userHead -> data.name,name))
		{
			return pre;
		}
		pre = userHead;
		userHead = userHead -> next;
	}
	return NULL;
}


/**************************************在线用户链表操作函数***********************************/

int insertAfterOnlineLink(OnlineLink *onlineHead,Online *online)
{
	if(NULL == onlineHead)
	{
		printf(ONLINELINK_IS_NULL);
		return 0;
	}
	OnlineLink *newNode = createOnlineLinkNode(online);
	while(NULL != onlineHead -> next)
	{
		onlineHead = onlineHead -> next;
	}
	onlineHead -> next = newNode;
	return 1;
}
int freeOnlineLinkAllNode(OnlineLink *onlineHead)
{
	if(NULL == onlineHead)
	{
		printf(ONLINELINK_IS_NULL);
		return 0;
	}
	OnlineLink *pre = NULL;
	while(NULL != onlineHead)
	{
		pre = onlineHead -> next;
		free(onlineHead);
		onlineHead = pre;
	}
	printf("OnlineLink释放完毕！\n");
	return 1;
}
int freeOnlineLinkOneNode(OnlineLink *onlineHead,int clientfd)
{
	if(onlineHead == NULL)
	{
		perror(ONLINELINK_IS_NULL);
		return 0;
	}
	OnlineLink *pre = getOnlineLinkPreNode(onlineHead,clientfd);
	if(pre == NULL)//没找到前节点，说明链表里没有要找的节点
	{
		return 0;
	}
	else//找到前节点
	{
		OnlineLink *currentNode = pre -> next;//目标节点
		pre -> next = currentNode -> next;
		free(currentNode);
		currentNode = NULL;
		return 1;
	}
}
OnlineLink *createOnlineLinkNode(Online *online)
{
	OnlineLink *newNode = (OnlineLink *)calloc(1,sizeof(OnlineLink));
	newNode -> data = *online;
	newNode -> next = NULL;
	return newNode;
}
OnlineLink *getOnlineLinkPreNode(OnlineLink *onlineHead,int clientfd)
{
	if(NULL == onlineHead)
	{
		printf(ONLINELINK_IS_NULL);
		return NULL;
	}
	OnlineLink *pre = onlineHead;
	onlineHead = onlineHead -> next;
	while(NULL != onlineHead)
	{
		if(onlineHead -> data.clientfd == clientfd)
		{
			return pre;
		}
		pre = onlineHead;
		onlineHead = onlineHead -> next;
	}
	return NULL;
}
OnlineLink *getOnlineFriendPreNode(OnlineLink *onlineHead,char *name)
{
	if(NULL == onlineHead)
	{
		printf(ONLINELINK_IS_NULL);
		return NULL;
	}
	OnlineLink *pre = onlineHead;
	onlineHead = onlineHead -> next;
	while(NULL != onlineHead)
	{
		if(strcmp(onlineHead -> data.name,name) == 0)
		{
			return pre;
		}
		pre = onlineHead;
		onlineHead = onlineHead -> next;
	}
	return NULL;
}


/******************************客户端链表*********************************/

int insertAfterClientLink(ClientLink *clientHead,Client *client)
{
	if(NULL == clientHead)
	{	
		printf("insert\n");
		printf(CLIENTLINK_IS_NULL);
		return 0;
	}
	ClientLink *newNode = createClientLinkNode(client);
	while(NULL != clientHead -> next)
	{
		clientHead = clientHead -> next;
	}
	clientHead -> next = newNode;
	return 1;
}
int freeClientLinkAllNode(ClientLink *clientHead)
{
	if(NULL == clientHead)
	{	
		printf("free\n");
		printf(CLIENTLINK_IS_NULL);
		return 0;
	}	
	ClientLink *pre = NULL;
	while(NULL != clientHead)
	{
		pre = clientHead -> next;
		free(clientHead);
		clientHead = pre;
	}	
	printf("ClientLink释放完毕！\n");
	return 1;
}
int freeClientLinkOneNode(ClientLink *clientHead,int clientfd)
{
	if(NULL == clientHead)
	{
		printf("freeone\n");
		printf(CLIENTLINK_IS_NULL);
		return 0;
	}
	ClientLink *pre = getClientLinkPreNode(clientHead,clientfd);
	if(NULL == pre)
	{
		printf("要释放的节点不存在！\n");
		return 0;
	}
	else
	{
		ClientLink *currentNode = pre -> next;
		pre -> next = currentNode -> next;
		free(currentNode);
		currentNode = NULL;
		return 1;
	}
}


ClientLink *createClientLinkNode(Client *client)
{
	ClientLink *newNode = (ClientLink *)calloc(1,sizeof(ClientLink));
	newNode -> data = *client;
	newNode -> next = NULL;
	return newNode;
}
ClientLink *getClientLinkPreNode(ClientLink *clientHead,int clientfd)
{
	if(NULL == clientHead)
	{
		printf("get\n");
		printf(CLIENTLINK_IS_NULL);
		return NULL;
	}
	ClientLink *pre = clientHead;
	clientHead = clientHead -> next;
	while(NULL != clientHead)
	{
		if(clientHead -> data.clientfd == clientfd)
		{
			return pre;
		}
		pre = clientHead;
		clientHead = clientHead -> next;
	}
	return NULL;
}

