#pragma once
#ifndef HASH_H
#define	HASH_H
#define _CRT_SECURE_NO_WARNINGS
#include <openssl/md5.h>//hash算法的一类
#include <openssl/sha.h>//hash算法的一类
#include <iostream>
#include <string.h>

#include <openssl/pem.h>//为包含下面的NID宏

//使用该类前需项目加入openssl的include包含目录及库lib目录，将接口封装为C++

//enum HashType
//{
//	T_MD5,		//16byte,就是hash出来的值
//	T_SHA1,		//20byte
//	T_SHA224,	//28byte
//	T_SHA256,	//32byte
//	T_SHA384,	//48byte
//	T_SHA512	//64byte
//};
enum HashType
{
	T_MD5		= NID_md5,		//16byte,就是hash出来的值
	T_SHA1		= NID_sha1,		//20byte
	T_SHA224	= NID_sha224,	//28byte
	T_SHA256	= NID_sha256,	//32byte
	T_SHA384	= NID_sha384,	//48byte
	T_SHA512	= NID_sha512	//64byte
};

class Hash
{
public:
	Hash();
	~Hash();

	int init_hash_type(HashType type);

	int add_data(std::string data);
	std::string hash_result();

private:
	//MD5
	inline int md5_init() { return MD5_Init(&m_md5); }
	inline int md5_add_data(const char* data) { return MD5_Update(&m_md5, data, strlen(data)); }
	std::string md5_result();

	//SHA1
	inline int sha1_init() { return SHA1_Init(&m_sha1); }
	inline int sha1_add_data(const char* data) { return SHA1_Update(&m_sha1, data, strlen(data)); }
	std::string sha1_result();

	//SHA224
	inline int sha224_init() { return SHA224_Init(&m_sha224); }
	inline int sha224_add_data(const char* data){ return SHA224_Update(&m_sha224, data, strlen(data)); }
	std::string sha224_result();

	//SHA256
	inline int sha256_init() { return  SHA256_Init(&m_sha256); }
	inline int sha256_add_data(const char* data) { return SHA256_Update(&m_sha256, data, strlen(data)); }
	std::string sha256_result();

	//SHA384
	inline int sha384_init() { return SHA384_Init(&m_sha384); }
	inline int sha384_add_data(const char* data){ return SHA384_Update(&m_sha384, data, strlen(data)); }
	std::string sha384_result();

	// sha512
	inline int sha512_init() { return SHA512_Init(&m_sha512); }
	inline int sha512_add_data(const char* data) { return SHA512_Update(&m_sha512, data, strlen(data));  }
	std::string sha512_result();


private:
	HashType	m_type;	//hash方法
	MD5_CTX		m_md5;	//算法变量
	SHA_CTX		m_sha1;
	SHA256_CTX	m_sha224;
	SHA256_CTX	m_sha256;
	SHA512_CTX	m_sha384;
	SHA512_CTX	m_sha512;
};


#endif // !HASH_H



