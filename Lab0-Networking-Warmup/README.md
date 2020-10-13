> # Lab0: Networking Warmup

## 环境配置

Windows WSL Ubuntu 18.04

默认的`g++`版本是`g++-7`，实验需要`g++-8`及以上，需要支持C++17语法标准

```bash
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt-get update
sudo apt-get install g++-8
cd /usr/bin 
sudo rm gcc
sudo rm g++
sudo ln -s /usr/bin/gcc-8  ./gcc
sudo ln -s /usr/bin/g++-8  ./g++

$ gcc --version
gcc (Ubuntu 8.4.0-1ubuntu1~18.04) 8.4.0
Copyright (C) 2018 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

$ g++ --version
g++ (Ubuntu 8.4.0-1ubuntu1~18.04) 8.4.0
Copyright (C) 2018 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

## Writing webget

需要去修改`.apps/wbget.cc`里面的`get_URL`函数，实现向指定的地址发送`GET`请求，然后将响应输出。

根据`get_URL`的定义和`main`函数的逻辑

```c++
void get_URL(const string &host, const string &path);
```

可以通过连接`http://cs144.keithw.org/hello`访问提供的页面，得到的是`Hello, CS144!`的响应，其中`cs144.keithw.org`就是需要传入的`host`，`hello`即为`path`。

所以我们的工作是进行字符串的拼接，利用`socket.hh`和`socket.cc`里提供的函数，来建立TCP连接，并发送HTTP的`GET`请求。

那么首先需要熟悉`socket.{hh, cc}`两个文件的内容，路径在`libsponge\util`下。





------

修改完后，在`sponge`目录下（我的是在`Lab0-Networking-Warmup`）目录下，新建一个`build`目录

```bash
mkdir build
cd build
cmake ..
make
## 构建完成得到
[100%] Linking CXX executable parser_dt
[100%] Built target parser_dt
```

`build`的文件结构：

```
.
├── CMakeCache.txt
├── CMakeDoxyfile.in
├── CMakeDoxygenDefaults.cmake
├── CMakeFiles
├── CTestTestfile.cmake
├── Doxyfile
├── Makefile
├── apps
├── cmake_install.cmake
├── compile_commands.json
├── doctests
├── libsponge
└── tests
```

通过下面命令检验程序：

```bash
$  ./apps/webget cs144.keithw.org /hello
HTTP/1.1 200 OK
Date: Tue, 13 Oct 2020 04:55:43 GMT
Server: Apache
Last-Modified: Thu, 13 Dec 2018 15:45:29 GMT
ETag: "e-57ce93446cb64"
Accept-Ranges: bytes
Content-Length: 14
Content-Type: text/plain

Hello, CS144!
```

表明程序可以正常运行，进一步的，需要通过`make check_webget`使用官方提供的测试样例来检验。

```bash
$ make check_webget
Scanning dependencies of target check_webget
[100%] Testing webget...
Test project /mnt/f/Project/Stanford-CS144-Introduction-to-Computer-Networking/Lab0-Networking-Warmup/build
    Start 28: t_webget
1/1 Test #28: t_webget .........................   Passed    1.29 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   1.31 sec
[100%] Built target check_webget
```



## An in-memory reliable byte stream

修改在`libsonge/byte_stream.{cc, hh}`文件的内容，来实现一个固定大小的`buffer`，用来存储输入的字节流。



最后使用`make check_lab0`检验：

```bash
$ make check_lab0
$ make check_lab0
[100%] Testing Lab 0...
Test project /mnt/f/Project/Stanford-CS144-Introduction-to-Computer-Networking/Lab0-Networking-Warmup/build
    Start 23: t_byte_stream_construction
1/9 Test #23: t_byte_stream_construction .......   Passed    0.02 sec
    Start 24: t_byte_stream_one_write
2/9 Test #24: t_byte_stream_one_write ..........   Passed    0.01 sec
    Start 25: t_byte_stream_two_writes
3/9 Test #25: t_byte_stream_two_writes .........   Passed    0.01 sec
    Start 26: t_byte_stream_capacity
4/9 Test #26: t_byte_stream_capacity ...........   Passed    0.59 sec
    Start 27: t_byte_stream_many_writes
5/9 Test #27: t_byte_stream_many_writes ........   Passed    0.02 sec
    Start 28: t_webget
6/9 Test #28: t_webget .........................   Passed    1.45 sec
    Start 50: t_address_dt
7/9 Test #50: t_address_dt .....................   Passed    0.04 sec
    Start 51: t_parser_dt
8/9 Test #51: t_parser_dt ......................   Passed    0.02 sec
    Start 52: t_socket_dt
9/9 Test #52: t_socket_dt ......................   Passed    0.01 sec

100% tests passed, 0 tests failed out of 9

Total Test time (real) =   2.21 sec
[100%] Built target check_lab0
```

偶尔会出现`t_webget` TIME_OUT的情况，这是因为需要连接的服务器在国外，并不稳定，此时多试几次就好了。