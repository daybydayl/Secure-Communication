#include "../../include/openssl/RsaCrypto.h"
#include <openssl/bio.h>
#include <openssl/err.h>
#include <iostream>
#ifdef _WIN32
extern "C"
{
#include <openssl/applink.c>//官方的ssl库封装但没有带这个c文件一块编译，所以这里要手动申明C文件一块编译
};
#else

#endif // _WIN32


RsaCrypto::RsaCrypto()
{
	m_public_key = RSA_new();
	m_private_key = RSA_new();
}

RsaCrypto::~RsaCrypto()
{
	RSA_free(m_private_key);
	RSA_free(m_public_key);
}

int RsaCrypto::generate_rsa_key(int bit_num, string pub, string pri)
{
	RSA* r = RSA_new();
	// 生成RSA密钥对
	// 创建bignum对象
	BIGNUM* e = BN_new();
	// 初始化bignum对象
	BN_set_word(e, 456787);
	if (bit_num % 8 != 0)
		bit_num = bit_num - (bit_num % 8);
	RSA_generate_key_ex(r, bit_num, e, NULL);

	// 得到公钥和私钥
	m_private_key = RSAPrivateKey_dup(r);
	m_public_key = RSAPublicKey_dup(r);

	if (pub != "")
	{
		// 创建bio文件对象
		BIO* pubIO = BIO_new_file(pub.data(), "w");
		// 公钥以pem格式写入到文件中
		PEM_write_bio_RSAPublicKey(pubIO, r);
		// 缓存中的数据刷到文件中
		BIO_flush(pubIO);
		BIO_free(pubIO);
	}
	if (pri != "")
	{
		// 创建bio对象
		BIO* priBio = BIO_new_file(pri.data(), "w");
		// 私钥以pem格式写入文件中
		PEM_write_bio_RSAPrivateKey(priBio, r, NULL, NULL, 0, NULL, NULL);
		BIO_flush(priBio);
		BIO_free(priBio);
	}

	// 释放资源
	BN_free(e);
	RSA_free(r);
	return 0;
}

int RsaCrypto::generate_rsa_key_from_file(string pub_file, string pri_file)
{
	bool ret;
	if (!pub_file.data() && !pri_file.data())
		return -1;
	ret = get_pub_key_from_file(pub_file);
	if (!ret)
		return ret;
	ret = get_pri_key_from_file(pri_file);
	if (!ret)
		return ret;
	return 0;
}

int RsaCrypto::pubkey_to_string(string& pubkey)
{
	BIO* bio = BIO_new(BIO_s_mem());
	PEM_write_bio_RSAPublicKey(bio, m_public_key);
	char* key_data;
	long key_len = BIO_get_mem_data(bio, &key_data);
	pubkey = std::string(key_data, key_len);
	BIO_free(bio);

	return 0;
}

int RsaCrypto::prikey_to_string(string& prikey)
{
	BIO* bio = BIO_new(BIO_s_mem());
	PEM_write_bio_RSAPrivateKey(bio, m_private_key, NULL, NULL, 0, NULL, NULL);
	char* key_data;
	long key_len = BIO_get_mem_data(bio, &key_data);
	prikey = std::string(key_data, key_len);
	BIO_free(bio);

	return 0;
}

int RsaCrypto::string_to_pubkey(const string pubkey)
{
	BIO* bio = BIO_new_mem_buf(pubkey.data(), pubkey.size());
	m_public_key = PEM_read_bio_RSAPublicKey(bio, NULL, NULL, NULL);
	BIO_free(bio);

	return 0;
}

int RsaCrypto::string_to_prikey(const string prikey)
{
	BIO* bio = BIO_new_mem_buf(prikey.data(), prikey.size());
	m_private_key = PEM_read_bio_RSAPrivateKey(bio, NULL, NULL, NULL);
	BIO_free(bio);

	return 0;
}

int RsaCrypto::rsa_pub_key_encrypt(const string data,string& dec_data)
{
	// 计算公钥长度
	int keyLen = RSA_size(m_public_key);
	//cout << "pubKey len: " << keyLen << endl;
	// 申请内存空间
	char* encode = new char[keyLen + 1];
	// 使用公钥加密，成二进制
	int len = RSA_public_encrypt(data.size(), (const unsigned char*)data.data(),
		(unsigned char*)encode, m_public_key, RSA_PKCS1_PADDING);
	if (len >= 0)
	{
		// 加密成功使用base64将二进制编码为字符串
		//dec_data = string(encode, len);
		toBase64(encode, len, dec_data);
	}
	else
		return -1;
	// 释放资源
	delete[]encode;
	return 0;
}

int RsaCrypto::rsa_pri_key_decrypt(const string enc_data,string& data)
{
	//text指向内存需释放
	char* text = (char*)malloc(RSA_size(m_private_key));
	int text_len;
	//将base64字符串解码成二进制
	fromBase64(enc_data, text, text_len);

	// 计算私钥长度
	int keyLen = RSA_size(m_private_key);
	// 使用私钥解密，把二进制还原成数据
	char* decode = new char[keyLen + 1];
	/*int ret = RSA_private_decrypt(enc_data.size(), (const unsigned char*)enc_data.data(),
		(unsigned char*)decode, m_private_key, RSA_PKCS1_PADDING);*/
	int ret = RSA_private_decrypt(keyLen, (const unsigned char*)text,
		(unsigned char*)decode, m_private_key, RSA_PKCS1_PADDING);
	if (ret >= 0)
	{
		data = string(decode, ret);
	}
	else
		return -1;
	delete[]decode;
	free(text);
	return 0;
}

int RsaCrypto::rsa_sign(const string data,string& sign_data,SignLevel level)
{
	unsigned int len;
	//私钥加密后的数据
	char* signBuf = new char[RSA_size(m_private_key)];

	//密钥太长，先hash成字串变短
	m_hash.init_hash_type((HashType)level);
	m_hash.add_data(data);
	string hash_data = m_hash.hash_result();
	//对私钥hash后签名(也是加密成二进制)，hash会简短私钥数据，否则太长签名不了
	int ret = RSA_sign(level, (const unsigned char*)hash_data.data(), hash_data.size(), (unsigned char*)signBuf,
		&len, m_private_key);
	/*int ret = RSA_sign(level, (const unsigned char*)data.data(), data.size(), (unsigned char*)signBuf,
		&len, m_private_key);*/
	//cout << "sign len: " << len << endl;
	//sign_data = string(signBuf, len);
	toBase64(signBuf, len, sign_data);
	delete[]signBuf;
	return 0;
}

int RsaCrypto::rsa_verify(const string signData, const string data, SignLevel level)
{
	// 验证签名
	//无base64法
	/*int ret = RSA_verify(level, (const unsigned char*)data.data(), data.size(),
		(const unsigned char*)signData.data(), signData.size(), m_public_key);*/
	int ret;
	char* sign = (char*)malloc(RSA_size(m_public_key));
	int key_len = RSA_size(m_public_key);
	fromBase64(signData, sign, key_len);

	//密钥太长，先hash成字串变短
	m_hash.init_hash_type((HashType)level);
	m_hash.add_data(data);
	string hash_data = m_hash.hash_result();

	//验证签名((也是从二进制解密)
	ret = RSA_verify(level, (const unsigned char*)hash_data.data(), hash_data.size(),
		(const unsigned char*)sign, key_len, m_public_key);
	/*ret = RSA_verify(level, (const unsigned char*)data.data(), data.size(),
		(const unsigned char*)sign, key_len, m_public_key);*/
	if (ret != 1)
	{
		free(sign);
		return -1;
	}
	free(sign);
	return 0;
}

int RsaCrypto::hash_data(const string data, string& hash_value, SignLevel level)
{
	//密钥太长，先hash成字串变短
	m_hash.init_hash_type((HashType)level);
	m_hash.add_data(data);
	hash_value = m_hash.hash_result();
	return 0;
}

bool RsaCrypto::get_pub_key_from_file(string pubfile)
{
	// 通过BIO读文件
	BIO* pubBio = BIO_new_file(pubfile.data(), "r");
	// 将bio中的pem数据读出
	if (PEM_read_bio_RSAPublicKey(pubBio, &m_public_key, NULL, NULL) == NULL)
	{
		ERR_print_errors_fp(stdout);
		return false;
	}
	BIO_free(pubBio);
	return true;
}

bool RsaCrypto::get_pri_key_from_file(string prifile)
{
	// 通过bio读文件
	BIO* priBio = BIO_new_file(prifile.data(), "r");
	// 将bio中的pem数据读出
	if (PEM_read_bio_RSAPrivateKey(priBio, &m_private_key, NULL, NULL) == NULL)
	{
		ERR_print_errors_fp(stdout);
		return false;
	}
	BIO_free(priBio);
	return true;
}

int RsaCrypto::toBase64(const char* str, const int len, string& base64_str)
{
	BIO* mem = BIO_new(BIO_s_mem());
	BIO* bs64 = BIO_new(BIO_f_base64());
	// mem添加到bs64中
	bs64 = BIO_push(bs64, mem);
	// 写数据
	BIO_write(bs64, str, len);
	BIO_flush(bs64);
	// 得到内存对象指针
	BUF_MEM* memPtr;
	BIO_get_mem_ptr(bs64, &memPtr);
	// 确保 memPtr->length > 1，否则 base64_str 可能为空
	if (memPtr->length > 1) 
	{
		base64_str = string(memPtr->data, memPtr->length - 1); // -1 是去掉换行符
	}
	else 
	{
		base64_str = ""; // 如果长度不够，设置为空字符串
	}
	//base64_str.assign(memPtr->data, memPtr->length - 1);
	BIO_free_all(bs64);

	return 0;
}

int RsaCrypto::fromBase64(const string base64_str, char*& buf, int& len)
{
	int length = base64_str.size();
	BIO* bs64 = BIO_new(BIO_f_base64());
	BIO* mem = BIO_new_mem_buf(base64_str.data(), length);
	BIO_push(bs64, mem);
	//char* buffer = new char[length];
	if (buf)
		buf = (char*)realloc(buf, length);
	else
		buf = (char*)malloc(length);
	//len = length;
	memset(buf, 0, length);
	BIO_read(bs64, buf, length);
	BIO_free_all(bs64);

	return 0;
}
