#pragma once
#ifndef _SECCLIENT_H
#define _SECCLIENT_H
#include "../include/public_macro.h"
#include "../include/json/json.h"
#include "../include/ShareMemManager.h"
#include "../include/openssl/RsaCrypto.h"
#include "../include/TcpSocket.h"
#include "RequestCodec.h"
#include "ResponCodec.h"
//#include "Message.pb.h"
using namespace Json;

class SecClient
{
public:
	SecClient();
	~SecClient();

	//读取终端配置文件
	int read_config(const char* jsonFile);

	//秘钥协商
	int seckeyAgree();

	//秘钥校验
	int seckeyCheck();

	//秘钥注销
	int seckeyLogout();


private:
	char	m_client_name[MAX_COMMON_NAME_LEN];	//客户端名
	int		m_client_id;						//客户端id
	int		m_server_id;						//服务器id
	char	m_server_ip[MAX_COMMON_NAME_LEN];	//服务器ip
	int		m_server_port;						//服务器端口


	RsaCrypto			m_rsa;					//openssl非对称及hash封装类
	//RequestInfo			m_reqInfo;				//请求报文数据结构
	//RequestCodec		m_reqCode;				//请求报文序列化
	//ResponInfo			m_resInfo;				//响应报文数据结构
	//ResponCodec			m_resCode;				//响应报文序列化
	SecKeyInfo*			m_psecinfo;				//对称秘钥结构(用存共享内存)



private:
	ShareMemManager		m_shm;
	//TcpSocket* ptcp;//长连接测试

};

#endif // !_SECCLIENT_H