#include "RequestCodec.h"
#include <iostream>

RequestCodec::RequestCodec()
{
}

RequestCodec::RequestCodec(const string encstr)
{
	initMessage(encstr);
}

RequestCodec::RequestCodec(const RequestInfo* info)
{
	initMessage(info);
}

void RequestCodec::initMessage(const string encstr)
{
	m_encstr = encstr;
}

void RequestCodec::initMessage(const RequestInfo* info)
{
	m_pmsg->set_cmdtype(info->cmd);
	m_pmsg->set_clientid(info->clientID);
	m_pmsg->set_serverid(info->serverID);
	m_pmsg->set_sign(info->sign);
	m_pmsg->set_data(info->data);
}

int RequestCodec::encodeMsg(string& enc_str)
{
	m_pmsg->SerializeToString(&enc_str);
	return 0;
}

int RequestCodec::decodeMsg(RequestInfo*& dec_info)
{
	m_pmsg->ParseFromString(m_encstr);
	dec_info->cmd = m_pmsg->cmdtype();
	dec_info->clientID = m_pmsg->clientid();
	dec_info->serverID = m_pmsg->serverid();
	dec_info->sign = m_pmsg->sign();
	dec_info->data = m_pmsg->data();
	return 0;
}

int RequestCodec::dataEncodeMsg(const RequestInfo* info, string& enc_str)
{
	//直接做上面两部，请求和接收两个类有点冗余，后面考虑合并
	initMessage(info);
	encodeMsg(enc_str);
	return 0;
}

int RequestCodec::dataDecodeMsg(const string enc_str, RequestInfo*& dec_info)
{
	initMessage(enc_str);
	decodeMsg(dec_info);
	return 0;
}

RequestCodec::~RequestCodec()
{
}
