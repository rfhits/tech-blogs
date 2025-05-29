# 网络编程入门

[toc]

```
v2
created, 2025-05-26T20:28+08:00
published, 2025-05-29T12:14+08:00
category: computer-network
```

用 C 语言进行网络编程，是 [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/html/split/system-calls-or-bust.html) 的阅读笔记。

## `sockaddr`, `addrinfo`

首先有一堆表示网络地址的数据结构[^1]

```c
struct sockaddr {
    unsigned short    sa_family;    // address family, AF_xxx
    char              sa_data[14];  // 14 bytes of protocol address
};
```

这是一个统一表示地址的最简单的数据结构，IPv4、IPv6、unix 套接字 都可以表示，具体表示规则参考 https://www.doubao.com/thread/w0b3d39e6f3deed50。

`sockaddr_in` 和 `sockaddr_in6`可以理解为 `sockaddr` 的子类，可以直接 cast 成父类。
表示的规则是相通的，因为 port 在前，IP 地址在后。

```c
// (IPv4 only--see struct sockaddr_in6 for IPv6)

struct sockaddr_in {
    short int          sin_family;  // Address family, AF_INET
    unsigned short int sin_port;    // Port number
    struct in_addr     sin_addr;    // Internet address
    unsigned char      sin_zero[8]; // Same size as struct sockaddr
};
```

注：为什么端口号到 65535 内，因为 `short int` 占据 2 个字节，最大表示到 2^16 -1 = 65535

在 `sockaddr` 上面 wrap 一层就是 `addrinfo`，有了 TCP/UDP

```c
struct addrinfo {
    int              ai_flags;     // AI_PASSIVE, AI_CANONNAME, etc.
    int              ai_family;    // AF_INET, AF_INET6, AF_UNSPEC
    int              ai_socktype;  // SOCK_STREAM, SOCK_DGRAM
    int              ai_protocol;  // use 0 for "any"
    size_t           ai_addrlen;   // size of ai_addr in bytes
    struct sockaddr *ai_addr;      // struct sockaddr_in or _in6
    char            *ai_canonname; // full canonical hostname

    struct addrinfo *ai_next;      // linked list, next node
};
```

为什么设计成联表呢，因为 addrinfo 作为一个返回值，可以串起来，相当于函数可以直接返回一个链表。
[show-ip.c](./show-ip.c) 中提供了一个例子，作为查询域名对应的 ip，返回的结果就是 `addrinfo`，
域名可以有 IPv4 和 IPv6 地址。

提供了一些接口互相抓换以及 DNS query IP 地址：

-   `inet_ntop`
-   `inet_pton`
-   `getaddrinfo`: IP 查询

## `socket()`

[Beej's Guide to Network Programming](https://beej.us/guide/bgnet/html/split/system-calls-or-bust.html#fnref22)

`bind()` 将 socket 绑定到本地一个端口

`connect` 连接到远程主机的端口，如果自己是客户端的话，也不是非必须调用 bind，直接 connect 就好，内核会自动选择一个端口进行 bind

> One small extra final note about bind(): there are times when you won’t absolutely have to call it. If you are connect()ing to a remote machine and you don’t care what your local port is (as is the case with telnet where you only care about the remote port), you can simply call connect(), it’ll check to see if the socket is unbound, and will bind() it to an unused local port if necessary.

|          服务器端          |      客户端      |
| :------------------------: | :--------------: |
|        s = socket()        |   s = socket()   |
|      bind(s, ip:port)      |        -         |
|     listen(s, backlog)     |        -         |
|             -              |    connect()     |
|      内核处理三次握手      | 内核处理三次握手 |
| int accept() -> new_socket |        -         |
| 用 new_socket `recv/send`  | 用 s `recv/send` |

accept 会返回一个全新的 socket，然后老的 socket 可以继续用于监听旧的端口。
客户端在 connect 后，用初始的 socket 进行通信就好了。

`int send()` 返回发送了多少 bytes，`recv()` 返回收到了多少 bytes，如果 recv 返回 0，说明对面 shutdown 了。

## `poll()`

accept 和 recv 默认都是阻塞的，只能用来监控一个 socket fd
不过可以用 `fcntl` 来设置为非阻塞

实际我们的需求是监控一堆 fd，等到有一个 fd 突然响应就通知/唤醒我们处理

> Notice again that poll() returns the number of elements in the pfds array for which events have occurred. It doesn’t tell you which elements in the array (you still have to scan for that), but it does tell you how many entries have a non-zero revents field (so you can stop scanning after you find that many).

注意这个 fd 甚至可以是 标准输入输出

```cpp
#include <stdio.h>
#include <poll.h>

int main(void)
{
    struct pollfd pfds[1]; // More if you want to monitor more

    pfds[0].fd = 0;          // Standard input
    pfds[0].events = POLLIN; // Tell me when ready to read

    // If you needed to monitor other things, as well:
    //pfds[1].fd = some_socket; // Some socket descriptor
    //pfds[1].events = POLLIN;  // Tell me when ready to read

    printf("Hit RETURN or wait 2.5 seconds for timeout\n");

    int num_events = poll(pfds, 1, 2500); // 2.5 second timeout

    if (num_events == 0) {
        printf("Poll timed out!\n");
    } else {
        int pollin_happened = pfds[0].revents & POLLIN;

        if (pollin_happened) {
            printf("File descriptor %d is ready to read\n", pfds[0].fd);
        } else {
            printf("Unexpected event occurred: %d\n", pfds[0].revents);
        }
    }

    return 0;
}
```

所以说 poll 会在内核和用户态直接拷贝大量的数据，epoll 会更好，这个参考其他相关的文章。

[^1]: [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/html/split/ip-addresses-structs-and-data-munging.html#structs)
