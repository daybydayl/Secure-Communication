#pragma once
#include "Codec.h"
#include "Message.pb.h"
#include <iostream>
using namespace std;

//请求数据结构体
struct RequestInfo
{
	int cmd;
	int clientID;
	int serverID;
	string sign;	//对数据签名后密文(私钥加密)
	string data;	//密钥数据(明文数据)
};

//请求序列化类
class RequestCodec : public Codec
{
public:
	//空对象
	RequestCodec();
	//用于解码场景，传入需解的序列化字符串
	RequestCodec(const string encstr);//in
	//用于编码，传入结构
	RequestCodec(const RequestInfo* info);//in
	//给空构造函数准备，解码场景使用
	void initMessage(const string encstr);//in
	//给空构造函数使用，解码场景使用
	void initMessage(const RequestInfo* info);//in
	//重写父类函数 -> 序列化函数，返回序列化的字符串
	int encodeMsg(string& enc_str);//out
	//重写父类函数 -> 反序列化函数，返回结构或类对象
	int decodeMsg(RequestInfo*& dec_info);//out

	//直接将数据序列化，同上
	int dataEncodeMsg(
		const RequestInfo* info,
		string& enc_str);
	//直接数据解序列化，同上
	int dataDecodeMsg(
		const string enc_str,
		RequestInfo*& dec_info);


	~RequestCodec();

private:
	//报文要解码的字符串
	string m_encstr;
	//待序列化的类，通过该类进行序列化操作
	RequestMsg* m_pmsg = new RequestMsg();
};

