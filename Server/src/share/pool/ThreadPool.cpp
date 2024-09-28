#include "../../include/pool/ThreadPool.h"


const int NUMBER = 2; // 单次添加或销毁线程数
ThreadPool::ThreadPool(int min_thr_num, int max_thr_num)
{
	do
	{
		m_min_thr_num = min_thr_num;
		m_max_thr_num = max_thr_num;
		m_live_thr_num = min_thr_num;
		m_busy_thr_num = 0;
		m_wait_exit_thr_num = 0;

		m_shutdown = 0;
		// 创建并启动管理者的线程
		m_manage_thread_id = thread(manager_thr, this);
		// 创建工作者线程
		m_work_thread_id = (thread*)malloc(sizeof(thread) * max_thr_num);
		if (m_work_thread_id == NULL)
		{
			printf("work thread malloc failed!!!\n");
			break;
		}
		memset(m_work_thread_id, 0, sizeof(thread) * max_thr_num);
		for (int i = 0; i < m_min_thr_num; i++)
		{
			m_work_thread_id[i] = thread(worker_thr, this);
		}

		return;
	} while (0);

}

ThreadPool::~ThreadPool()
{
	m_shutdown = 1;
	//阻塞回收管理者线程
	if (m_manage_thread_id.joinable())//看是否可join
	{
		m_manage_thread_id.join();//join等待线程退出
	}
	//无任务了，但也通知所有活着线程，"自杀"
	m_queue_not_empty_cond.notify_all();
	for (int i = 0; i < m_max_thr_num; i++)
	{
		if (m_work_thread_id[i].joinable())
		{
			m_work_thread_id[i].join();
		}
	}
	printf("ThreadPool is destroy!!!\n");
}

void ThreadPool::add_pool_task(Task& task)
{
	////通过自身生命周期自动加解锁的变量,add_task已加锁，这里不加了
	//unique_lock<mutex> lock(m_thr_pool_mutex);
	m_thr_pool_mutex.lock();
	if (m_shutdown)
		return;
	//添加任务
	m_task_queue.push(task);
	m_queue_not_empty_cond.notify_one();
	m_thr_pool_mutex.unlock();
}

int ThreadPool::get_busy_thr_num()
{
	m_thr_pool_mutex.lock();
	int busy_num = m_busy_thr_num;
	m_thr_pool_mutex.unlock();
	return busy_num;
}

int ThreadPool::get_live_thr_num()
{
	m_thr_pool_mutex.lock();
	int live_num = m_live_thr_num;
	m_thr_pool_mutex.unlock();
	return live_num;
}

void ThreadPool::manager_thr(void* arg)
{
	ThreadPool* pool = static_cast<ThreadPool*>(arg);//C++的转换，类C的强转
	while (!pool->m_shutdown)
	{
		//每隔3秒检测一次
#ifdef _WIN32
		Sleep(3000);
#else
		sleep(3);
#endif // _WIN32

		//取出线程池中任务的数量和当前线程的数量
		pool->m_thr_pool_mutex.lock();
		int queue_size = pool->m_task_queue.size();
		int live_num = pool->m_live_thr_num;
		int busy_num = pool->m_busy_thr_num;
		pool->m_thr_pool_mutex.unlock();

		//添加线程
		//任务的个数 > 存活线程个数 && 存活线程个数 < 最大线程数
		if (queue_size > live_num && live_num < pool->m_max_thr_num)
		{
			pool->m_thr_pool_mutex.lock();
			int count = 0;
			//从i遍历到max主要看数组里有哪些可以存放新创建线程的id
			//而且增加DEFAULT_THREAD_VARY线程数还要保证存活线程要小于max
			for (int i = 0; i < pool->m_max_thr_num && count < NUMBER
				&& pool->m_live_thr_num < pool->m_max_thr_num; i++)
			{
				//检查 pool->threadIDs[i] 是否是一个空线程。thread::id表示空线程（即线程未初始化）
				if (pool->m_work_thread_id[i].get_id() == thread::id())
				{
					printf("Create a new Thread...\n");
					pool->m_work_thread_id[i] = thread(worker_thr, pool);
					count++;
					pool->m_live_thr_num++;
				}
			}
			pool->m_thr_pool_mutex.unlock();
		}

		//销毁线程
		//忙线程*2 < 存活线程数 && 存活线程数 > 最小线程数
		if (busy_num * 2 < live_num && live_num > pool->m_min_thr_num)
		{
			pool->m_thr_pool_mutex.lock();
			pool->m_wait_exit_thr_num++;
			pool->m_thr_pool_mutex.unlock();

			//让空转的线程，假通知取任务让其"自杀"
			for (int i = 0; i < NUMBER; i++)
			{
				pool->m_queue_not_empty_cond.notify_one();
			}
		}

	}
}

void ThreadPool::worker_thr(void* arg)
{
	ThreadPool* pool = static_cast<ThreadPool*>(arg);

	//线程启动即不停检查是否有任务
	while (1)
	{
		//创建即加锁，默认声明周期结束解锁
		unique_lock<mutex> lk(pool->m_thr_pool_mutex);
		//当前队列是否为空且是否线程池运行
		while (pool->m_task_queue.size() == 0 && !pool->m_shutdown)
		{
			//阻塞工作线程，等到有任务通知
			pool->m_queue_not_empty_cond.wait(lk);//wait需要自动加解锁mutex的变量unique_lock
			//如果通知假任务
			if (pool->m_wait_exit_thr_num > 0)
			{
				pool->m_wait_exit_thr_num--;
				if (pool->m_live_thr_num > pool->m_min_thr_num)
				{
					pool->m_live_thr_num--;
					//printf("threadid: %ld exit....\n", this_thread::get_id());//c++11跨平台获取线程id
					cout << "threadid: " << this_thread::get_id() << "	exit..." << endl;//c++11跨平台获取线程id
					lk.unlock();
					return;//退出线程
				}
			}
		}

		//判断线程池是否关闭了
		if (pool->m_shutdown)
		{
			cout << "ThreadPool exit,threadid: " << this_thread::get_id() << "	exit..." << endl;
			return;//退出线程
		}

		//从任务池中拿取一个任务
		Task task = pool->m_task_queue.front();
		pool->m_task_queue.pop();

		pool->m_busy_thr_num++;
		//解锁lk
		lk.unlock();

		cout << "threadid: " << this_thread::get_id() << "	start working..." << endl;
		task.function(task.arg);//(*task.function)(task.arg);另种方法

		cout << "threadid: " << this_thread::get_id() << "	end working..." << endl;
		lk.lock();
		pool->m_busy_thr_num--;
		lk.unlock();

	}
}
