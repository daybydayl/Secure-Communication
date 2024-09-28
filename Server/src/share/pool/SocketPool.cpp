#include "../../include/pool/SocketPool.h"

SocketPool::SocketPool(string ip, unsigned short port, int capacity)
	:m_ip(ip),
	m_port(port),
	m_capacity(capacity),
	m_node_number(capacity)
{
	semaphore_init(m_skt_sem, 1);//值为1的mutex
	//创建连接池中的连接
	create_connect();

}

SocketPool::~SocketPool()
{
	// 销毁互斥锁
	semaphore_destroy(m_skt_sem);
	// 关闭连接, 释放套接字对象
	while (m_skt_queue.size())
	{
		TcpSocket* t = m_skt_queue.front();
		t->disConnect();
		delete t;
		m_skt_queue.pop();
	}
}

TcpSocket* SocketPool::get_connect()
{
	if (m_skt_queue.empty())
	{
		return NULL;
	}

	semaphore_wait(m_skt_sem,-1);
	//取出对头连接
	TcpSocket* tcp = m_skt_queue.front();
	//弹出取出的连接
	m_skt_queue.pop();
	printf("connect size: %d\n", m_skt_queue.size());
	semaphore_post(m_skt_sem);

	return tcp;
}

int SocketPool::put_connect(TcpSocket* tcp, bool isValid)
{
	semaphore_wait(m_skt_sem,-1);
	//判断连接是否可用
	if (isValid)
	{
		//放回连接池队尾中
		m_skt_queue.push(tcp);
		printf("放回队尾可用连接,当前size: %d\n", m_skt_queue.size());
	}
	else
	{
		printf("连接不可用,当前size: %d\n",m_skt_queue.size());
		//关闭坏掉的连接，并释放对象
		tcp->disConnect();
		delete tcp;
		//创建新连接
		m_node_number = m_skt_queue.size() + 1;
		create_connect();
	}
	semaphore_post(m_skt_sem);
	return 0;
}

inline bool SocketPool::is_empty()
{
	return m_skt_queue.empty() == true ? true : false;
}

int SocketPool::create_connect()
{
	int retcode;
	printf("current list size: %d\n", m_skt_queue.size());
	printf("nodenumber: %d\n", m_node_number);
	if (m_skt_queue.size() >= m_node_number)
	{
		retcode = -1;
		return retcode;
	}

	// 创建socket通信对象
	TcpSocket* tcp = new TcpSocket;
	// 连接服务器
	retcode = tcp->connectToHost(m_ip, m_port);
	if (retcode == 0)
	{
		m_skt_queue.push(tcp);
		printf("create new tcp,cur size: %d\n", m_skt_queue.size());
	}
	else
	{
		delete tcp;
	}

	// 递归创建连接
	create_connect();
}

int semaphore_init(semaphore_t &sem, int init_val)
{
	int retcode = -1;
#ifdef _WIN32
	//获取命名信号量
	sem = CreateSemaphore(
		NULL,		// 默认安全描述符
		init_val,	// 初始计数器值
		LONG_MAX,	// 最大计数器值，这里给个大值
		NULL);		// 信号量名称
	if (sem == NULL)
	{
		retcode = -1;	//创建失败
	}
	else
	{
		retcode = 0;	//创建成功
	}
#else
	//0是进程内共享
	retcode = sem_init(&sem, 0, init_val);
#endif // _WIN32

	return retcode;
}

int semaphore_wait(semaphore_t &sem, int wait_ms)
{
	int retcode = -1;
#ifdef _WIN32
	//INFINITE:指无线等待
	if (wait_ms == -1)
	{
		retcode = WaitForSingleObject(sem, INFINITE);
	}
	else
	{
		retcode = WaitForSingleObject(sem, wait_ms);
	}
	if (retcode == WAIT_OBJECT_0)
	{
		// 成功等待到信号量，返回0表示成功
		return 0;
	}
	else
	{
		// 等待失败，释放信号量（此处释放信号量的操作通常不需要）,这里只是等待
		//ReleaseSemaphore(sem, 1, NULL);
		return -1;
	}
#else
	if (wait_ms == -1)
	{
		retcode = sem_wait(&sem);
	}
	else
	{
		struct timespec ts;
		ts.tv_sec = wait_ms / 1000;
		ts.tv_nsec = (wait_ms % 1000) * 1000000;

		retcode = sem_timedwait(&sem, &ts);
	}
	if (retcode != 0)
	{
		printf("sem_wait errno : %d\n", errno);
	}
#endif // _WIN32

	return retcode;
}

int semaphore_post(semaphore_t &sem)
{
	int retcode = -1;

#ifdef _WIN32
	retcode = ReleaseSemaphore(sem, 1, NULL);
#else
	retcode = sem_post(&sem);
#endif // _WIN32

	return retcode;
}

int semaphore_destroy(semaphore_t &sem)
{
	int retcode = -1;
#ifdef _WIN32
	retcode = CloseHandle(sem);
#else
	retcode = sem_destroy(&sem);
#endif // _WIN32

	return retcode;
}
