#include "../include/TcpSocket.h"

TcpSocket::TcpSocket():m_socket(-1)
{
}

TcpSocket::TcpSocket(int connfd)
{
	//传入已有的通信套接字创建通信类
	m_socket = connfd;
}

TcpSocket::~TcpSocket()
{

}

int TcpSocket::connectToHost(string ip, unsigned short port, int timeout)
{
	int ret = 0;
	//参数检查
	if (port <= 0 || port > 65535 || timeout < 0)
	{
		ret = ParamError;
		return ret;
	}

#ifdef _WIN32
	WSADATA wsaData;
	//初始化 Winsock 环境
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cerr << "WSAStartup failed." << std::endl;
		return 1;
	}
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_socket == INVALID_SOCKET)
	{//如套接字无效
		//获取Winsock的最后一个错误代码
		ret = WSAGetLastError();
		std::cerr << "创建套接字失败，错误代码：" << ret << std::endl;
		//终止Winsock的使用，清理
		WSACleanup();
		return ret;
	}
#else
	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket < 0)
	{
		ret = errno;//errno系统全局变量，记录系统函数错误号
		printf("创建套接字失败，错误代码:  %d\n", ret);
		return ret;
	}
#endif // _WIN32

	//准备服务器地址结构
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;//使用IPV4
	servaddr.sin_port = htons(port);//将端口号从主机字节序转为网络字节序
#ifdef _WIN32
	//servaddr.sin_addr.s_addr = inet_pton(AF_INET,ip.c_str(), &servaddr.sin_addr);//将字符串转换程点分十进制ip地址
	inet_pton(AF_INET, ip.c_str(), &servaddr.sin_addr);//将字符串转换程点分十进制ip地址
#else
	servaddr.sin_addr.s_addr = inet_addr(ip.data());//将字符串转换程点分十进制ip地址
#endif
	

	//连接并设置超时
	ret = connectTimeout((struct sockaddr_in*)(&servaddr), (unsigned int)timeout);
	if (ret < 0)
	{
		// 连接超时

#ifdef _WIN32
		if (ret == -1 && WSAGetLastError() == WSAETIMEDOUT)
#else
		if (ret == -1 && errno == ETIMEDOUT) 
#endif
		{
			ret = TimeoutError;
			return ret;
		}
		else
		{

#ifdef _WIN32
			printf("connectTimeout 调用异常, 错误号: %d\n", WSAGetLastError());
			return ret;
#else
			printf("connectTimeout 调用异常, 错误号: %d\n", errno);
			return ret;
#endif
			
		}
	}

	return ret;
}

int TcpSocket::sendMsg(string sendData, int timeout)
{
	// 返回0表示未超时，返回-1表示超时
	int ret = writeTimeout(timeout);
	if (ret == 0)
	{
		int writed = 0;
		int dataLen = sendData.size() + 4;
		// 添加4字节作为数据头，用于存储数据块长度
		unsigned char* netdata = (unsigned char*)malloc(dataLen);
		if (netdata == NULL)
		{
			ret = MallocError;
			printf("func sckClient_send() malloc Err:%d\n ", ret);
			return ret;
		}
		// 转换为网络字节序
		int netlen = htonl(sendData.size());
		memcpy(netdata, &netlen, 4); // 复制数据长度到数据头
		memcpy(netdata + 4, sendData.data(), sendData.size()); // 复制实际数据到数据缓冲区

		// writen函数成功返回发送的实际字节数，应该等于dataLen；失败返回-1
		writed = writen(netdata, dataLen);
		if (writed < dataLen) // 发送失败
		{
			if (netdata != NULL)
			{
				free(netdata); // 释放内存
				netdata = NULL;
			}
			return writed;
		}
		// 释放内存
		if (netdata != NULL)
		{
			free(netdata);
			netdata = NULL;
		}
	}
	else
	{
		// 失败返回-1，超时返回-1并且errno = ETIMEDOUT
		if (ret == -1 && errno == ETIMEDOUT)
		{
			ret = TimeoutError;
			printf("func sckClient_send() timeout Err:%d\n ", ret);
		}
	}

	return ret;
}

string TcpSocket::recvMsg(int timeout)
{
	// 返回0表示未超时且接收到数据，-1表示超时或异常
	int ret = readTimeout(timeout);
	if (ret != 0)
	{
		if (ret == -1 || errno == ETIMEDOUT)
		{
			printf("readTimeout(timeout) err: TimeoutError \n");
			return string();
		}
		else
		{
			printf("readTimeout(timeout) err: %d \n", ret);
			return string();
		}
	}

	int netdatalen = 0;
	ret = readn(&netdatalen, 4); // 读取包头4个字节
	if (ret == -1)
	{
		//printf("func readn() err:%d \n", ret);
		printf("有终端下线. err:%d \n", ret);
		return string();
	}
	else if (ret < 4)
	{
		printf("func readn() err peer closed:%d \n", ret);
		return string();
	}

	int n = ntohl(netdatalen); // 将网络字节序转换为主机字节序
	// 根据包头中记录的数据大小申请内存，用于接收数据
	char* tmpBuf = (char*)malloc(n + 1);
	if (tmpBuf == NULL)
	{
		ret = MallocError;
		printf("malloc() err \n");
		return NULL;
	}

	ret = readn(tmpBuf, n); // 根据长度读取数据
	if (ret == -1)
	{
		printf("func readn() err:%d \n", ret);
		return string();
	}
	else if (ret < n)
	{
		printf("func readn() err peer closed:%d \n", ret);
		return string();
	}

	tmpBuf[n] = '\0'; // 多分配一个字节用于兼容可见字符串，字符串的真实长度仍为n
	string data = string(tmpBuf); // 将接收到的数据转换为字符串
	// 释放内存
	free(tmpBuf);

	return data;
}

void TcpSocket::disConnect()
{
	if (m_socket != -1) {
#ifdef _WIN32
		closesocket(m_socket);
		WSACleanup();  // Windows 下的清理
#else
		close(m_socket);
#endif
	}
}


/////////////////////////////////////////////////
//////             子函数                   //////
/////////////////////////////////////////////////
/*
* setNonBlock - 设置I/O为非阻塞模式
* @fd: 文件描符符
*/
int TcpSocket::setNonBlock(int fd)
{
#ifdef _WIN32
	// Windows平台使用ioctlsocket设置非阻塞模式
	unsigned long mode = 1;
	int ret = ioctlsocket(fd, FIONBIO, &mode);
	return (ret == NO_ERROR) ? 0 : -1;
#else
	// Linux平台使用fcntl设置非阻塞模式
	int flags = fcntl(fd, F_GETFL);
	if (flags == -1) {
		return -1;
	}

	flags |= O_NONBLOCK;
	int ret = fcntl(fd, F_SETFL, flags);
	return ret;
#endif
}

/*
* setBlock - 设置I/O为阻塞模式
* @fd: 文件描符符
*/
int TcpSocket::setBlock(int fd)
{
#ifdef _WIN32
	// Windows平台使用ioctlsocket设置阻塞模式
	unsigned long mode = 0;
	int ret = ioctlsocket(fd, FIONBIO, &mode);
	return (ret == NO_ERROR) ? 0 : -1;
#else
	// Linux平台使用fcntl设置阻塞模式
	int flags = fcntl(fd, F_GETFL);
	if (flags == -1) {
		return -1;
	}

	flags &= ~O_NONBLOCK;
	int ret = fcntl(fd, F_SETFL, flags);
	return ret;
#endif
}

/*
* readTimeout - 读超时检测函数，不含读操作
* @wait_seconds: 等待超时秒数，如果为0表示不检测超时
* 成功（未超时）返回0，失败返回-1，超时返回-1并且errno = ETIMEDOUT
*/
int TcpSocket::readTimeout(unsigned int wait_seconds)
{
	/*跨平台win用select，linux用epoll，epoll高并发更好*/
	int ret = 0;
	// 如果需要等待
	if (wait_seconds > 0) {
#ifdef _WIN32
		// Windows 平台使用 select
		fd_set read_fdset;
		struct timeval timeout;
		FD_ZERO(&read_fdset);
		FD_SET(m_socket, &read_fdset);
		timeout.tv_sec = wait_seconds;
		timeout.tv_usec = 0;
		do {
			//如果wait_seconds没准备好就返回，且window下第一个参数0或者m_socket + 1都可,读在第二个参数
			ret = select(0, &read_fdset, NULL, NULL, &timeout);
		} while (ret < 0 && WSAGetLastError() == WSAEINTR);
#else
		// Linux 平台使用 epoll
		int epfd = epoll_create(1);//默认传1，系统会分配红黑树节点数，epfd是红黑树的根
		struct epoll_event ev, events[1];
		ev.events = EPOLLOUT;//文件描述可写 !!!!!!!!!针对写的宏
		ev.data.fd = m_socket;//关心的文件描述符
		//epfd：epoll实例
		//EPOLL_CTL_ADD：将m_socket加入到epfd
		//ev：事件结构体
		epoll_ctl(epfd, EPOLL_CTL_ADD, m_socket, &ev);
		struct epoll_event evlist;//存储返回的事件
		//在epfd中等待最多1个事件，最多等wait_seconds * 1000超时时间，等到的事件存放至evlist结构体
		ret = epoll_wait(epfd, &evlist, 1, wait_seconds * 1000);
		close(epfd);
#endif

		if (ret <= 0)
		{//超时
			// 没有事件发生或者出错，直接返回 -1
			ret = -1;
		}
		else
		{
			ret = 0;
		}
	}

	return ret;

/*跨平台都都用select方式*/
//	int ret = 0;
//	if (wait_seconds > 0) {
//		fd_set read_fdset;
//		struct timeval timeout;
//
//		FD_ZERO(&read_fdset);
//		FD_SET(m_socket, &read_fdset);
//
//		timeout.tv_sec = wait_seconds;
//		timeout.tv_usec = 0;
//
//		// select返回值三态
//		// 1. 若timeout时间到（超时），没有检测到读事件 ret返回=0
//		// 2. 若ret返回<0 && errno == EINTR 说明select的过程中被别的信号中断（可中断睡眠原理）
//		// 2.1. 若返回-1，select出错
//		// 3. 若ret返回值>0 表示有read事件发生，返回事件发生的个数
//
//		do {
//#ifdef _WIN32
//			ret = select(0, &read_fdset, NULL, NULL, &timeout);
//#else
//			ret = select(m_socket + 1, &read_fdset, NULL, NULL, &timeout);
//#endif
//		} while (ret < 0
//#ifdef _WIN32
//			&& WSAGetLastError() == WSAEINTR
//#else
//			&& errno == EINTR
//#endif
//			);
//
//		if (ret == 0) {
//			ret = -1;
//#ifdef _WIN32
//			WSASetLastError(WSAETIMEDOUT);
//#else
//			errno = ETIMEDOUT;
//#endif
//		}
//		else if (ret == 1) {
//			ret = 0;
//		}
//	}
//
//	return ret;
}

/*
* writeTimeout - 写超时检测函数，不含写操作
* @wait_seconds: 等待超时秒数，如果为0表示不检测超时
* 成功（未超时）返回0，失败返回-1，超时返回-1并且errno = ETIMEDOUT
*/
int TcpSocket::writeTimeout(unsigned int wait_seconds)
{
	/*跨平台win用select，linux用epoll，epoll高并发更好*/
	int ret = 0;
	// 如果需要等待
	if (wait_seconds > 0) {
#ifdef _WIN32
		// Windows 平台使用 select
		fd_set write_fdset;
		struct timeval timeout;
		FD_ZERO(&write_fdset);
		FD_SET(m_socket, &write_fdset);
		timeout.tv_sec = wait_seconds;
		timeout.tv_usec = 0;
		do {
			//如果wait_seconds没准备好就返回，且window下第一个参数0或者m_socket + 1都可，写在第二个参数
			ret = select(0, NULL, &write_fdset, NULL, &timeout);
		} while (ret < 0 && WSAGetLastError() == WSAEINTR);
#else
		// Linux 平台使用 epoll
		int epfd = epoll_create(1);//默认传1，系统会分配红黑树节点数，epfd是红黑树的根
		struct epoll_event ev, events[1];
		ev.events = EPOLLOUT;//文件描述可写 !!!!!!!!!针对写的宏
		ev.data.fd = m_socket;//关心的文件描述符
		//epfd：epoll实例
		//EPOLL_CTL_ADD：将m_socket加入到epfd
		//ev：事件结构体
		epoll_ctl(epfd, EPOLL_CTL_ADD, m_socket, &ev);
		struct epoll_event evlist;//存储返回的事件
		//在epfd中等待最多1个事件，最多等wait_seconds * 1000超时时间，等到的事件存放至evlist结构体
		ret = epoll_wait(epfd, &evlist, 1, wait_seconds * 1000);
		close(epfd);
#endif

		if (ret <= 0) 
		{//超时
			// 没有事件发生或者出错，直接返回 -1
			ret = -1;
		}
		else
		{
			ret = 0;
		}
	}

	return ret;

/*跨平台都都用select方式*/
//	int ret = 0;
//	if (wait_seconds > 0) {
//		fd_set write_fdset;
//		struct timeval timeout;
//
//		FD_ZERO(&write_fdset);
//		FD_SET(m_socket, &write_fdset);
//		timeout.tv_sec = wait_seconds;
//		timeout.tv_usec = 0;
//
//		// select返回值三态
//		// 1. 若timeout时间到（超时），没有检测到写事件 ret返回=0
//		// 2. 若ret返回<0 && errno == EINTR 说明select的过程中被别的信号中断（可中断睡眠原理）
//		// 2.1. 若返回-1，select出错
//		// 3. 若ret返回值>0 表示有write事件发生，返回事件发生的个数
//
//		do {
//#ifdef _WIN32
//			ret = select(0, NULL, &write_fdset, NULL, &timeout);
//#else
//			ret = select(m_socket + 1, NULL, &write_fdset, NULL, &timeout);
//#endif
//		} while (ret < 0
//#ifdef _WIN32
//			&& WSAGetLastError() == WSAEINTR
//#else
//			&& errno == EINTR
//#endif
//			);
//
//		// 超时
//		if (ret == 0) {
//			ret = -1;
//#ifdef _WIN32
//			WSASetLastError(WSAETIMEDOUT);
//#else
//			errno = ETIMEDOUT;
//#endif
//		}
//		else if (ret == 1) {
//			ret = 0; // 没超时
//		}
//	}
//
//	return ret;
}

/*
* connectTimeout - connect
* @addr: 要连接的对方地址
* @wait_seconds: 等待超时秒数，如果为0表示正常模式
* 成功（未超时）返回0，失败返回-1，超时返回-1并且errno = ETIMEDOUT
*/
int TcpSocket::connectTimeout(sockaddr_in* addr, unsigned int wait_seconds)
{
	int ret;
	socklen_t addrlen = sizeof(struct sockaddr_in);

	if (wait_seconds > 0)
	{
		setNonBlock(m_socket);	// 设置非阻塞IO
	}

	ret = connect(m_socket, (struct sockaddr*)addr, addrlen);
	// 非阻塞模式连接, 返回-1, 并且errno为EINPROGRESS, 表示连接正在进行中
	/*跨平台win用select，linux用epoll，epoll高并发更好*/
#ifdef _WIN32
	if (ret < 0 && WSAGetLastError() == WSAEWOULDBLOCK)
	{
		// Windows 平台使用 select
		fd_set connect_fdset;//存储套接字的，使用这个告诉select函数我们关心哪些套接字
		struct timeval timeout;
		FD_ZERO(&connect_fdset);//清空connect_fdset的套接字
		FD_SET(m_socket, &connect_fdset);//把m_socket放入connect_fdset
		timeout.tv_sec = wait_seconds;
		timeout.tv_usec = 0;
		do {
			//ret = select(0, &connect_fdset, NULL, NULL, &timeout);
			// 一但连接建立，则套接字就可写 所以connect_fdset放在了写集合中
			//如果wait_seconds没准备好就返回，且window下第一个参数0或者m_socket + 1都可
			ret = select(0, NULL, &connect_fdset, NULL, &timeout);
		} while (ret < 0 && WSAGetLastError() == WSAEINTR);

		if (ret == 0)
		{
			// 超时
			ret = -1;
			WSASetLastError(WSAETIMEDOUT);
		}
		else if (ret < 0)
		{
			return -1;
		}
		else if (ret == 1)
		{
			/* ret返回为1（表示套接字可写），可能有两种情况，一种是连接建立成功，一种是套接字产生错误，*/
			/* 此时错误信息不会保存至errno变量中，因此，需要调用getsockopt来获取。 */
			int err;
			socklen_t sockLen = sizeof(err);
			int sockoptret = getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (char*)&err, &sockLen);
			if (sockoptret == -1)
			{
				return -1;
			}
			if (err == 0)
			{
				ret = 0;	// 成功建立连接
			}
			else
			{
				// 连接失败
				WSASetLastError(err);
				ret = -1;
			}
	}
}
	if (wait_seconds > 0)
	{
		setBlock(m_socket);	// 套接字设置回阻塞模式
	}
	return ret;
	
#else
	if (ret < 0 && errno == EINPROGRESS)
	{
		// Linux 平台使用 epoll
		int epfd = epoll_create(1);//默认传1，系统会分配红黑树节点数，epfd是红黑树的根
		struct epoll_event ev, events[1];
		ev.events = EPOLLOUT;//文件描述可写 !!!!!!!!!针对写的宏
		ev.data.fd = m_socket;//关心的文件描述符
		//epfd：epoll实例
		//EPOLL_CTL_ADD：将m_socket加入到epfd
		//ev：事件结构体
		epoll_ctl(epfd, EPOLL_CTL_ADD, m_socket, &ev);
		struct epoll_event evlist;//存储返回的事件
		//在epfd中等待最多1个事件，最多等wait_seconds * 1000超时时间，等到的事件存放至evlist结构体
		ret = epoll_wait(epfd, &evlist, 1, wait_seconds * 1000);
		close(epfd);

		if (ret == 0)
		{
			// 超时
			ret = -1;
			errno = ETIMEDOUT;
		}
		else if (ret < 0)
		{
			return -1;
		}
		else if (ret == 1)
		{
			/* ret返回为1（表示套接字可写），可能有两种情况，一种是连接建立成功，一种是套接字产生错误，*/
			/* 此时错误信息不会保存至errno变量中，因此，需要调用getsockopt来获取。 */
			int err;
			socklen_t sockLen = sizeof(err);
			int sockoptret = getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (char*)&err, &sockLen);
			if (sockoptret == -1)
			{
				return -1;
			}
			if (err == 0)
			{
				ret = 0;	// 成功建立连接
			}
			else
			{
				// 连接失败
				errno = err;
				ret = -1;
			}
		}
}
	if (wait_seconds > 0)
	{
		setBlock(m_socket);	// 套接字设置回阻塞模式
	}
	return ret;
#endif
/*跨平台都都用select方式*/
//#ifdef _WIN32
//	if (ret < 0 && WSAGetLastError() == WSAEWOULDBLOCK)
//#else
//	if (ret < 0 && errno == EINPROGRESS)
//#endif
//	{
//		fd_set connect_fdset;//存储套接字的，使用这个告诉select函数我们关心哪些套接字
//		struct timeval timeout;
//		FD_ZERO(&connect_fdset);//清空connect_fdset的套接字
//		FD_SET(m_socket, &connect_fdset);//把m_socket放入connect_fdset
//		timeout.tv_sec = wait_seconds;
//		timeout.tv_usec = 0;
//		do
//		{
//			// 一但连接建立，则套接字就可写 所以connect_fdset放在了写集合中
//			//如果wait_seconds没准备好就返回
//			ret = select(m_socket + 1, NULL, &connect_fdset, NULL, &timeout);
//#ifdef _WIN32
//		} while (ret < 0 && WSAGetLastError() == WSAEINTR);
//#else
//	} while (ret < 0 && errno == EINTR);
//#endif
//
//		if (ret == 0)
//		{
//			// 超时
//			ret = -1;
//#ifdef _WIN32
//			WSASetLastError(WSAETIMEDOUT);
//#else
//			errno = ETIMEDOUT;
//#endif
//		}
//		else if (ret < 0)
//		{
//			return -1;
//		}
//		else if (ret == 1)
//		{
//			/* ret返回为1（表示套接字可写），可能有两种情况，一种是连接建立成功，一种是套接字产生错误，*/
//			/* 此时错误信息不会保存至errno变量中，因此，需要调用getsockopt来获取。 */
//			int err;
//			socklen_t sockLen = sizeof(err);
//			int sockoptret = getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (char*)&err, &sockLen);
//			if (sockoptret == -1)
//			{
//				return -1;
//			}
//			if (err == 0)
//			{
//				ret = 0;	// 成功建立连接
//			}
//			else
//			{
//				// 连接失败
//#ifdef _WIN32
//				WSASetLastError(err);
//#else
//				errno = err;
//#endif
//				ret = -1;
//			}
//		}
//	}
//	if (wait_seconds > 0)
//	{
//		setBlock(m_socket);	// 套接字设置回阻塞模式
//	}
//	return ret;
}

/*
* readn - 读取固定字节数
* @fd: 文件描述符
* @buf: 接收缓冲区
* @count: 要读取的字节数
* 成功返回count，失败返回-1，读到EOF返回<count
*/
int TcpSocket::readn(void* buf, int count)
{
	size_t nleft = count;
	long int nread;
	char* bufp = (char*)buf;

	while (nleft > 0) {
#ifdef _WIN32
		nread = recv(m_socket, bufp, nleft, 0);
#else
		nread = read(m_socket, bufp, nleft);
#endif
		if (nread < 0) {
#ifdef _WIN32
			if (WSAGetLastError() == WSAEINTR) {
#else
			if (errno == EINTR) {
#endif
				continue;
			}
			return -1;
			}
		else if (nread == 0) {
			return count - nleft; // EOF
		}

		bufp += nread;
		nleft -= nread;
		}

	return count;
}

/*
* writen - 发送固定字节数
* @buf: 发送缓冲区
* @count: 要读取的字节数
* 成功返回count，失败返回-1
*/
int TcpSocket::writen(const void* buf, int count)
{
	size_t nleft = count;
	long int nwritten;
	const char* bufp = (const char*)buf;

	while (nleft > 0) {
#ifdef _WIN32
		nwritten = send(m_socket, bufp, nleft, 0);
#else
		nwritten = write(m_socket, bufp, nleft);
#endif
		if (nwritten < 0) {
#ifdef _WIN32
			if (WSAGetLastError() == WSAEINTR) { // 被信号打断
#else
			if (errno == EINTR) { // 被信号打断
#endif
				continue;
			}
			return -1;
			}
		else if (nwritten == 0) {
			continue;
		}

		bufp += nwritten;
		nleft -= nwritten;
		}

	return count;
}
