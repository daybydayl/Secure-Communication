#include <iostream>
#include "SecClient.h"

int option();
int main() {
    int retcode;
    SecClient client_obj;
    //读客户端配置文件，exe找文件
    string cfg_path = string("../cfg/client_cfg.json");
    retcode = client_obj.read_config(cfg_path.c_str());
    if (retcode == 0)
        printf("客户端配置读取完成，共享内存装载成功\n");
    else
    {
        //debug找文件
        cfg_path = string("../../cfg/client_cfg.json");
        retcode = client_obj.read_config(cfg_path.c_str());
        if (retcode == 0)
            printf("客户端配置读取完成，共享内存装载成功\n");
        else
            return -1;
    }
        
    //做图形界面或console这里应搞个switch选
    bool task = true;
    while (task)
    {
        int sel = option();
        switch (sel)
        {
        case SEC_KEY_AGREE:
            // 秘钥协商
            client_obj.seckeyAgree();
            break;
        case SEC_KEY_CHECK:
            // 秘钥校验
            client_obj.seckeyCheck();
            break;
        case SEC_KEY_LOGOUT:
            // 秘钥注销
            client_obj.seckeyLogout();
            break;
        default:
            task = false;
            break;
        }
    }
    cout << "客户端退出" << endl;

    return 0;
}

int option()
{
    int nSel = -1;
    printf("\n  /*************************************************************/");
    printf("\n  /*************************************************************/");
    printf("\n  /*     1.密钥协商                                             */");
    printf("\n  /*     2.密钥校验                                             */");
    printf("\n  /*     3.密钥注销                                             */");
    printf("\n  /*     0.退出系统                                             */");
    printf("\n  /*************************************************************/");
    printf("\n  /*************************************************************/");
    printf("\n\n  选择:");

    scanf("%d", &nSel);
    while (getchar() != '\n');

    return nSel;
}