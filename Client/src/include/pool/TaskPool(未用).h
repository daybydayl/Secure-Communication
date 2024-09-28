#pragma once
#ifndef TASK_POOL_H
#define TASK_POOL_H
#include "../public_macro.h"
#include "SocketPool.h"
#include <queue>

#include <thread>
#include <mutex>
#include <condition_variable>

//����ṹ��
struct Task
{
	Task()
	{
		function = NULL;
		arg = NULL;
	}
#ifdef _WIN32
	DWORD(*function)(void*);
#else
	//���庯��ָ�룬��������Ϊvoid*,��������Ϊvoid*�ĺ���
	void* (*function)(void*);
#endif // _WIN32


	//���溯���Ĳ���
	void* arg;
};

#ifdef _WIN32
typedef DWORD(*callback)(void*);
#else
//���庯��ָ�����ͣ���������Ϊvoid*,��������Ϊvoid*�ĺ���
typedef void* (*callback)(void*);
#endif // _WIN32

class TaskPool
{
public:
	TaskPool();
	~TaskPool();

	//����������������
	void add_task(Task& task);
	void add_task(callback func, void* arg);

	//��ȡ��ǰ������е���������
	inline int get_task_num()
	{
		return m_task_queue.size();
	}

	//�Ӷ�����ȡ��һ������
	Task get_one_task();

private:
	//semaphore_t		m_task_sem;	//������
	//pthread_mutex_t m_task_lock;
	mutex			m_task_mutex;//������
	queue<Task>		m_task_queue;//�������

};

#endif // !TASK_POOL_H