#include "../include/ShareMemManager.h"

int load_shm_lib()
{
	return ShareMemManager::init_shm_lib();
}

int unload_shm_lib()
{
	return ShareMemManager::uninit_shm_lib();
}

bool ShareMemManager::m_exist_flag = false;
GHANDLE	ShareMemManager::m_client_sec_area_handle = 0;
char* ShareMemManager::m_pclient_sec_area = 0;
GHANDLE	ShareMemManager::m_client_sec_area_lock = 0;
int ShareMemManager::m_client_sec_area_size = 1024;//单位：字节

ShareMemManager::ShareMemManager()
{
}

ShareMemManager::~ShareMemManager()
{
}

int ShareMemManager::set_sec_info(SecKeyInfo*& psec_info)
{
	//semaphore_P(m_client_sec_area_lock);//暂无多个进程同时访问，不必加锁
	//最简易版
	memcpy(m_pclient_sec_area, (char*)psec_info, sizeof(SecKeyInfo));

	//semaphore_V(m_client_sec_area_lock);
	return 0;
}

int ShareMemManager::get_sec_info(SecKeyInfo*& psec_info)
{
	//最简易版
	/*buf = (char*)realloc(buf, m_client_sec_area_size);
	buf_size = m_client_sec_area_size;
	memcpy((void *)buf, m_pclient_sec_area, m_client_sec_area_size);*/
	memcpy((char*)&psec_info, m_pclient_sec_area, sizeof(SecKeyInfo));

	return 0;
}

int ShareMemManager::init_shm_lib()
{
	if (m_exist_flag)
	{
		return 0;
	}

	//需要通过配置文件来开辟空间可先读文件
	//创建内存
	create_client_area_shmem();

	//创建锁
	create_system_sem();

	m_exist_flag = true;

	return 0;
}

int ShareMemManager::uninit_shm_lib()
{
	if (!m_exist_flag)
		return -1;

	release_all_locks();
	unmap_all_shmem();
	m_exist_flag = false;

	return 0;
}

int ShareMemManager::create_client_area_shmem()
{
	if (m_exist_flag)
		return 0;
	int	cur_exist_flag;	//当前shmem存在标志

	//设置开辟shmem大小,单位：字节
	int shmem_size = sizeof(SecKeyInfo) * MAX_CLIENT_SEC_KEY_NUM;
	m_client_sec_area_size = sizeof(SecKeyInfo) * MAX_CLIENT_SEC_KEY_NUM;
	
#ifdef _WIN32
	//获取shmem句柄
	//win下要字符串名，linux下要id整型值
	m_client_sec_area_handle = init_shm_relay(CLIENT_SEC_AREA_KEY, shmem_size, cur_exist_flag);
	//映射为地址
	m_pclient_sec_area = shmem_map_relay(m_client_sec_area_handle);
#else
	//获取shmem句柄
	m_client_sec_area_handle = init_shm_relay(CLIENT_SEC_AREA_KEY, shmem_size, cur_exist_flag);
	//映射为地址
	m_pclient_sec_area = shmem_map_relay(m_client_sec_area_handle);
	//printf("m_client_sec_area_handle:%d\n", m_client_sec_area_handle);
	//printf("m_pclient_sec_area:%p\n", m_pclient_sec_area);
	//置上删除标记，连接数为0时删除shmem，但标记上新进程就不能附加到该shmem了
	//shmctl(m_client_sec_area_handle, IPC_RMID, nullptr);
	//printf("shmctl ret = %d\n", ret);
#endif // _WIN32
	
	if (cur_exist_flag)
	{
		//这个内存存在就不初始化，别覆盖数据
		return 0;
	}

	init_client_area_shmem();

	return 0;
}

int ShareMemManager::init_client_area_shmem()
{
	//最简易版
	if (m_exist_flag)
		return 0;
	memset(m_pclient_sec_area, 0, sizeof(SecKeyInfo));

	return 0;
}


int ShareMemManager::unmap_all_shmem()
{
	if (!m_exist_flag)
		return -1;

	shmem_unmap_relay(m_pclient_sec_area);
	m_pclient_sec_area = NULL;

#ifdef _WIN32
	if (m_client_sec_area_handle != NULL)
	{
		CloseHandle(m_client_sec_area_handle);
		m_client_sec_area_handle = NULL;
}
#else
	//linux下pid_t是整型值，不用释放
#endif // _WIN32


	return 0;
}

int ShareMemManager::create_system_sem()
{
	if (m_exist_flag)
		return 0;

	m_client_sec_area_lock = 0;
	m_client_sec_area_lock = init_sem_relay(CLIENT_SEC_SEM_KEY, SEM_INITVAL);
	//printf("m_client_sec_area_lock = %d\n", m_client_sec_area_lock);
	if (m_client_sec_area_lock == 0)
	{
		exit(-1);
	}

	return 0;
}

int ShareMemManager::release_all_locks()
{
	if (!m_exist_flag)
		return 0;

#ifdef _WIN32
	if (m_client_sec_area_lock != NULL)
	{
		CloseHandle(m_client_sec_area_lock);
		m_client_sec_area_lock = NULL;
	}

#else
	//linux下pid_t是整型值，不用释放
#endif // _WIN32

	return 0;
}

#ifdef _WIN32
GHANDLE init_shm_relay(const char* shmem_name, int shmem_size, int& cur_exist)
{
	GHANDLE	shmemhan;
	char gl_shmem_name[128] = { 0 };

	cur_exist = false;
	shmemhan = NULL;

	gl_shmem_name[0] = '\0';
	strcpy_s(gl_shmem_name, "Global\\"); // 创建一个全局命名的内核对象（如文件映射对象、互斥体、信号量等）
	strcat_s(gl_shmem_name, shmem_name);

	//win打开文件映射对象的获取句柄
	shmemhan = CreateFileMapping(
		INVALID_HANDLE_VALUE,	// 文件句柄或 INVALID_HANDLE_VALUE
		NULL,					// 默认安全属性
		PAGE_READWRITE,			// 读写权限
		0,						// 高位大小
		shmem_size,				// 低位大小
		gl_shmem_name);			// shmem名称
	if (shmemhan == NULL)
	{
		//创建共享内存失败
		printf( "Share memory create error:%s\n", shmem_name);
		exit(0);
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		//当前共享内存已经存在
		cur_exist = TRUE;
	}

	return shmemhan;
}
#else
GHANDLE init_shm_relay(key_t shmem_name, int shmem_size, int& cur_exist)
{
	int shmemhan = -1;

	cur_exist = false;
	shmemhan = shmget(
		shmem_name,		// 共享内存的键值
		shmem_size,		// 共享内存段的大小
		0666 | IPC_CREAT | IPC_EXCL);// 共享内存段的大小
	if (shmemhan == -1)
	{
		//创建共享内存失败
		//接下来查看该共享内存是否存在
		shmemhan = shmget(shmem_name, shmem_size, 0666 | IPC_EXCL);
		if (shmemhan == -1)
		{
			exit(0);
		}
		//printf("exist shmemhan:%d\n", shmemhan);
		cur_exist = true;
	}

	return shmemhan;
}
#endif //_WIN32

char* shmem_map_relay(GHANDLE shMemHan)
{
	char* mem_ptr;
#ifdef	_WIN32
	mem_ptr = (char*)MapViewOfFile(
		shMemHan,		// 文件映射对象的句柄
		FILE_MAP_WRITE,	// 映射对象的访问模式（写入模式）
		0,				// 文件映射对象在文件中的偏移量的高位
		0,				// 文件映射对象在文件中的偏移量的低位
		0);				// 映射的字节数，0 表示映射整个文件
#else
	mem_ptr = (char*)shmat(
		shMemHan,		// 共享内存标识符（由 shmget 返回）
		0,				// 指定系统为映射选择的地址，通常设置为 0	
		0);				// 操作标志位，通常设置为 0
#endif
	return mem_ptr;
}

int shmem_unmap_relay(const char* share_ptr)
{
#ifdef _WIN32
	//当前进程与shmem分离
	UnmapViewOfFile(share_ptr);
#else
	//当前进程与shmem分离
	shmdt(share_ptr);
#endif
	return 0;
}

#ifdef _WIN32
GHANDLE init_sem_relay(const char* sem_name, int init_val)
{
	GHANDLE	semHan;

	//获取命名信号量
	semHan = CreateSemaphore(
		NULL,		// 默认安全描述符
		init_val,	// 初始计数器值
		init_val,	// 最大计数器值
		sem_name);	// 信号量名称

	return semHan;
}
#else
GHANDLE init_sem_relay(key_t sem_name, int init_val)
{
	union	semun ini_v;// 联合体，用于信号量操作参数
	int	semid;// 存储信号量集标识符(ID)
	int	ret;

	ini_v.val = 1;
	//获取信号量集
	semid = semget(
		sem_name,	// 信号量的键值，用于唯一标识信号量集
		1,			// 需要创建或获取的信号量数量
		0666 | IPC_CREAT | IPC_EXCL);// 0666访问权限|不存在即创建|存在及返回错误
	//printf("semid : %d\n", semid);
	if (semid == -1)
	{
		//创建失败,观察是否已经存在
		//原先判断信号量存在的方法:在多线程不安全的环境下可能有问题;
# ifdef	_HPUX
		//HP-UX 操作系统上编译
		//直接获取
		semid = semget(sem_name, 1, 0666 | IPC_EXCL);
# else
#    ifdef _IBM
		//IBM 的 AIX 操作系统上编译
		if (errno == EIO || errno == EEXIST)
#    else
		if (errno == EEXIST)
#    endif
			semid = semget(sem_name, 1, 0666 | IPC_EXCL);
		else
		{
			//printf( "打开信号量错误1,sem_name=%x\n", sem_name );
			return -1;
		}
# endif
	}
	else
	{
		//首次创建,给其初始化
		ret = semctl(semid, 0, SETVAL, ini_v);
		if (ret == -1 || semid == -1)
		{
			//printf("打开信号量错误2,sem_name=%x\n",sem_name);
			return -2;
		}
		//int ret = -1;
		//置上删除标记，连接数为0时删除sem，标记上新进程就附加不了这个了
		//ret = semctl(semid, 0, IPC_RMID);
		//printf("semctl ret = %d\n", ret);
	}

	return semid;
}
#endif //_WIN32


//P操作－进入关键区
int semaphore_P(GHANDLE sem_han)
{
#ifdef _WIN32
	int retcode;

	//printf( "上锁, sem_han = %d\n", sem_han );
	// 尝试等待信号量，超时时间为3000毫秒（3秒）
	retcode = WaitForSingleObject(sem_han, 3000);
	if (retcode == WAIT_OBJECT_0)
	{
		// 成功等待到信号量，返回0表示成功
		return 0;
	}
	else
	{
		//printf( "上锁失败\n" );
		// 等待失败，释放信号量（此处释放信号量的操作通常不需要）
		ReleaseSemaphore(sem_han, 1, NULL);
		return -1;
	}
#else
	struct sembuf	p_buf;

	// 设置P操作参数
	p_buf.sem_num = 0;	// 信号量集中的第一个信号量
	p_buf.sem_op = -1;	// P操作，减1
	p_buf.sem_flg = SEM_UNDO;	// 在进程异常退出时自动撤销该操作

	// 执行P操作
	if (semop(sem_han, &p_buf, 1) == -1)
	{
		//printf( "上锁失败, sem_han = %d, 错误号:%d %s\n",
		//sem_han, errno, error_string(errno) );
		// P操作失败，返回-1表示失败
		return -1;
	}
#endif
	// 成功进入临界区，返回0表示成功
	return 0;
}
//V操作－离开关键区
int semaphore_V(GHANDLE sem_han)
{
#ifdef _WIN32
	// 释放信号量，将其值增加1
	// sem_han: 信号量句柄
	// 1: 增加的数量
	// NULL: 保留参数，通常设置为NULL
	// 释放成功后返回0
	ReleaseSemaphore(sem_han, 1, NULL);
	// 	printf( "解锁, sem_han = %d\n", sem_han );
#else
	struct sembuf	v_buf;

	v_buf.sem_num = 0;	// 信号量集中的第一个信号量
	v_buf.sem_op = 1;	// V操作，加1
	v_buf.sem_flg = SEM_UNDO;	// 在进程异常退出时自动撤销该操作

	if (semop(sem_han, &v_buf, 1) == -1)
	{
		// 		printf( "解锁失败, sem_han = %d, 错误号:%d %s\n",
		// 			sem_han, errno, error_string(errno) );
		// V操作失败，返回-1表示失败
		return -1;
	}
#endif
	// 成功离开临界区，返回0表示成功
	return 0;
}
