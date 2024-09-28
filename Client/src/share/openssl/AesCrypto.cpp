#include "../../include/openssl/AesCrypto.h"

AesCrypto::AesCrypto(string key)
{
	int key_len = key.size();
	if (key_len > 0 && key_len <= 16)
		key_len = 16 * 8;
	else if (key_len > 16 && key_len <= 24)
		key_len = 24 * 8;
	else if(key_len > 24 && key_len <= 32)
		key_len = 32 * 8;
	const unsigned char* aesKey = (const unsigned char*)key.data();
	AES_set_encrypt_key(aesKey, key_len, &m_enc_key);
	AES_set_decrypt_key(aesKey, key_len, &m_dec_key);
	m_key = key;
}

AesCrypto::~AesCrypto()
{
}

int AesCrypto::aes_cbc_encrypt(const string data, string& enc_data)
{
	return aes_crypto(data, enc_data, AES_ENCRYPT);
}

int AesCrypto::aes_cbc_decrypt(const string enc_data, string& data)
{
	return aes_crypto(enc_data, data, AES_DECRYPT);
}

int AesCrypto::aes_crypto(const string in, string& out, int crypto)
{
	AES_KEY* key = crypto == AES_ENCRYPT ? &m_enc_key : &m_dec_key;

	unsigned char ivec[AES_BLOCK_SIZE];
	int length = in.size() + 1;	// +1 是添加\0的长度
	if (length % 16)
	{
		length = (length / 16 + 1) * 16;
	}
	char* result = new char[length];
	generate_ivec(ivec);

	AES_cbc_encrypt((const unsigned char*)in.c_str(),
		(unsigned char*)result, length, key, ivec, crypto);

	out = string(result);
	delete[]result;
	return 0;
}

int AesCrypto::generate_ivec(unsigned char* ivec)
{
	//根据使用者的密钥来生成初始向量
	for (int i = 0; i < AES_BLOCK_SIZE; ++i)
	{
		ivec[i] = 'a' + m_key.size() + i;
	}
	return 0;
}
