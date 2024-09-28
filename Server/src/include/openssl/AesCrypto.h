#pragma once
#ifndef AESCRYPTO_H
#define AESCRYPTO_H

#include <string>
#include <openssl/aes.h>
using namespace std;

class AesCrypto
{
public:
	//可使用 16byte, 24byte, 32byte 的秘钥,字符串可随意，接口自适应
	AesCrypto(string key);
	~AesCrypto();

public:
	//加密
	int aes_cbc_encrypt(
		const string data,		//待加密数据	[in]
		string& enc_data);		//加密的密文 [out]
	//解密
	int aes_cbc_decrypt(
		const string enc_data,	//待解密的密文 [in]
		string& data);			//解密的数据 [out]

private:
	//加解密
	int aes_crypto(
		const string in,
		string& out,
		int crypto);
	//生成初始向量
	int generate_ivec(
		unsigned char* ivec);

private:
	AES_KEY		m_enc_key;	//密钥格式变量
	AES_KEY		m_dec_key;	//密钥格式变量
	string			m_key;		//密钥

};

#endif // !AESCRYPTO_H


