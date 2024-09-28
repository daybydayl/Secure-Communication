#pragma once
#ifndef PUBLICMACRO_H
#define PUBLICMACRO_H

#include <stdio.h>
#include <stdlib.h>
using namespace std;
#ifdef _WIN32
#include <WinSock2.h>   // Windows Sockets API
#include <WS2tcpip.h>   // TCP/IP Protocols
#include <Windows.h>    // For Sleep function
#pragma comment(lib, "ws2_32.lib")  // 链接 ws2_32.lib 库
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <semaphore.h>
#include <signal.h>
#endif


#define SEM_INITVAL						1

#ifdef _WIN32
#define SEC_AREA_KEY                    "0x7001"
//#define CLIENT_SEC_AREA_KEY           "0x7002"//服务端客户端不区分内存标识了

#define SEC_SEM_KEY                     "0x8001"
//#define CLIENT_SEC_SEM_KEY            "0x8002"
#else
#define SEC_AREA_KEY                     0x7001
//#define CLIENT_SEC_AREA_KEY            0x7002

#define SEC_SEM_KEY                      0x8001
//#define CLIENT_SEC_SEM_KEY             0x8002

#endif // _WIN32


#ifdef _WIN32
typedef	HANDLE			GHANDLE;	//通用句柄:共享内存,信号量
#else
typedef	pid_t			GHANDLE;	//通用句柄:共享内存,信号量
#endif // _WIN32

#ifdef _WIN32

#else
//#ifdef _LINUX
union semun
{
    int              val;    /* 通过这个字段设置或获取单个信号量的值。 */
    struct semid_ds* buf;    /* 通过这个字段获取或设置信号量集的状态。 */
    unsigned short* array;   /* 通过这个字段获取或设置信号量集中的所有信号量的值。 */
    struct seminfo* __buf;   /* 通过这个字段获取系统范围内的信号量信息。 */
};
//#endif
//#ifdef _UNIX
//union semun
//{
//    int     val;
//    struct semid_ds* buf;
//    unsigned short* array;
//};
//#endif
#endif

#define MAX_SEC_KEY_NUM  	            20  //共享内存最大秘钥数(对称秘钥)
//#define MAX_CLIENT_SEC_KEY_NUM  	    1   //客户端共享内存最大秘钥数(对称秘钥)

#define MAX_COMMON_NAME_LEN		        64	//通用名最大长
#define MAX_FILE_NAME_LEN      			512 //最长文件名长


#define SEC_KEY_AGREE           		1   //秘钥协商
#define SEC_KEY_CHECK                   2   //秘钥校验
#define SEC_KEY_LOGOUT                  3   //秘钥注销
#define SEC_KEY_LOOK                    4   //秘钥查看





#endif // PUBLICMACRO_H
