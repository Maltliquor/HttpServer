# HttpServer
[![license](https://img.shields.io/github/license/mashape/apistatus.svg)](https://opensource.org/licenses/MIT)  
A tiny web server in C++

## Table of Contents

- [Introduction](#Introduction)
- [Environment](#Enviroment)
- [Install](#Install)
- [Usage](#Usage)
- [Model](#Model)
- [Test](#Test)
- [Update](#Update)

## Introduction

- 本项目为参考muduo编写的简化版Web服务器，实现的功能为服务端解析GET请求并返回静态资源、支持HTTP长连接。  
- 代码部分关于thread pool和线程调度的部分采用了muduo的源码，出于后续改进的需要，本项目删除了Buffer类、LogFile相关的类、Timer相关的类以及Acceptor类，并重写了HttpResponse、HttpServer等类的成员函数。
- 测试网址：[HttpServer主页](http://39.101.190.70/) HttpServer测试页:39.101.190.70/test

## Environment

- OS: Ubuntu 16.04
- Compiler: g++ 4.8

## Install

```sh
$ ./build.sh
```

## Usage

```sh
$ cd build/Debug/bin
$ ./demo_server [thread_numbers]
```

## Model

-	本项目采用的并发模型为Reactor模式+One Loop per Thread，图示如下：  
![thread_pool](image/threadpool_gh.png)

- 类间组织关系如下图：  
![class](image/uml class_gh.png)


## Test

使用webbench进行连接测试 
Nginx测试结果：1000个客户端，持续请求30s，速率为998230 pages/min  
![webbench-nginx](image/webbench-nginx.png)  
 
HttpServer测试结果：1000个客户端，持续请求30s，速率为788 pages/min  
![webbench-server](image/webbench-server.png)  
由结果可见，本服务器在高并发环境下运行效果并不理想，与反向代理引擎nginx相比，性能差距巨大。


## Update

要实现高并发高性能的Web服务器，需要在以下几个方面进行改进：
- 增加Buffer类：write()写入数据，会阻塞直到全部数据写完。若一次write()未写完所有数据，为避免write()阻塞在线程中，先在buffer类中存储未发送完的数据，立即返回并等待下一次可写事件。
- 增加Timer类：定时器事件是服务端工作的重要组成部分，是心跳包等工作机制实现的基础。系统内产生的多个定时器事件组成了定时器队列，常见的队列组织形式包括时间轮，时间堆，本项目拟采用时间堆的结构操作定时器事件。
- 增加LogFile类：实现异步读写日志的功能。日志记录了服务器运行时的状态，是系统调试和维护的依据。在多线程并发模型中，为生成准确的日志文件，muduo使用双缓冲技术，并成功实现了高效的异步日志记录功能。

