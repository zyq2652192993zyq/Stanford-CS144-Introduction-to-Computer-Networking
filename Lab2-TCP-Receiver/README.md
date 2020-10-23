## Lab 2: the TCP receiver

执行`cmake`的时候报错缺少`pcap`库，选择安装`libpcap-dev`：

```bash
$ cd build && cmake ..
CMake Error: The following variables are used in this project, but they are set to NOTFOUND.
Please set them or make sure they are set and tested correctly in the CMake files:
LIBPCAP

$ sudo apt-get install libpcap-dev
```

安装完成后不会报错。