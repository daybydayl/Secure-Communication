#pragma once
#ifndef SHAREMEMMANAGER_H
#define SHAREMEMMANAGER_H

#include "../include/TcpSocket.h"
#include "../include/public_macro.h"
#include "../include/public_struct.h"

#ifdef _WIN32

#else
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#endif // _WIN32

/*
该类共享内存和信号量操作都是使用System V的方法，还有POSIX方法和mmap文件映射方法

如果需要与传统 UNIX 系统兼容，并且可以接受较为复杂的管理，可以选择 System V 共享内存。
如果需要简单易用的 API，且系统支持，可以选择 POSIX 共享内存。另外posix方法的信号量是线程安全，System V不是
如果需要持久化存储，并且跨平台支持，可以选择内存映射文件。

该类有一缺陷(同郭佬)，linux下共享内存和信号量不会释放，
如果用shmctl标记删除的话新的进程也不能连接到该shmem*/

//外封装加载函数
int load_shm_lib();
int unload_shm_lib();

//秘钥共享内存，同机用一块内存，异机用各自申请的内存
class ShareMemManager
{
public:
	ShareMemManager();
	~ShareMemManager();

public:
	//对key内存区操作的接口
	int set_sec_info(
		SecKeyInfo*& psec_info);	//密钥结构[in]

	int get_sec_info_by_condition(
		const int sec_id,			//秘钥id[in],值<=0则忽略该条件
		const int server_id,		//服务器id[in],值<=0则忽略该条件
		const int client_id,		//客户端id[in],值<=0则忽略该条件
		SecKeyInfo*& psec_info);	//密钥结构[out]
	
	int get_all_sec_info(
		SecKeyInfo*& psec_infos,	//密钥数组[out]
		int& sec_num);				//密钥数[out]

	int del_sec_info_by_condition(
		const int sec_id,			//秘钥id[in],值<=0则忽略该条件
		const int server_id,		//服务器id[in],值<=0则忽略该条件
		const int client_id);		//客户端id[in],值<=0则忽略该条件


private:
	//装载库接口加入友元
	friend int load_shm_lib();
	//卸载库接口加入友元
	friend int unload_shm_lib();
	//分离shmem
	friend int shmem_unmap_relay(const char* share_ptr);

	//初始化库
	static int init_shm_lib();

	//释放库
	static int uninit_shm_lib();

	//创建密钥shmem
	static int create_key_area_shmem();
	//初始化密钥shmem
	static int init_key_area_shmem();

	//分离shmem
	static int unmap_all_shmem();

	//创建各shmem区锁
	static int create_system_sem();

	//释放所有锁
	static int release_all_locks();

private:
	//shmem存在标志
	static bool		m_exist_flag;

	//shmem的密钥区域句柄
	static GHANDLE	m_key_area_handle;
	//shmem的密钥地址
	static char*	m_pkey_area;
	//shmem的密钥锁
	static GHANDLE	m_key_area_lock;

private:
	static int		m_key_area_size;//存对称密钥总大小[用于后面读文件的话]
	//shmem manager初始化标志
	//bool m_init_flag;

};



/*跨平台封装系统获取内存相关api*/
#ifdef _WIN32
	//获取开辟内存的句柄
GHANDLE init_shm_relay(const char* shmem_name, int shmem_size, int& cur_exist);
#else
	//获取开辟内存的句柄
GHANDLE init_shm_relay(key_t shmem_name, int shmem_size, int& cur_exist);
#endif

//将句柄映射为内存首地址
char* shmem_map_relay(GHANDLE shMemHan);

//将shmem与当前进程分离
int shmem_unmap_relay(const char* share_ptr);

#ifdef _WIN32
//获取系统信号量集合(加锁使用)
GHANDLE init_sem_relay(const char* sem_name, int init_val);
#else
//获取系统信号量集合(加锁使用)
GHANDLE init_sem_relay(key_t sem_name, int init_val);
#endif

//需要互斥同步情况下的加解锁
//P操作－进入关键区
int semaphore_P(GHANDLE sem_han);
//V操作－离开关键区
int semaphore_V(GHANDLE sem_han);



#endif // !SHAREMEMMANAGER_H