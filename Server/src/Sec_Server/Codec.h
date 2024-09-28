#pragma once
#include <iostream>

//编解码的父类(基类)
class Codec
{
public:
	Codec();
	//编码虚函数
	virtual int encodeMsg(std::string& enc_str);
	//解码虚函数
	virtual int decodeMsg(void*& dec_info);

	//多态时，应用虚析构
	virtual ~Codec();
};

