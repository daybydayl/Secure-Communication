#include "../../include/openssl/Hash.h"

Hash::Hash()
{
	
}

Hash::~Hash()
{
}

int Hash::init_hash_type(HashType type)
{
	//初始化
	m_type = type;
	switch (m_type)
	{
	case HashType::T_MD5:
		md5_init();
		break;
	case HashType::T_SHA1:
		sha1_init();
		break;
	case HashType::T_SHA224:
		sha224_init();
		break;
	case HashType::T_SHA256:
		sha256_init();
		break;
	case HashType::T_SHA384:
		sha384_init();
		break;
	case HashType::T_SHA512:
		sha512_init();
		break;
	default:
		md5_init();
		break;
	}
	return 0;
}

int Hash::add_data(std::string data)
{
	int retcode = 0;
	switch (m_type)
	{
	case HashType::T_MD5:
		retcode = md5_add_data(data.data());
		break;
	case HashType::T_SHA1:
		retcode = sha1_add_data(data.data());
		break;
	case HashType::T_SHA224:
		retcode = sha224_add_data(data.data());
		break;
	case HashType::T_SHA256:
		retcode = sha256_add_data(data.data());
		break;
	case HashType::T_SHA384:
		retcode = sha384_add_data(data.data());
		break;
	case HashType::T_SHA512:
		retcode = sha512_add_data(data.data());
		break;
	default:
		retcode = md5_add_data(data.data());
		break;
	}
	return retcode;
}

std::string Hash::hash_result()
{
	std::string result = std::string();
	switch (m_type)
	{
	case HashType::T_MD5:
		result = md5_result();
		break;
	case HashType::T_SHA1:
		result = sha1_result();
		break;
	case HashType::T_SHA224:
		result = sha224_result();
		break;
	case HashType::T_SHA256:
		result = sha256_result();
		break;
	case HashType::T_SHA384:
		result = sha384_result();
		break;
	case HashType::T_SHA512:
		result = sha512_result();
		break;
	default:
		result = md5_result();
		break;
	}
	return result;
}

std::string Hash::md5_result()
{
	//计算结果
	unsigned char md[MD5_DIGEST_LENGTH];
	char res[MD5_DIGEST_LENGTH * 2 + 1];
	//得到ascci码hash值
	MD5_Final(md, &m_md5);
	//转成16进制hash值
	for (int i = 0; i < MD5_DIGEST_LENGTH; ++i)
	{
		sprintf(&res[i * 2], "%02x", md[i]);
	}
	return std::string(res, MD5_DIGEST_LENGTH * 2 + 1);
}

std::string Hash::sha1_result()
{
	unsigned char md[SHA_DIGEST_LENGTH];
	char res[SHA_DIGEST_LENGTH * 2 + 1];
	SHA1_Final(md, &m_sha1);
	for (int i = 0; i < SHA_DIGEST_LENGTH; ++i)
	{
		sprintf(&res[i * 2], "%02x", md[i]);
	}
	return std::string(res, SHA_DIGEST_LENGTH * 2 + 1);
}

std::string Hash::sha224_result()
{
	unsigned char md[SHA224_DIGEST_LENGTH];
	char res[SHA224_DIGEST_LENGTH * 2 + 1];
	SHA224_Final(md, &m_sha224);
	for (int i = 0; i < SHA224_DIGEST_LENGTH; ++i)
	{
		sprintf(&res[i * 2], "%02x", md[i]);
	}
	return std::string(res, SHA224_DIGEST_LENGTH * 2 + 1);
}

std::string Hash::sha256_result()
{
	unsigned char md[SHA256_DIGEST_LENGTH];
	char res[SHA256_DIGEST_LENGTH * 2 + 1];
	SHA256_Final(md, &m_sha256);
	for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
	{
		sprintf(&res[i * 2], "%02x", md[i]);
	}
	return std::string(res, SHA256_DIGEST_LENGTH * 2 + 1);
}

std::string Hash::sha384_result()
{
	unsigned char md[SHA384_DIGEST_LENGTH];
	char res[SHA384_DIGEST_LENGTH * 2 + 1];
	SHA384_Final(md, &m_sha384);
	for (int i = 0; i < SHA384_DIGEST_LENGTH; ++i)
	{
		sprintf(&res[i * 2], "%02x", md[i]);
	}
	return std::string(res, SHA384_DIGEST_LENGTH * 2 + 1);
}

std::string Hash::sha512_result()
{
	unsigned char md[SHA512_DIGEST_LENGTH];
	char res[SHA512_DIGEST_LENGTH * 2 + 1];
	SHA512_Final(md, &m_sha512);
	for (int i = 0; i < SHA512_DIGEST_LENGTH; ++i)
	{
		sprintf(&res[i * 2], "%02x", md[i]);
	}
	return std::string(res, SHA512_DIGEST_LENGTH * 2 + 1);
}
