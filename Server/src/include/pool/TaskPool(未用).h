#pragma once
#ifndef TASK_POOL_H
#define TASK_POOL_H
#include "../public_macro.h"
#include "SocketPool.h"
#include <queue>

#include <thread>
#include <mutex>
#include <condition_variable>

//任务结构体
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
	//定义函数指针，返回类型为void*,参数类型为void*的函数
	void* (*function)(void*);
#endif // _WIN32


	//上面函数的参数
	void* arg;
};

#ifdef _WIN32
typedef DWORD(*callback)(void*);
#else
//定义函数指针类型，返回类型为void*,参数类型为void*的函数
typedef void* (*callback)(void*);
#endif // _WIN32

class TaskPool
{
public:
	TaskPool();
	~TaskPool();

	//往任务池中添加任务
	void add_task(Task& task);
	void add_task(callback func, void* arg);

	//获取当前任务池中的任务数量
	inline int get_task_num()
	{
		return m_task_queue.size();
	}

	//从队列中取出一个任务
	Task get_one_task();

private:
	//semaphore_t		m_task_sem;	//互斥锁
	//pthread_mutex_t m_task_lock;
	mutex			m_task_mutex;//互斥锁
	queue<Task>		m_task_queue;//任务队列

};

#endif // !TASK_POOL_H