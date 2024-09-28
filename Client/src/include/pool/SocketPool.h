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
typedef sem_t semaphore_t;	//����û��shmem��System V��ʽ�ź�������һ��posix

#endif // _WIN32


//�׽������ӳأ�ͨ�׾��� ��ǰ����n�����ӷ��������׽��֣����ü���
class SocketPool
{
public:
	SocketPool(string ip, unsigned short port, int capacity);
	~SocketPool();
	//�����ӳ����л�ȡһ������
	TcpSocket* get_connect();
	//�����ӷŻ����ӳ���
	int put_connect(TcpSocket* tcp,bool isValid = true);
	//�ж����ӳ��Ƿ�Ϊ��
	inline bool is_empty();

private:
	int create_connect();

private:
	string				m_ip;	//������ip
	int					m_capacity;
	int					m_node_number;
	unsigned short		m_port;
	queue<TcpSocket*>	m_skt_queue;
	semaphore_t			m_skt_sem;
};

int semaphore_init(semaphore_t &sem, int init_val);
int semaphore_wait(semaphore_t &sem,int wait_ms);//wait_ms�ȴ�ʱ�䣬��λ���룬-1һֱ��
int semaphore_post(semaphore_t &sem);
int semaphore_destroy(semaphore_t &sem);

#endif // !SOCKET_POOL_H




