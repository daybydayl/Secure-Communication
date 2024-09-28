#pragma once
#ifndef SOCKET_POOL_H
#define SOCKET_POOL_H
#include "../public_macro.h"
#include "../TcpSocket.h"
#include <iostream>
#include <queue>

#if _WIN32
typedef HANDLE semaphore_t;
#else
typedef sem_t semaphore_t;	//这里没用shmem的System V方式信号量，换一种posix

#endif // _WIN32


//套接字连接池，通俗就是 提前创建n个连接服务器的套接字，即拿即用
class SocketPool
{
public:
	SocketPool(string ip, unsigned short port, int capacity);
	~SocketPool();
	//从连接池子中获取一条连接
	TcpSocket* get_connect();
	//将连接放回连接池中
	int put_connect(TcpSocket* tcp,bool isValid = true);
	//判断连接池是否为空
	inline bool is_empty();

private:
	int create_connect();

private:
	string				m_ip;	//服务器ip
	int					m_capacity;
	int					m_node_number;
	unsigned short		m_port;
	queue<TcpSocket*>	m_skt_queue;
	semaphore_t			m_skt_sem;
};

int semaphore_init(semaphore_t &sem, int init_val);
int semaphore_wait(semaphore_t &sem,int wait_ms);//wait_ms等待时间，单位毫秒，-1一直等
int semaphore_post(semaphore_t &sem);
int semaphore_destroy(semaphore_t &sem);

#endif // !SOCKET_POOL_H




