#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h> //atoi()
#include <pthread.h>
void * recv_msg(void *arg);//接收消息函数声明

int main(int argc, char *argv[])
{

	//从命令行获取端口号
	int port = 7777;
	if( port<1025 || port>65535 )//0~1024一般给系统使用，一共可以分配到65535
	{
		printf("端口号范围应为1025~65535");
		return -1;
	}
	
	//1 创建tcp通信socket
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_fd == -1)
	{
		perror("socket failed!\n");
		return -1;
	}
 
	//2 绑定socket地址
	struct sockaddr_in server_addr = {0};
	server_addr.sin_family = AF_INET;//AF_INET->IPv4  
	server_addr.sin_port = htons(port);// server port
	server_addr.sin_addr.s_addr = INADDR_ANY;//server ip (auto set by system)
	int ret = bind(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr) );
	if(ret == -1)
	{
		perror("bind failed!\n");
		return -1;
	}
 
	//3 设置监听队列，设置为可以同时连接5个客户端
	ret = listen(socket_fd, 5);
	if(ret == -1)
	{
		perror("listen falied!\n");
		return -1;
	}
 
 
	printf("server is running!\n");
 
	struct sockaddr_in client_addr = {0};//用来存放客户端的地址信息
	socklen_t len = sizeof(client_addr);
	int new_socket_fd = -1;//存放与客户端的通信socket
	
	//4 等待客户端连接
	new_socket_fd = accept( socket_fd, (struct sockaddr *)&client_addr, &len);
	if(new_socket_fd == -1)
	{
		perror("accpet error!\n");
	}
	else
	{
		printf("IP:%s, PORT:%d [connected]\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
	}
 
	//开启接收线程
	pthread_t recv_thread;//存放线程id       recv_msg：线程执行的函数，将通信socket：new_socket_fd传递进去
	ret = pthread_create(&recv_thread, NULL, recv_msg, (void*)&new_socket_fd);
	if(ret != 0)
	{
		printf("开启线程失败\n");
	}
	
	while(1)
	{
		char buf[1024] = {0};
		sleep(1);
		strcpy(buf,"0.5;-0.5;1.0;1.0;-2;0.5;1;1.0;8;9;10;11;-1;-2;-3;");
		write(new_socket_fd, buf, sizeof(buf));//发送消息
		
		if(strcmp(buf, "exit") == 0 || strcmp(buf, "") == 0)//退出
		{
			ret = pthread_cancel(recv_thread);//取消线程
			
			break;
		}
	}
 
	//5 关闭通信socket
	close(new_socket_fd);
	close(socket_fd);
 
	return 0;
}

void * recv_msg(void *arg)//接收线程所要执行的函数 接收消息
{
	int *socket_fd = (int *)arg;//通信的socket
	while(1)
	{
		char buf[1024] = {0};
		read(*socket_fd, buf, sizeof(buf));//阻塞，等待接收消息
		printf("receive msg:%s\n", buf);
		if(strncmp(buf, "exit", 4) == 0 || strcmp(buf, "") == 0)
		{
			//通知主线程。。。
			
			break;//退出
		}
	}
	return NULL;
}


