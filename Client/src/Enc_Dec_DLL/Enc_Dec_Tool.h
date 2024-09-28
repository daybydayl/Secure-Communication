#pragma once
#include "../include/openssl/AesCrypto.h"
#include "../include/ShareMemManager.h"
using namespace std;

////可在项目预处理加ENC_DEC_DLL_EXPORT宏用这方式
//#if defined(ENC_DEC_DLL_EXPORT)
//#	define ENC_DEC_DLL_API __declspec(dllexport)
//#elif defined(ENC_DEC_DLL_DIRECT)
//#	define ENC_DEC_DLL_API __declspec(dllimport)
//#else
//#	define ENC_DEC_DLL_API
//#endif
//
//// If no API is defined, assume default
//#ifndef ENC_DEC_DLL_API
//#	define ENC_DEC_DLL_API
//#endif

#ifdef _WIN32
class __declspec(dllexport) Enc_Dec_Tool
#else
class Enc_Dec_Tool
#endif // _WIN32
{
public:
	Enc_Dec_Tool();
	~Enc_Dec_Tool();

public:
	int get_all_sec_info(
		SecKeyInfo*& psec_infos,	//密钥数组[out]
		int& sec_num);				//密钥数[out]

	//加解密前先创建aes对象
	int create_aes_by_sec(
		const int sec_id,			//秘钥id[in],值<=0则忽略该条件
		const int server_id = -1,	//服务器id[in],值<=0则忽略该条件
		const int client_id = -1);	//客户端id[in],值<=0则忽略该条件

public:
	// 数据加密
	int encryptData(const string data,string &enc_data);
	// 数据解密
	int decryptData(const string dec_data,string &data);


public:
	ShareMemManager		m_shmem;
	SecKeyInfo*			m_pkey_info;
	AesCrypto*			m_paes_obj;
};

