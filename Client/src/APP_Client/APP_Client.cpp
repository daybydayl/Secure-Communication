// APP_Client.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#pragma comment(lib,"../../lib/Enc_Dec_DLL.lib")
#include <iostream>
#include "../Enc_Dec_DLL/Enc_Dec_Tool.h"
#include "../include/TcpSocket.h"

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


    TcpSocket* ptcp = new TcpSocket();
	retcode = ptcp->connectToHost("192.168.8.134", 8888);//用自己真实ip测试
	if (retcode != 0)
	{
		printf("连接服务器失败\n");
		return -1;
	}
	printf("连接服务器成功\n");
	string send_data;
	while (1)
	{
		printf("---------------------------------------------------\n");
		printf("发送明文：");
		std::cin >> send_data;
		string enc_msg;
		ed_tool.encryptData(send_data, enc_msg);
		printf("加密后密文：%s\n", enc_msg.c_str());

		if (send_data == "discon")
		{
			ptcp->sendMsg(enc_msg);
			break;
		}
		//printf("客户端：%s\n", enc_str.c_str());
		ptcp->sendMsg(enc_msg);
		string rec_msg = ptcp->recvMsg();
		string rec_data;
		ed_tool.decryptData(rec_msg, rec_data);
		printf("---------------------------------------------------\n");
		printf("收到消息：%s\n", rec_msg.c_str());
		printf("解码明文：【%s】\n", rec_data.c_str());
	}


	printf("与服务端断开连接\n");
	ptcp->disConnect();
    delete ptcp;
    std::cout << "Hello World!\n";
}

