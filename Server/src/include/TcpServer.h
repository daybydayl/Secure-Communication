#pragma once
#include "../include/TcpSocket.h"
#include <map>

class TcpServer
{
public:
	TcpServer();
	~TcpServer();

	// 服务器设置监听
	int setListen(unsigned short port);
	// 等待并接受客户端连接请求, 默认连接超时时间为10000s
	TcpSocket* acceptConn(int timeout = 10000);
	void closefd();

public:
	// 存放已连接的终端的通讯套接字和地址的键值对
	map<int, struct sockaddr_in> m_mp_sktToaddr;

private:
	// 用于监听的文件描述符
	int m_lfd;	

};

