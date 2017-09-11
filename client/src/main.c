#include <stdio.h>
#include <unistd.h>
#include "client.h"

int main(void)
{
	//连接服务器
	int clientfd = 0;
	if(-1 == (clientfd = connect_server()))
	{
		printf("连接服务器失败！请检查您的网络...\n");
		return 0;
	}

	return 0;
}
