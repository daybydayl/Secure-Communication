#pragma once
#ifndef _PUBLIC_STRUCT_H
#define _PUBLIC_STRUCT_H
#include "public_macro.h"

//非对称密钥信息通过网络传输给其他结点，已用protobuf编码
//对称密钥信息结构体,用于放共享内存，进程间通信
struct SecKeyInfo
{//这里都用C的类型是因为共享内存都是字节存储，用原始类型存准确
	int		server_id;	//服务器id(关键字)
	int		client_id;	//客户端id(关键字)
	int		sec_status;	//秘钥状态，1可用，0不可用
	int		sec_id;		//秘钥id(关键字)
	char	sec_key[MAX_FILE_NAME_LEN];	//对称加密的秘钥
};

#endif // !_PUBLIC_STRUCT_H

