#include "Enc_Dec_Tool.h"

Enc_Dec_Tool::Enc_Dec_Tool()
{
	load_shm_lib();
}

Enc_Dec_Tool::~Enc_Dec_Tool()
{
	if (m_pkey_info)
	{
		delete m_pkey_info;
		m_pkey_info = NULL;
	}
	if (m_paes_obj)
	{
		delete m_paes_obj;
		m_paes_obj = NULL;
	}
	//unload_shm_lib();
}

int Enc_Dec_Tool::get_all_sec_info(SecKeyInfo*& psec_infos, int& sec_num)
{
	return m_shmem.get_all_sec_info(psec_infos, sec_num);
}

int Enc_Dec_Tool::create_aes_by_sec(const int sec_id, const int server_id, const int client_id)
{
	int retcode;
	m_pkey_info = new SecKeyInfo();
	retcode = m_shmem.get_sec_info_by_condition(sec_id, server_id, client_id, m_pkey_info);
	if (retcode == 0 && m_pkey_info->sec_status)
	{
		m_paes_obj = new AesCrypto(m_pkey_info->sec_key);
	}
	return retcode;
}

int Enc_Dec_Tool::encryptData(const string data, string& enc_data)
{
	int retcode = -1;;
	if(m_paes_obj)
		retcode = m_paes_obj->aes_cbc_encrypt(data, enc_data);
	return retcode;
}

int Enc_Dec_Tool::decryptData(const string dec_data, string& data)
{
	int retcode = -1;
	if(m_paes_obj)
		retcode = m_paes_obj->aes_cbc_decrypt(dec_data, data);
	return retcode;
}
