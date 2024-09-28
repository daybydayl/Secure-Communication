#pragma comment(lib,"../../lib/Enc_Dec_DLL.lib")
#include <iostream>
#include "Enc_Dec_Tool.h"

int main()
{
    Enc_Dec_Tool ed_tool(1, 1);
    printf("共享内存拿的秘钥信息为:服务器id=%d,客户端id=%d,秘钥状态=%d,秘钥id=%d,密钥=【%s】\n", 
        ed_tool.m_pkey_info->server_id,
        ed_tool.m_pkey_info->client_id,
        ed_tool.m_pkey_info->sec_status,
        ed_tool.m_pkey_info->sec_id,
        ed_tool.m_pkey_info->sec_key);
    //测试数据加密
    string enc_data = "测试数据";
    string data;
    ed_tool.decryptData(enc_data, data);
    printf("用公钥加密数据后的密文为：%s\n", data.c_str());

    std::cout << "Hello World!\n";
}
