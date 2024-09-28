# 1. base64

## 1.1 概念

> Base64**是一种基于64个可打印字符**来<font color="red">表示二进制数据</font>的表示方法。在Base64中的可打印字符包括[字母](https://zh.wikipedia.org/wiki/%E6%8B%89%E4%B8%81%E5%AD%97%E6%AF%8D)`A-Z`、`a-z`、[数字](https://zh.wikipedia.org/wiki/%E6%95%B0%E5%AD%97)`0-9`，这样共有62个字符，加上**+**和**/**, 共64个字符.(一般二进制数据为了能显示可见才需要base64编解码)
>
> - 普通的文本数据也可以使用base64进行编解码
> - Base64编解码的过程是可逆的
> - Base64不能当做加密算法来使用

![](assets/1.png)

## 1.2 应用场景

> 在计算机中任何数据都是按ascii码存储的，而ascii码的128～255之间的值是不可见字符。 而在网络上交换数据时，比如说从A地传到B地，往往要经过多个路由设备，由于不同的设备对字符的处理方式有一些不同，这样那些不可见字符就有可能被处理错误，这是不利于传输的。所以就先把数据先做一个Base64编码，统统变成可见字符，这样出错的可能性就大降低了。 (学习的密钥加密后就是二进制会有不可见字符有些可能就是\0，网络传输走别人收到直接\0结束了，所以将二进制进行base64编码一下)

- base64 应用

  > - 它可用来作为[电子邮件](https://zh.wikipedia.org/wiki/%E7%94%B5%E5%AD%90%E9%82%AE%E4%BB%B6)的传输[编码](https://zh.wikipedia.org/wiki/%E5%AD%97%E7%AC%A6%E7%BC%96%E7%A0%81)。 
  >   - 附件
  >     - 图片 -> 二进制
  >     - 音频/视频 -> 二进制格式的文件
  >   - 邮件传输协议只支持 ASCII字符传递，因此如果要传输二进制文件：图片、视频是无法实现的。 
  > - Http协议
  >   - HTTP协议要求请求行和请求头都必须是ASCII编码 
  > - 数据库数据读写 - blob
  >   - blob格式
  >     - 文件
  >     - 音频
  >     - 视频
  >     - 图片
  >   - blob格式的数据中有一些特殊的数据:
  >     - 中文
  >       - 写之前进行编码
  >       - 读出来之后解码
  >     - 二进制数据

- base64 算法

  > - 把3个8位字节（3\*8=24）转化为4个6位的字节（4*6=24）
  >   - 假设有一个字符串, 需要对这个字符串分组, 每3个字节为一组, 分成N组
  >   - 将每一组的3个字节拆分, 拆成4个字节, 每个字节有6bit
  > - 在6位的前面补两个0，形成8位一个字节的形式
  >   - 每个组就从3个字节变成了4个字节
  >   - 结论: base64编码之后的字符串变大了, 
  > - 如果剩下的字符不足3个字节，则用0填充，输出字符使用'='，因此编码后输出的文本末尾可能会出现1或2个'=', 表示补了多少字节，解码的时候，会自动去掉。

  ```c
  /*
  	数据通过base64编码, 编码之后的数据边长了还是变短了?
  		- 变长了
  		- 每三个字节一组, 编码之后每组增加一个字节
  	编码之后的数据末尾有可能有=, 这个=是什么意思?
  		- =代表0, 0是给最后一组补位用的, 最后一组缺几个字节就补几个0
  		- 在解码的时候根据=的个数去掉对应的字节数
  */
  ```
  
  
  
  ![](assets/2.png)
  
  ![](assets/1527764667795.png)

```c
Qt中base64编解码
 在QByteArray中
 	QByteArray QByteArray::toBase64() const;
    [static] QByteArray QByteArray::fromBase64(const QByteArray &base64);
```



## 1.3 openssl 中base64的使用

OpenSSL中文手册之BIO库详解: 	<https://blog.csdn.net/liao20081228/article/details/77193729>

- BIO 操作

  ```c
  // 创建BIO对象
  BIO *BIO_new(const BIO_METHOD *type);
  // 封装了base64编码方法的BIO,写的时候进行编码，读的时候解码
  BIO_METHOD* BIO_f_base64();
  // 封装了内存操作的BIO接口，包括了对内存的读写操作
  BIO_METHOD* BIO_s_mem();
  
  // 创建一个内存型的BIO对象
  // BIO * bio = BIO_new(BIO_s_mem());
  BIO *BIO_new_mem_buf(void *buf, int len);
  // 创建一个磁盘文件操作的BIO对象
  BIO* BIO_new_file(const char* filename, const char* mode);
  
  // 从BIO接口中读出len字节的数据到buf中。
  // 成功就返回真正读出的数据的长度，失败返回0或－1，如果该BIO没有实现本函数则返回－2。
  int BIO_read(BIO *b, void *buf, int len);
  	- buf: 存储数据的缓冲区地址
  	- len: buf的最大容量
  // 往BIO中写入长度为len的数据。
  // 成功返回真正写入的数据的长度，失败返回0或－1，如果该BIO没有实现本函数则返回－2。
  int BIO_write(BIO *b, const void *buf, int len);
  	- buf: 要写入的数据, 写入到b对应的设备(内存/磁盘文件)中
  	- len: 要写入的数据的长度
  // 将BIO内部缓冲区的数据都写出去, 成功: 1, 失败: 0或-1
  int BIO_flush(BIO *b);
  	- 在使用BIO_write()进行写操作的时候, 数据有时候在openssl提供的缓存中
  	- 将openssl提供的缓存中的数据刷到设备(内存/磁盘文件)中
  
  // 把参数中名为append的BIO附加到名为b的BIO上，并返回b
  // 连接两个bio对象到链表中
  // 在链表中的关系: b->append
  BIO * BIO_push(BIO *b, BIO *append);
  	- b: 要插入到链表中的头结点
  	- append: 头结点的后继
  // 把名为b的BIO从一个BIO链中移除并返回下一个BIO，如果没有下一个BIO，那么就返回NULL。
  BIO * BIO_pop(BIO *b);
  
  typedef struct buf_mem_st BUF_MEM;
  struct buf_mem_st {
      size_t length;              /* current number of bytes */
      char *data;
      size_t max;                 /* size of buffer */
      unsigned long flags;
  };
  // 该函数也是一个宏定义函数，它将b底层的BUF_MEM结构放在指针pp中。
  BUF_MEM* ptr;
  long BIO_get_mem_ptr(BIO *b, BUF_MEM **pp);
  // 释放整个bio链
  void BIO_free_all(BIO *a);
  
  ```

  > 我们举几个简单的例子说明BIO_push和BIO_pop的作用，假设md1、md2是digest类型的BIO，b64是Base64类型的BIO，而f是file类型的BIO，那么如果执行操作:

  ```c
  BIO* md1 = BIO_new();	// md1 hash计算
  BIO* md2 = BIO_new();	// md2 hash计算
  BIO* b64 = BIO_new();	// base64 编解码
  BIO* f = BIO_new();		// 操作磁盘文件
  
  // 组织bio链
  // b64->f
  BIO_push(b64, f);
  // 那么就会形成一个b64-f的链。然后再执行下面的操作：
  // md2->b64->f
  BIO_push(md2, b64);
  // md1->md2->b64->f
  BIO_push(md1, md2);
  
  // bIO链
  md1->md2->b64->f
  // 写数据操作
  int BIO_write(md1, "hello, world", 11);
  // 数据的处理过程
  // 进行md1 的哈希计算 -> md2的哈希计算 -> base64编码 -> 数据被写到磁盘
  ```

  > 那么就会形成md1-md2-b64-f的BIO链，大家可以看到，在构造完一个BIO后，头一个BIO就代表了整个BIO链，**这根链表的概念几乎是一样的**。 
  >
  > 这时候，任何写往md1的数据都会经过md1,md2的摘要（或说hush运算)，然后经过base64编码，最后写入文件f。可以看到，构造一条好的BIO链后，操作是非常方便的，你不用再关心具体的事情了，整个BIO链会自动将数据进行指定操作的系列处理。 
  >
  > 需要注意的是，如果是读操作，那么数据会从相反的方向传递和处理，对于上面的BIO链，数据会从f文件读出，然后经过base64解码，然后经过md1,md2编码，最后读出。

  ![bio链](assets/20170815180254544.png)

- base64 编码 -> bio链的写操作

  ```c
  char* data = "hello, world";
  // 创建base64编码的bio对象
  BIO* b64 = BIO_new(BIO_f_base64());
  // 最终在内存中得到一个base64编码之后的字符串
  BIO* mem = BIO_new(BIO_s_mem());
  // 将两个bio对象串联, 结构: b64->mem
  BIO_push(b64, mem);
  // 将要编码的数据写入到bio对象中
  BIO_write(b64, data, strlen(data)+1);
  // 将数据从bio对象对应的内存中取出 -> char*
  BUF_MEM* ptr;
  // 数据通过ptr指针传出
  long BIO_get_mem_ptr(b64, &ptr);
  char* buf = new char[ptr->length];
  memcpy(buf, ptr->data, ptr->length);
  printf("编码之后的数据: %s\n", buf);
  ```
  
  
  
- base64 解码 -> bio链的读操作

  ```c
  // 要解码的数据
  char* data = "xxxxxxxxxxxxxxxxxxxx";
  // 创建base64解码的bio对象
  BIO* b64 = BIO_new(BIO_f_base64());
  #if 0
  // 存储要解码的数据
  BIO* mem = BIO_new(BIO_s_mem());
  // 将数据写入到mem对应的内存中
  BIO_write(mem, data, strlen(data));
  #else
  BIO *mem = BIO_new_mem_buf(data, strlen(data));
  #endif
   
  // 组织bio链
  BIO_push(b64, mem);
  // 读数据
  char buf[1024];
  int BIO_read(b64, buf, 1024);
  printf("base64解码的数据: %s\n", buf);
  ```
  
  
