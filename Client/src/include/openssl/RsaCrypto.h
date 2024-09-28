#pragma once
#ifndef RSACRYPTO_H
#define RSACRYPTO_H

#include <string>
#include <openssl/rsa.h>
#include <openssl/pem.h>

#include <openssl/bio.h>//base64所用
#include <openssl/buffer.h>

#include "Hash.h"//签名用
using namespace std;

enum SignLevel
{
	Level1 = NID_md5,
	Level2 = NID_sha1,
	Level3 = NID_sha224,
	Level4 = NID_sha256,
	Level5 = NID_sha384,
	Level6 = NID_sha512
};

//非对称加密，已有hash编码，无需另个封装的hash类
//公钥加密私钥解密一般分发对称密钥,一般对称密钥才是加密通信数据的
//私钥加密公钥解密一般签名
class RsaCrypto
{
public:
	RsaCrypto();
	~RsaCrypto();

public:
	//生成RSA密钥对,填缺省值可写入文件
	int generate_rsa_key(
		int bit_num,	//bit_num为秘钥长度也是数据加密后长度(len = bit_num/8), 单位: bit, 常用的长度 1024*n (n正整数) [in]
		string pub = ""/*"public.pem"*/,	//生成的密钥对写入文件 [in]
		string pri = ""/*"private.pem"*/);	//生成的密钥对写入文件 [in]
	//从文件获取密钥对
	int generate_rsa_key_from_file(
		string pub_file,				//获取公钥文件 [in]
		string pri_file);				//获取私钥文件 [in]

	//公钥转换为PEM格式的字符串,方便传走
	int pubkey_to_string(string& pubkey);//out

	//私钥转换为PEM格式的字符串
	int prikey_to_string(string& prikey);//out

	//从字符串转换为公钥
	int string_to_pubkey(const string pubkey);//in

	//从字符串转换为私钥
	int string_to_prikey(const string prikey);//in

	//公钥加密
	int rsa_pub_key_encrypt(
		const string data,				//待加密数据 [in]
		string& dec_data);				//加密后密文 [out]
	//私钥解密
	int rsa_pri_key_decrypt(
		const string enc_data,			//待解密密文 [in]
		string& data);					//解密后数据 [out]
	//使用rsa签名
	int rsa_sign(
		const string data,				//待签名数据 [in]
		string& sign_data,				//签名后密文 [out]
		SignLevel level = Level3);		//签名(hash)算法 [in]
	// 使用RSA验证签名
	int rsa_verify(
		const string sign_data,			//签名密文 [in]
		const string data,				//待验证数据 [in]
		SignLevel level = Level3);		//签名(hash)算法 [in]

	//使用hash对数据计算哈希值
	int hash_data(
		const string data,				//待hash数据
		string& hash_value,				//hash值
		SignLevel level = Level3);		//签名(hash)算法 [in]

	
private:
	//初始公钥
	bool get_pub_key_from_file(string pubfile);
	//初始私钥
	bool get_pri_key_from_file(string prifile);

	//生成的密钥串是二进制，但对字符串都是遇0就结束，所以存在不全的可能
	//通过base64编码让二进制变成易保存的字符串
	//base64编码
	int toBase64(const char* str, //待编码串
		const int len,				//长度
		string &base64_str);		//base64编码后
	//base64解码
	int fromBase64(const string base64_str,	//待解码的base64
		char*& buf,							//解码后的原串
		int& len);							//原串长度

private:
	RSA*	m_public_key;	// 私钥
	RSA*	m_private_key;	// 公钥

	Hash	m_hash;

};

#endif // RSACRYPTO_H




