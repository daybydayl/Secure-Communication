#include "../../include/pool/TaskPool.h"

TaskPool::TaskPool()
{

}

TaskPool::~TaskPool()
{

}

void TaskPool::add_task(Task& task)
{
	//¼ÓËø
	//m_task_mutex.lock();
	m_task_queue.push(task);
	//m_task_mutex.unlock();
}

void TaskPool::add_task(callback func, void* arg)
{
	
	Task t;
	t.arg = arg;
	t.function = func;
	//¼ÓËø
	//m_task_mutex.lock();
	m_task_queue.push(t);
	//m_task_mutex.unlock();
}

Task TaskPool::get_one_task()
{
	if (m_task_queue.size() > 0)
	{
		//¼ÓËø
		//m_task_mutex.lock();
		Task t = m_task_queue.front();
		m_task_queue.pop();
		//m_task_mutex.unlock();
		return t;
	}

	return Task();
}
