// APP_Server.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#pragma comment(lib,"../../lib/Enc_Dec_DLL.lib")
#include <iostream>
#include "../Enc_Dec_DLL/Enc_Dec_Tool.h"
#include "../include/TcpServer.h"

int main()
{
	int retcode;
	Enc_Dec_Tool ed_tool;
	SecKeyInfo* psec_key = (SecKeyInfo*)malloc(sizeof(SecKeyInfo));
	int sec_num;
	ed_tool.get_all_sec_info(psec_key, sec_num);
	ed_tool.create_aes_by_sec(psec_key[0].sec_id);//完成加解密准备工作
	printf("使用的对称密钥为：%s\n", psec_key[0].sec_key);
	free(psec_key);

	TcpServer* ptcpServer = new TcpServer;
	retcode = ptcpServer->setListen(8888);//监听本地所有地址的该端口
	if (retcode == 0)
		printf("监听本地地址:%d，等待通信客户端的连接请求\n", 8888);

	while (1)
	{
		TcpSocket* ptcp = ptcpServer->acceptConn();
		if (ptcp == NULL)
			continue;
		char ip_str[INET_ADDRSTRLEN] = { 0 };
		inet_ntop(AF_INET, &ptcpServer->m_mp_sktToaddr[ptcp->m_socket].sin_addr, ip_str, sizeof(ip_str));
		printf("通信客户端:%s:%d请求连接成功\n", ip_str, ptcpServer->m_mp_sktToaddr[ptcp->m_socket].sin_port);

		while (1)
		{
			string rec_msg = ptcp->recvMsg();
			string rec_data;
			ed_tool.decryptData(rec_msg, rec_data);
			printf("---------------------------------------------------\n");
			printf("收到消息：%s\n", rec_msg.c_str());
			printf("解码明文：【%s】\n", rec_data.c_str());

			if (rec_data == "discon")
				break;
			string send_data = "server-" + rec_data;
			string send_msg;
			ed_tool.encryptData(send_data, send_msg);
			printf("---------------------------------------------------\n");
			printf("回应数据：%s\n", send_data.c_str());
			printf("加密后密文：%s\n", send_msg.c_str());
			ptcp->sendMsg(send_msg);
		}

		//结束通信
		printf("通信客户端:%s:%d断开连接\n", ip_str, ptcpServer->m_mp_sktToaddr[ptcp->m_socket].sin_port);
		printf("重新等待客户端连接请求\n");
		ptcp->disConnect();
		delete ptcp;
	}

	delete ptcpServer;

    std::cout << "Hello World!\n";
}


