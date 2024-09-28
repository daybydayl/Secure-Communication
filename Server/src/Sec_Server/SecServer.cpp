//#pragma comment(lib,"lib_json64.lib")
#pragma comment(lib,"../../lib/JsonDLL.lib")//只在WIN上有效,可以放这也可以放项目属性输入的附加依赖项，linux上编译会警告忽略
#include "SecServer.h"
#include <fstream>
using namespace Json;

SecServer::SecServer()
{
	m_server_id = 0;
	m_seckey_id = 1;
	m_port = 8989;
	m_server_name[0] = '\0';
	m_server = NULL;
	m_mp_thr_skt.clear();

}

SecServer::~SecServer()
{
}

int SecServer::read_config(const char* jsonFile)
{
	int retcode;
	// 解析json文件, 读文件 -> Value
	ifstream ifs(jsonFile);
	Reader r;
	Value root;
	if (!r.parse(ifs, root))
	{
		printf("未找到server_cfg.json配置\n");
		return -1;
	}
	// 将root中的键值对value值取出
	m_port = root["Port"].asInt();
	m_server_id = root["ServerID"].asInt();
	//// 数据库相关的信息
	//m_dbUser = root["UserDB"].asString();
	//m_dbPwd = root["PwdDB"].asString();
	//m_dbConnStr = root["ConnStrDB"].asString();
	//// 实例化一个连接oracle数据的对象
	//m_occi.connectDB(m_dbUser, m_dbPwd, m_dbConnStr);

	// 实例化共享内存对象
	retcode = load_shm_lib();
	if (retcode != 0)
	{
		//log_obj.write_log_at_once(0, 70, "加载共享内存动态库失败，返回值=%d", retcode);
		return retcode;
	}

	return 0;
}

void SecServer::startServer()
{
	int retcode;
	m_server = new TcpServer;
	retcode = m_server->setListen(m_port);//监听本地所有地址的该端口
	if(retcode == 0)
		printf("监听本地地址:%d，等待客户端的连接请求\n",m_port);
	while (1)
	{
//#ifdef _WIN32
//		//初始化 Winsock 库，用于网络编程，监听通信
//		WSADATA wsaData;
//		WSAStartup(MAKEWORD(2, 2), &wsaData);
//#endif
		TcpSocket* ptcp = m_server->acceptConn();
		if (ptcp == NULL)
			continue;
		char ip_str[INET_ADDRSTRLEN] = { 0 };
		inet_ntop(AF_INET, &m_server->m_mp_sktToaddr[ptcp->m_socket].sin_addr, ip_str, sizeof(ip_str));
		printf("客户端:%s:%d请求连接成功\n", ip_str, m_server->m_mp_sktToaddr[ptcp->m_socket].sin_port);

		//通信，这里线程用C++11对跨平台友好
		// 这个回调可以是类的静态函数, 类的友元函数, 普通的函数
		// 友元的类的朋友, 但是不属于这个类
		// 友元函数可以访问当前类的私有成员
		thread tid = thread(work_thr, this);
		m_mp_thr_skt.insert(make_pair(tid.get_id(), ptcp));//关联关系存入map
		tid.detach();

//#ifdef _WIN32
//		Sleep(1);
//#else
//		usleep(1000);
//#endif // _WIN32
	}

	delete m_server;
}

void* SecServer::work_thr(void* arg)
{
	int retcode;
	//适应单核CPU,避免进程一直持有CPU
#ifdef _WIN32
	Sleep(1000);
#else
	sleep(1);
#endif // _WIN32

	//取出传入的arg参数，并从map取出通信套接字
	SecServer* psec_server = (SecServer*)arg;
	TcpSocket* ptcp = psec_server->m_mp_thr_skt[std::this_thread::get_id()];

	//1.接收客户端数据 -> 编码
	string req_msg = ptcp->recvMsg();
	//2.反序列化 -> 得到原始数据 RequestInfo类型
	RequestCodec* preqCode = new RequestCodec(req_msg);
	RequestInfo* preqInfo = new RequestInfo();
	preqCode->decodeMsg(preqInfo);
	printf("接收:待解序列化字符串为：【%s】\n", req_msg.c_str());
	//3.取出数据
	//看请求数据类型
	printf("解序后报文类型为：%d\n", preqInfo->cmd);
	string respon_data;
	ResponInfo resinfo;
	switch (preqInfo->cmd)
	{
	case SEC_KEY_AGREE:
		retcode = psec_server->seckeyAgree(preqInfo, respon_data);
		if (retcode != 0)
		{
			//失败情况
		}
		break;
	case SEC_KEY_CHECK:
		retcode = psec_server->seckeyCheck(preqInfo, respon_data);
		if (retcode != 0)
		{
			//失败情况
		}
		break;
	case SEC_KEY_LOGOUT:
		break;
	case SEC_KEY_LOOK:
		break;
	default:
		break;
	}

	//4.短连接情况，发完响应数据断开stk并关联map擦除
	printf("发送:响应结构体序列化后的字符串为:【%s】\n", respon_data.c_str());
	ptcp->sendMsg(respon_data);
	ptcp->disConnect();
	psec_server->m_mp_thr_skt.erase(std::this_thread::get_id());
	// 释放资源
	delete preqCode;
	delete preqInfo;
	delete ptcp;

	//int flag = 1;//长连接测试
	//do
	//{
	//	//1.接收客户端数据 -> 编码
	//	string req_msg = ptcp->recvMsg();
	//	//2.反序列化 -> 得到原始数据 RequestInfo类型
	//	RequestCodec* preqCode = new RequestCodec(req_msg);
	//	RequestInfo* preqInfo = new RequestInfo();
	//	preqCode->decodeMsg(preqInfo);
	//	printf("接收:待解序列化字符串为：【%s】\n", req_msg.c_str());
	//	//3.取出数据
	//	//看请求数据类型
	//	printf("解序后报文类型为：%d\n", preqInfo->cmd);
	//	string respon_data;
	//	ResponInfo resinfo;
	//	switch (preqInfo->cmd)
	//	{
	//	case SEC_KEY_AGREE:
	//		retcode = psec_server->seckeyAgree(preqInfo, respon_data);
	//		if (retcode != 0)
	//		{
	//			//失败情况
	//		}
	//		break;
	//	case SEC_KEY_CHECK:
	//		retcode = psec_server->seckeyCheck(preqInfo, respon_data);
	//		if (retcode != 0)
	//		{
	//			//失败情况
	//		}
	//		break;
	//	case SEC_KEY_LOGOUT:
	//		break;
	//	case SEC_KEY_LOOK:
	//		break;
	//	default:
	//		flag = 0;
	//		break;
	//	}
	//	//4.短连接情况，发完响应数据断开stk并关联map擦除
	//	printf("发送:响应结构体序列化后的字符串为:【%s】\n", respon_data.c_str());
	//	ptcp->sendMsg(respon_data);
	//	
	//	delete preqCode;
	//	delete preqInfo;
	//} while (flag);
	//ptcp->disConnect();
	//psec_server->m_mp_thr_skt.erase(std::this_thread::get_id());
	//// 释放资源
	//delete ptcp;
	
	return NULL;
}

int SecServer::seckeyAgree(const RequestInfo* reqInfo, string& respon)
{
	//公加私解为加密，私加公解为签名
	int retcode;
	//RsaCrypto rsa;
	ResponInfo* presInfo = new ResponInfo;
	m_rsa.string_to_pubkey(reqInfo->data);//将接收的公钥串转成rsa类(相当于创建公钥)
	//1.将请求结构中的数据验证(公钥解密)成为公钥(内部先把data进行hash,再把hash结果进行验证)
	retcode = m_rsa.rsa_verify(reqInfo->sign, reqInfo->data);
	if (retcode == 0)
	{//校验成功
		printf("秘钥字串签名校验成功\n");
		presInfo->status = true;//响应数据正确性
		//1.1生成随机字符串(对称秘钥)
		//对称加密的秘钥, 使用对称加密算法 aes, 秘钥长度: 16, 24, 32byte
		string aes_key;
		retcode = get_rand_key(Len16, aes_key);
		printf("生成随机对称密钥为:【%s】\n", aes_key.c_str());
		//1.2用公钥加密对称秘钥
		string sec_key;
		retcode = m_rsa.rsa_pub_key_encrypt(aes_key, sec_key);
		printf("对称秘钥经公钥加密后再有base64编码后的字符串为:【%s】\n", sec_key.c_str());
		//1.3初始化回复数据
		presInfo->clientID = reqInfo->clientID;
		presInfo->data = sec_key;
		presInfo->serverID = m_server_id;
		presInfo->seckeyID = m_seckey_id++;

		//1.4将对称密钥写入共享内存供其他进程使用
		SecKeyInfo* psecinfo = (SecKeyInfo*)malloc(sizeof(SecKeyInfo));
		psecinfo->client_id = presInfo->clientID;
		psecinfo->sec_id = presInfo->seckeyID;
		memset(psecinfo->sec_key, 0, MAX_FILE_NAME_LEN);
		memcpy(psecinfo->sec_key, aes_key.c_str(), aes_key.size());//对称秘钥
		//memcpy(psecinfo->sec_key, presInfo->data.c_str(), presInfo->data.size());//加密后的秘钥存入
		psecinfo->sec_status = 1;//正常
		psecinfo->server_id = presInfo->serverID;
		retcode = m_shm.set_sec_info(psecinfo);
		free(psecinfo);

		//1.5如果加数据库的话，这里OCCI把对称密钥信息写入数据库

	}
	else
	{//校验失败
		printf("秘钥校验失败\n");
		presInfo->status = false;
		presInfo->clientID = reqInfo->clientID;
		presInfo->data = "";
		presInfo->serverID = m_server_id;
		presInfo->seckeyID = -1;
	}

	//2.ResponInfo结构填入经序列化后返回
	ResponCodec resCode(presInfo);
	resCode.encodeMsg(respon);//将结构数据序列化为字符串待发送

	delete presInfo;
	return 0;
}

int SecServer::seckeyCheck(const RequestInfo* reqInfo, string& respon)
{
	int retcode;
	//1.查找对应客户端密钥
	SecKeyInfo* psec_key = (SecKeyInfo*)malloc(sizeof(SecKeyInfo));
	retcode = m_shm.get_sec_info_by_condition(-1, reqInfo->serverID, reqInfo->clientID, psec_key);
	if (retcode != 0)
		return -1;

	//2.hash收到的秘钥数据
	string hash_val;
	m_rsa.hash_data(psec_key->sec_key, hash_val);

	//3.组装响应报文结构体进行序列化
	ResponInfo* presInfo = new ResponInfo;
	presInfo->clientID = psec_key->client_id;
	presInfo->data = hash_val;
	presInfo->seckeyID = psec_key->sec_id;
	presInfo->serverID = psec_key->server_id;
	presInfo->status = psec_key->sec_status;

	ResponCodec resCode(presInfo);
	resCode.encodeMsg(respon);//将结构数据序列化为字符串待发送

	free(psec_key);
	delete presInfo;
	return 0;
}

int SecServer::seckeyLogout(const RequestInfo* reqInfo, string& respon)
{
	//1.先找到对应共享内存秘钥
	SecKeyInfo* psec_key = (SecKeyInfo*)malloc(sizeof(SecKeyInfo));
	//传过来的数据是sec_id
	m_shm.get_sec_info_by_condition(atoi(reqInfo->data.c_str()), reqInfo->serverID, reqInfo->clientID, psec_key);

	//2.将请求的秘钥状态填入再写回共享内存
	psec_key->sec_status = 0;
	m_shm.set_sec_info(psec_key);

	//3.组装镜像响应报文结构体进行序列化
	ResponInfo* presInfo = new ResponInfo;
	presInfo->clientID = reqInfo->clientID;
	presInfo->data = reqInfo->data;
	presInfo->seckeyID = psec_key->sec_id;
	presInfo->serverID = psec_key->server_id;
	presInfo->status = psec_key->sec_status;

	ResponCodec resCode(presInfo);
	resCode.encodeMsg(respon);//将结构数据序列化为字符串待发送

	free(psec_key);
	delete presInfo;
	return 0;
}

//字符串中包含: a-z, A-Z, 0-9, 特殊字符
int SecServer::get_rand_key(const KeyLen len, string& rand_key)
{
	//设置随机数数种子 => 根据时间
	srand(time(NULL));
	int flag = 0;
	char cs[32] = "~!@#$%^&*()_+}{|\';[]";
	for (int i = 0; i < len; ++i)
	{
		flag = rand() % 4;	//4中字符类型
		switch (flag)
		{
		case 0:	//a-z
			rand_key.append(1, 'a' + rand() % 26);
			break;
		case 1: //A-Z
			rand_key.append(1, 'A' + rand() % 26);
			break;
		case 2: //0-9
			rand_key.append(1, '0' + rand() % 10);
			break;
		case 3: //特殊字符
			rand_key.append(1, cs[rand() % strlen(cs)]);
			break;
		default:
			break;
		}
	}
	
	return 0;
}
