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
- 代码部分关于thread pool和线程调度的部分采用了muduo的源码，出于后续改进的需要，本项目删除了Buffer类、Timer相关的类以及Acceptor类，并重写了HttpResponse、HttpServer等类的成员函数。
- 测试网址：[HttpServer主页](https://39.101.190.70/) [HttpServer测试页](https://39.101.190.70/test)  

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

并发模型为Reactor+非阻塞IO+线程池，新连接Round Robin分配，

[![standard-readme compliant](https://img.shields.io/badge/readme%20style-standard-brightgreen.svg?style=flat-square)](https://github.com/RichardLitt/standard-readme)

为了加入徽章到 Markdown 文本里面，可以使用以下代码：

```
[![standard-readme compliant](https://img.shields.io/badge/readme%20style-standard-brightgreen.svg?style=flat-square)](https://github.com/RichardLitt/standard-readme)
```
  
UML时序图详见：[代码工作流程.md](代码工作流程.md)。

## Test

使用webbench进行连接测试
![cloc](https://github.com/linyacool/WebServer/blob/master/datum/cloc.png)

## Update

由于项：
- 增加Buffer类
- 增加Timer类
- 增加LogFile类：实现异步读写日志的功能

