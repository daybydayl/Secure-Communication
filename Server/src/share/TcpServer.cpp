#include "../include/TcpServer.h"

TcpServer::TcpServer()
{
//#ifdef _WIN32
//    //初始化 Winsock 库，用于网络编程，监听通信
//    WSADATA wsaData;
//    WSAStartup(MAKEWORD(2, 2), &wsaData);
//#endif
}

TcpServer::~TcpServer()
{
#ifdef _WIN32
	// 清理 Winsock库
	WSACleanup();
#endif
}

int TcpServer::setListen(unsigned short port)
{
    int ret = 0;
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 创建监听的套接字
#ifdef _WIN32

    //window情况下使用接口需要先初始化一下
    //初始化 Winsock 库，用于网络编程，监听通信
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    //ipv4,TCP协议类型,0自动选择与协议类型相匹配的默认协议
    m_lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_lfd == INVALID_SOCKET) {
        ret = WSAGetLastError();
        return ret;
    }

    int on = 1;
    // 设置端口复用：可以让服务器在关闭后快速重启，并避免端口被暂时占用的情况，还能方便并行开发和测试。
    //SOL_SOCKET：表示操作影响套接字本身
    //SO_REUSEADDR：选项名，表示允许套接字绑定到已在使用的地址和端口
    //on：表示启用这选项
    ret = setsockopt(m_lfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on));
    if (ret == SOCKET_ERROR) {
        ret = WSAGetLastError();
        return ret;
    }

    // 监听的套接字绑定本地IP和端口
    ret = bind(m_lfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if (ret == SOCKET_ERROR) {
        ret = WSAGetLastError();
        return ret;
    }

    //SOMAXCONN：指定监听队列的最大长度。这个值通常是系统定义的最大值。
    ret = listen(m_lfd, SOMAXCONN);
    if (ret == SOCKET_ERROR) {
        ret = WSAGetLastError();
        return ret;
    }

#else
    m_lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_lfd == -1) {
        ret = errno;
        return ret;
    }

    int on = 1;
    // 设置端口复用
    ret = setsockopt(m_lfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (ret == -1) {
        ret = errno;
        return ret;
    }

    // 监听的套接字绑定本地IP和端口
    ret = bind(m_lfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if (ret == -1) {
        ret = errno;
        return ret;
    }

    ret = listen(m_lfd, SOMAXCONN);
    if (ret == -1) {
        ret = errno;
        return ret;
    }
#endif

    return ret;
}

/*
* acceptConn - 接受连接
* @wait_seconds: 等待超时秒数
* 成功返回新的TcpSocket对象，失败返回NULL
*/
TcpSocket* TcpServer::acceptConn(int wait_seconds)
{
    /*跨平台win用select，linux用epoll，epoll高并发更好*/
    int ret;
    // 如果需要等待
#ifdef _WIN32
    //window情况下使用接口需要先初始化一下
	//初始化 Winsock 库，用于网络编程，监听通信
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (wait_seconds > 0) {
        // Windows 平台使用 select
        fd_set accept_fdset;
        struct timeval timeout;
        FD_ZERO(&accept_fdset);
        FD_SET(m_lfd, &accept_fdset);
        timeout.tv_sec = wait_seconds;
        timeout.tv_usec = 0;
        int errr_no = 0;
        do {
            ret = select(0, &accept_fdset, NULL, NULL, &timeout);
            errr_no = WSAGetLastError();
        } while (ret < 0 && WSAGetLastError() == WSAEINTR);

        if (ret <= 0) {
            // 没有事件发生或者出错，直接返回 NULL
            return NULL;
        }
    }

    // 一旦检测到事件发生，表示对等方完成了三次握手，客户端有新连接建立
    // 此时调用 accept 将不会阻塞
    struct sockaddr_in addrCli;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    int connfd;
    // Windows 平台调用 accept
    connfd = accept(m_lfd, (struct sockaddr*)&addrCli, &addrlen);
    if (connfd == INVALID_SOCKET) {
        return NULL;
    }
#else
    if (wait_seconds > 0) {
        // Linux 平台使用 epoll
        int epfd = epoll_create(1);//默认传1，系统会分配红黑树节点数，epfd是红黑树的根
        struct epoll_event ev, events[1];
        ev.events = EPOLLIN;//文件描述可读
        ev.data.fd = m_lfd;//关心的文件描述符
        //epfd：epoll实例
        //EPOLL_CTL_ADD：将m_lfd加入到epfd
        //ev：事件结构体
        epoll_ctl(epfd, EPOLL_CTL_ADD, m_lfd, &ev);
        struct epoll_event evlist;//存储返回的事件
        //在epfd中等待最多1个事件，最多等wait_seconds * 1000超时时间，等到的事件存放至evlist结构体
        ret = epoll_wait(epfd, &evlist, 1, wait_seconds * 1000);
        close(epfd);

        if (ret <= 0) {
            // 没有事件发生或者出错，直接返回 NULL
            return NULL;
        }
    }
    // 一旦检测到事件发生，表示对等方完成了三次握手，客户端有新连接建立
    // 此时调用 accept 将不会阻塞
    struct sockaddr_in addrCli;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    int connfd;
    // Linux 平台调用 accept
    connfd = accept(m_lfd, (struct sockaddr*)&addrCli, &addrlen);
    if (connfd == -1) {
        return NULL;
    }
#endif
    //将终端通信套接字和地址存入map
    m_mp_sktToaddr.insert(make_pair(connfd, addrCli));
    // 返回新的 TcpSocket 对象
    return new TcpSocket(connfd);

/*跨平台都都用select方式*/
//    int ret;
//    if (wait_seconds > 0) {
//        fd_set accept_fdset;
//        struct timeval timeout;
//        FD_ZERO(&accept_fdset);
//        FD_SET(m_lfd, &accept_fdset);
//        timeout.tv_sec = wait_seconds;
//        timeout.tv_usec = 0;
//        do {
//#ifdef _WIN32
//            ret = select(0, &accept_fdset, NULL, NULL, &timeout);//
//#else
//            ret = select(m_lfd + 1, &accept_fdset, NULL, NULL, &timeout);
//#endif
//        } while (ret < 0
//#ifdef _WIN32
//            && WSAGetLastError() == WSAEINTR
//#else
//            && errno == EINTR
//#endif
//            );
//        if (ret <= 0) {
//            return NULL;
//        }
//    }
//
//    // 一旦检测出有select事件发生，表示对等方完成了三次握手，客户端有新连接建立
//    // 此时再调用accept将不会阻塞
//    struct sockaddr_in addrCli;
//    socklen_t addrlen = sizeof(struct sockaddr_in);
//    int connfd;
//#ifdef _WIN32
//    connfd = accept(m_lfd, (struct sockaddr*)&addrCli, &addrlen);
//    if (connfd == INVALID_SOCKET) {
//        return NULL;
//    }
//#else
//    connfd = accept(m_lfd, (struct sockaddr*)&addrCli, &addrlen);
//    if (connfd == -1) {
//        return NULL;
//    }
//#endif
//
//    return new TcpSocket(connfd);
}

void TcpServer::closefd()
{
#ifdef _WIN32
    closesocket(m_lfd);
#else
    close(m_lfd);
#endif
}
