#include "ResponCodec.h"
#include <iostream>

ResponCodec::ResponCodec()
{
}

ResponCodec::ResponCodec(const string enc)
{
	initMessage(enc);
}

ResponCodec::ResponCodec(const ResponInfo* info)
{
	initMessage(info);
}

void ResponCodec::initMessage(const string enc)
{
	m_encstr = enc;
}

void ResponCodec::initMessage(const ResponInfo* info)
{
	m_pmsg->set_status(info->status);
	m_pmsg->set_seckeyid(info->seckeyID);
	m_pmsg->set_clientid(info->clientID);
	m_pmsg->set_serverid(info->serverID);
	m_pmsg->set_data(info->data);
}

int ResponCodec::encodeMsg(string& enc_str)
{
	m_pmsg->SerializeToString(&enc_str);
	return 0;
}

int ResponCodec::decodeMsg(ResponInfo*& dec_info)
{
	m_pmsg->ParseFromString(m_encstr);
	dec_info->status = m_pmsg->status();
	dec_info->seckeyID = m_pmsg->seckeyid();
	dec_info->clientID = m_pmsg->clientid();
	dec_info->serverID = m_pmsg->serverid();
	dec_info->data = m_pmsg->data();
	return 0;
}

int ResponCodec::dataEncodeMsg(const ResponInfo* info, string& enc_str)
{
	//直接做上面两部，请求和接收两个类有点冗余，后面考虑合并
	initMessage(info);
	encodeMsg(enc_str);
	return 0;
}

int ResponCodec::dataDecodeMsg(const string enc_str, ResponInfo*& dec_info)
{
	initMessage(enc_str);
	decodeMsg(dec_info);
	return 0;
}

ResponCodec::~ResponCodec()
{
}
