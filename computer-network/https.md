# HTTPS 原理

[toc]

```
v2
created, 2025-06-01T15:00+08:00
category: computer-network
tags: https
```

## 非对称加密

1. 公钥加密的内容私钥才能解开
2. 私钥加密的内容公钥才能解开

要特别注意：内容用公钥加密以后，公钥自己都无法解开。

客户端和服务器之间，对称加密和非对称加密都用了。
简单来说，就是先用非对称加密协商得到对称加密的密钥，后续通过对称加密沟通。
因为对称加密更省计算资源。

大致的流程如下：

1. 服务器告诉客户端自己的公钥 `A_pub`，自己保留私钥 `A`
2. 客户端把对称加密的密钥 `X` 通过 `A_pub` 加密后发送给服务器
3. 服务器用 `A` 解密得到 `X`
4. 后续服务器和客户端用 `X` 加密和解密

实际过程中，一共产生了三个随机数来生成这个会话密钥 `X`，可以参考 [^2]。

客户端两个：Clint Random，pre-master
服务器一个：Server Random

## CA 证书

假如服务器是 [ms.com](https://ms.com)

客户端如何确认服务器发送给自己的 `A_pub` 来自 `ms.com`，而不是某个中间人呢？
这个就涉及到 CA 证书。

服务器给客户端一个证书，证书里面有：

```
Content:
- ms.com
- pub_key
- expire_time
- ...

Signature by CA's private key for hash(Content)
```

这个服务器发送给客户端的证书在谷歌浏览器中可以通过地址栏旁边的那个小按钮点出来。

客户端本身内置 CA 的公钥，用 Signature 校验就好了。

有时候我们上不安全的网站，需要安装证书，就是要用第三方的 CA，安装以后本地就会有第三方 CA 的公钥。

[^1]: [彻底搞懂 HTTPS 的加密原理 - 知乎](https://zhuanlan.zhihu.com/p/43789231)
[^2]: [硬核！30 张图解 HTTP 常见的面试题](https://mp.weixin.qq.com/s/bUy220-ect00N4gnO0697A)
