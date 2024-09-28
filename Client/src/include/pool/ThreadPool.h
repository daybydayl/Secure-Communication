#pragma once
#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <iostream>
#include "../public_macro.h"

/*
该线程池用的C++11跨平台自带的，任务用队列存放
前面信号量用到system V和posix版本，这里使用C++11自带的跨平台版本
*/
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

	int (*function)(void*);
	//上面函数的传参
	void* arg;
};

class ThreadPool
{
public:
	ThreadPool(int min_thr_num, int max_thr_num);
	~ThreadPool();

	//给线程池添加任务
	void add_pool_task(Task& task);

	//获取忙线程数
	int get_busy_thr_num();

	//获取当前活跃线程个数
	int get_live_thr_num();

private:
	static void manager_thr(void* arg);
	static void worker_thr(void* arg);

private:
	thread		m_manage_thread_id;		// 管理线程
	thread*		m_work_thread_id;		// 工作线程数组
	queue<Task>	m_task_queue;			// 任务队列

	int			m_min_thr_num;			//最小线程数
	int			m_max_thr_num;			//最大线程数
	int			m_live_thr_num;			//当前存活线程数
	int			m_busy_thr_num;			//忙线程数
	int			m_wait_exit_thr_num;	//待销毁线程数

	mutex		m_thr_pool_mutex;		//线程池锁
	condition_variable	m_queue_not_empty_cond; //队列是否空条件变量
	int			m_shutdown;				//是否销毁线程池，1是，0否
};

#endif // !THREAD_POOL_H