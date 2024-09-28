#pragma once
#ifndef _SECSERVER_H
#define _SECSERVER_H
#include <thread>
#include <map>
#include "../include/public_macro.h"
#include "../include/json/json.h"
#include "../include/ShareMemManager.h"
#include "../include/openssl/RsaCrypto.h"
#include "../include/TcpSocket.h"
#include "../include/TcpServer.h"
#include "RequestCodec.h"
#include "ResponCodec.h"


class SecServer
{
public:
	SecServer();
	~SecServer();

	//读取终端配置文件
	int read_config(const char* jsonFile);

	//启动服务器
	void startServer();

	//线程工作函数 -> 推荐使用,友元破坏了类的封装
	static void* work_thr(void* arg);


	//秘钥协商
	int seckeyAgree(
		const RequestInfo* reqInfo,	//[in]请求
		string& respon);			//[out]响应数据

	//秘钥校验
	int seckeyCheck(
		const RequestInfo* reqInfo,	//[in]请求
		string& respon);			//[out]响应数据

	//秘钥注销
	int seckeyLogout(
		const RequestInfo* reqInfo,	//[in]请求
		string& respon);			//[out]响应数据


private:
	enum KeyLen { Len16 = 16, Len24 = 24, Len32 = 32 };
	//字符串中包含: a-z, A-Z, 0-9, 特殊字符
	int get_rand_key(
		const KeyLen len,			//[in]密钥长度
		string &rand_key);			//[out]对称密钥


private:
	char	m_server_name[MAX_COMMON_NAME_LEN];	//服务端名
	int		m_server_id;						//服务器id
	int		m_port;								//监听端口
	int		m_seckey_id;						//秘钥id

	TcpServer*		m_server;					//tcp监听类		
	map<std::thread::id, TcpSocket*>	m_mp_thr_skt;//工作线程id对应套接字

	ShareMemManager m_shm;						//服务端共享内存

	RsaCrypto		m_rsa;						//openssl非对称及hash封装类

	//数据库相关，待完善
};

#endif // !_SECSERVER_H