#pragma once
#include "Codec.h"
#include <iostream>
#include "Message.pb.h"
using namespace std;

//回应数据结构
struct ResponInfo
{
	int status;
	int seckeyID;
	int clientID;
	int serverID;
	string data;
};

//回应编解码类，与请求编解码类一致
class ResponCodec : public Codec
{
public:
	ResponCodec();
	//用于解码场景，传入需解的序列化字符串
	ResponCodec(const string enc);//in
	//用于编码，传入结构
	ResponCodec(const ResponInfo* info);//in
	//给空构造函数准备，解码场景使用
	void initMessage(const string enc);//in
	//给空构造函数使用，解码场景使用
	void initMessage(const ResponInfo* info);//in
	//重写父类函数 -> 序列化函数，返回序列化的字符串
	int encodeMsg(string &enc_str);//out
	//重写父类函数 -> 反序列化函数，返回结构或类对象
	int decodeMsg(ResponInfo*& dec_info);//out

	//直接将数据序列化，同上
	int dataEncodeMsg(
		const ResponInfo* info,
		string& enc_str);
	//直接数据解序列化，同上
	int dataDecodeMsg(
		const string enc_str,
		ResponInfo* dec_info);

	~ResponCodec();

private:
	string m_encstr;
	RespondMsg* m_pmsg = new RespondMsg();
};

