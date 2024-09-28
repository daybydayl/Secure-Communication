#include <iostream>
#include "SecServer.h"

int main()
{

    int retcode;
    SecServer server_obj;
    //读服务端配置文件,exe找文件
    string cfg_path = string("../cfg/server_cfg.json");
    retcode = server_obj.read_config(cfg_path.c_str());
    if(retcode == 0)
        printf("服务端配置读取完成，共享内存装载成功1\n");
    else
    {
        //debug找文件
        cfg_path = string("../../cfg/server_cfg.json");
        retcode = server_obj.read_config(cfg_path.c_str());
        if(retcode == 0)
            printf("服务端配置读取完成，共享内存装载成功2\n");
        else
            return -1;
    }

    server_obj.startServer();

    return 0;
}
