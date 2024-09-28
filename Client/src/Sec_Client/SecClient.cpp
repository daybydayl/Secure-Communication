//#pragma comment(lib,"lib_json64.lib")
#pragma comment(lib,"../../lib/JsonDLL.lib")
#include "SecClient.h"
#include <fstream>
using namespace Json;

//test
#include <thread>

SecClient::SecClient()
{
	m_client_name[0] = '\0';
	m_client_id = 0;
	m_server_id = 0;
	m_server_ip[0] = '\0';
	m_server_port = 0;
	m_psecinfo =(SecKeyInfo*)malloc(sizeof(SecKeyInfo));;
}

SecClient::~SecClient()
{
	if (m_psecinfo)
	{
		free(m_psecinfo);
		m_psecinfo = NULL;
	}
}

int SecClient::read_config(const char* jsonFile)
{
	int retcode;
	//解析json文件，读文件 -> value
	ifstream ifs(jsonFile);
	Reader r;//json解析类
	Value root;//json数据类
	if (!r.parse(ifs, root))
	{
		printf("未找到client_cfg.json配置\n");
		return -1;
	}
	//将root中的键值对value值取出
	m_server_id = root["ServerID"].asInt();
	m_client_id = root["ClientID"].asInt();
	strcpy(m_client_name, root["ClientName"].asString().c_str());
	strcpy(m_server_ip, root["ServerIP"].asString().c_str());
	m_server_port = root["Port"].asInt();

	retcode = load_shm_lib();
	if (retcode != 0)
	{
		//log_obj.write_log_at_once(0, 70, "加载共享内存动态库失败，返回值=%d", retcode);
		return retcode;
	}

	return 0;
}
//int num = 0;//长连接测试
int SecClient::seckeyAgree()
{
	int retcode;
	//0.生成秘钥对，将公钥字符串读出
	//RsaCrypto rsa;
	//生成秘钥对
	m_rsa.generate_rsa_key(1024);//这里生成1024位的，有点忘了
	printf("生成的非对称秘钥\n");
	string pubkey_str;
	string sign_pubkey_str;
	m_rsa.pubkey_to_string(pubkey_str);//把公钥转成字串
	//1.初始化待序列化数据
	RequestInfo* preqInfo = new RequestInfo();
	preqInfo->serverID = m_server_id;
	preqInfo->clientID = m_client_id;
	preqInfo->cmd = SEC_KEY_AGREE;
	preqInfo->data = pubkey_str;
	//公钥签名(公钥太长，内部先把公钥hash，对hash值再进行签名(私钥加密))
	m_rsa.rsa_sign(pubkey_str, sign_pubkey_str);//默认level3
	preqInfo->sign = sign_pubkey_str;
	printf("公钥转成字符串为：【%s】\n", preqInfo->data.c_str());
	printf("公钥转为字符串后经私钥签名(加密)后再base64编码为字符串:【%s】\n", preqInfo->sign.c_str());

	//2.protobuf序列化数据待传输 -> 编码(序列化)
	RequestCodec reqCode(preqInfo);
	string enc_str;//序列化后的字串
	reqCode.encodeMsg(enc_str);
	printf("发送:请求结构体经序列化后的字符串为:【%s】\n", enc_str.c_str());

	//3.连接服务器发送请求信息
	//长连接测试
	/*if (num == 0)
	{
		ptcp = new TcpSocket();
		retcode = ptcp->connectToHost(m_server_ip, m_server_port);
		if (retcode != 0)
		{
			printf("连接服务器失败\n");
			return -1;
		}
		printf("连接服务器成功，发送请求，待回复数据\n");
		num = 1;
	}*/
	TcpSocket* ptcp = new TcpSocket();
	retcode = ptcp->connectToHost(m_server_ip, m_server_port);
	if (retcode != 0)
	{
		printf("连接服务器失败\n");
		return -1;
	}
	printf("连接服务器成功，发送请求，待回复数据\n");
	//发送序列化数据
	ptcp->sendMsg(enc_str);
	//等待服务器回复
	string recv_msg = ptcp->recvMsg();

	//4.解析收到数据 -> 解码(反序列化)
	printf("收到:待解序列化字符串为:【%s】\n", recv_msg.c_str());
	ResponCodec resCode(recv_msg);
	ResponInfo* presInfo = new ResponInfo();
	resCode.decodeMsg(presInfo);
	string aes_code;
	if (presInfo->status)
	{
		//5.使用私钥对加密
		retcode = m_rsa.rsa_pri_key_decrypt(presInfo->data, aes_code);
		if (retcode == 0)
		{
			printf("客户端id：%d与服务端id：%d协商的对称密钥为：【%s】\n",
				presInfo->clientID, presInfo->serverID, aes_code.c_str());
		}
	}

	//5.将对称密钥写入共享内存供其他进程使用
	m_psecinfo->client_id = presInfo->clientID;
	m_psecinfo->sec_id = presInfo->seckeyID;
	memset(m_psecinfo->sec_key, 0, MAX_FILE_NAME_LEN);
	memcpy(m_psecinfo->sec_key, aes_code.c_str(), aes_code.size());//写是解密后的秘钥
	//memcpy(m_psecinfo->sec_key, presInfo->data.c_str(), presInfo->data.size());//写的是加密的密文
	m_psecinfo->sec_status = 1;//正常
	m_psecinfo->server_id = presInfo->serverID;
	m_shm.set_sec_info(m_psecinfo);

	delete(preqInfo);
	delete(presInfo);
	// 这是一个短连接, 通信完成, 断开连接
	ptcp->disConnect();
	delete ptcp;
	return 0;
}

int SecClient::seckeyCheck()
{
	int retcode;
	//1.将当前秘钥计算hash
	string hash_val;
	m_rsa.hash_data(m_psecinfo->sec_key, hash_val);
	RequestInfo* preqInfo = new RequestInfo();
	preqInfo->clientID = m_client_id;
	preqInfo->cmd = SEC_KEY_CHECK;
	preqInfo->data = hash_val;
	preqInfo->serverID = m_server_id;
	preqInfo->sign = "";//也不用加密了

	//2.protobuf序列化数据待传输 -> 编码(序列化)
	RequestCodec reqCode(preqInfo);
	string enc_str;//序列化后的字串
	reqCode.encodeMsg(enc_str);

	//3.建立连接，将请求报文发送过去
	TcpSocket* ptcp = new TcpSocket();
	retcode = ptcp->connectToHost(m_server_ip, m_server_port);
	if (retcode != 0)
	{
		printf("连接服务器失败\n");
		return -1;
	}
	printf("连接服务器成功，发送请求，待回复数据\n");
	//发送序列化数据
	ptcp->sendMsg(enc_str);
	//等待服务器回复
	string recv_msg = ptcp->recvMsg();

	//4.接收报文解序列化，验证数据是否正确
	printf("收到:待解序列化字符串为:【%s】\n", recv_msg.c_str());
	ResponCodec resCode(recv_msg);
	ResponInfo* presInfo = new ResponInfo();
	resCode.decodeMsg(presInfo);
	if (presInfo->status)
	{
		//将原先计算好得hash值与得到hash值比较，这里protobuf解序列去除了'\0'，手动加上再比较
		string check_hash_val = presInfo->data + '\0';
		if (hash_val == check_hash_val)
		{
			printf("密钥校验正确\n");
		}
		else
		{
			printf("密钥校验错误\n");
			////更新秘钥，写入共享内存
			//m_psecinfo->client_id = presInfo->clientID;
			//m_psecinfo->sec_id = presInfo->seckeyID;
			//memset(m_psecinfo->sec_key, 0, MAX_FILE_NAME_LEN);
			//memcpy(m_psecinfo->sec_key, presInfo->data.c_str(), presInfo->data.size());//写是解密后的秘钥
			//m_psecinfo->sec_status = 1;//正常
			//m_psecinfo->server_id = presInfo->serverID;
			//m_shm.set_sec_info(m_psecinfo);
		}
	}


	delete(preqInfo);
	delete(presInfo);
	// 这是一个短连接, 通信完成, 断开连接
	ptcp->disConnect();
	delete ptcp;
	return 0;
}

int SecClient::seckeyLogout()
{
	int retcode;
	//1.将当前秘钥状态置为不可用，并更新共享内存
	m_psecinfo->sec_status = 0;
	m_shm.set_sec_info(m_psecinfo);

	//2.protobuf序列化数据待传输 -> 编码(序列化)
	RequestInfo* preqInfo = new RequestInfo();
	preqInfo->serverID = m_server_id;
	preqInfo->clientID = m_client_id;
	preqInfo->cmd = SEC_KEY_LOGOUT;
	preqInfo->data = m_psecinfo->sec_id;
	RequestCodec reqCode(preqInfo);
	string enc_str;//序列化后的字串
	reqCode.encodeMsg(enc_str);
	printf("发送:请求结构体经序列化后的字符串为:【%s】\n", enc_str.c_str());

	//3.连接服务器发送请求信息
	TcpSocket* ptcp = new TcpSocket();
	retcode = ptcp->connectToHost(m_server_ip, m_server_port);
	if (retcode != 0)
	{
		printf("连接服务器失败\n");
		return -1;
	}
	printf("连接服务器成功，发送请求，待回复数据\n");
	//发送序列化数据
	ptcp->sendMsg(enc_str);
	//等待服务器回复
	string recv_msg = ptcp->recvMsg();

	//4.解析收到数据 -> 解码(反序列化)
	printf("收到:待解序列化字符串为:【%s】\n", recv_msg.c_str());
	ResponCodec resCode(recv_msg);
	ResponInfo* presInfo = new ResponInfo();
	resCode.decodeMsg(presInfo);

	//5.收到镜像报文的秘钥状态也是不可用即成功
	if (presInfo->status == 0)
	{
		printf("秘钥注销成功\n");
	}
	else
	{
		printf("秘钥注销失败\n");
	}

	delete preqInfo;
	return 0;
}
